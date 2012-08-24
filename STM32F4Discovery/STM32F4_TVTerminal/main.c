/*******************************************************************************
* File Name          : main.c
* Author             : KetilO
* Version            : V1.0.0
* Date               : 14/08/2012
* Description        : Main program body
*******************************************************************************/

/*******************************************************************************
* PAL timing Horizontal
* H-sync         4,70uS
* Front porch    1,65uS
* Active video  51,95uS
* Back porch     5,70uS
* Line total     64,0uS
*
* |                64.00uS                   |
* |4,70|1,65|          51,95uS          |5,70|
*
*            ---------------------------
*           |                           |
*           |                           |
*           |                           |
*       ----                             ----
* |    |                                     |
* -----                                      ----
*
* Vertical
* V-sync        0,576mS (9 lines)
* Frame         20mS (312,5 lines)
* Video signal  288 lines
*******************************************************************************/

/*******************************************************************************
* Port pins used
*
* Video out
* PA1   H-Sync and V-Sync
* PB15  Video out SPI2_MOSI
* RS232
* PA9   USART1 Tx
* PA10  USART1 Rx
* Keyboard
* PA8   Keyboard clock in
* PA11  Keyboard data in
* Leds
* PC08  Led
* PC09  Led
* User button
* PA1   User button
*******************************************************************************/

/*******************************************************************************
* Video output
*                  330
* PA1     O-------[  ]---o---------O  Video output
*                  1k0   |
* PB15    O-------[  ]---o
*                        |
*                       ---
*                       | |  82
*                       ---
*                        |
* GND     O--------------o---------O  GND
* 
*******************************************************************************/

/*******************************************************************************
* Keyboard connector 5 pin female DIN
*        2
*        o
*   4 o    o 5
*   1 o    o 3
* 
* Pin 1   CLK     Clock signal
* Pin 2   DATA    Data
* Pin 3   N/C     Not connected. Reset on older keyboards
* Pin 4   GND     Ground
* Pin 5   VCC     +5V DC
*******************************************************************************/

/*******************************************************************************
* Keyboard connector 6 pin female mini DIN
*
*   5 o    o 6
*   3 o    o 4
*    1 o o 2 
*
* Pin 1   DATA    Data
* Pin 2   N/C     Not connected.
* Pin 3   GND     Ground
* Pin 4   VCC     +5V DC
* Pin 5   CLK     Clock signal
* Pin 6   N/C     Not connected.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include <stdio.h>
#include "Font8x10.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TOP_MARGIN          30  // Number of lines before video signal starts
#define SCREEN_WIDTH        40  // 40 characters on each line.
#define SCREEN_HEIGHT       25  // 25 lines.
#define TILE_WIDTH          8   // Width of a character tile.
#define TILE_HEIGHT         10  // Height of a character tile.
#define SPI_DR              0x4001300C

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint16_t LineCount;
__IO uint16_t FrameCount;
uint8_t ScreenChars[SCREEN_HEIGHT][SCREEN_WIDTH];
uint8_t PixelBuff[SCREEN_WIDTH+2];

static uint8_t cx;
static uint8_t cy;
static uint8_t showcursor;

/* Private function prototypes -----------------------------------------------*/
void RCC_Config(void);
void NVIC_Config(void);
void GPIO_Config(void);
void TIM_Config(void);
void SPI_Config(void);
void DMA_Config(void);
void video_show_cursor();
void video_hide_cursor();
void video_scrollup();
void video_cls();
void video_cfwd();
void video_lfwd();
void video_lf();
void video_putc(char c);
void video_puts(char *str);
void video_puthex(uint8_t n);
void * memmove(void *dest, void *source, uint32_t count);
void * memset(void *dest, uint32_t c, uint32_t count); 
static void CURSOR_INVERT() __attribute__((noinline));


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint32_t i;
  uint16_t x,y;
  uint8_t c;
  y=0;
  c=0;
  while (y<SCREEN_HEIGHT)
  {
    x=0;
    while (x<SCREEN_WIDTH)
    {
      ScreenChars[y][x]=c;
      c++;
      x++;
    }
    y++;
  }

  RCC_Config();
  NVIC_Config();
  GPIO_Config();
  TIM_Config();
  SPI_Config();
  DMA_Config();
  /* Enable TIM3 */
  TIM_Cmd(TIM3, ENABLE);
  STM_EVAL_LEDInit(LED3);
  while (1)
  {
    if (FrameCount==50)
    {
      FrameCount=0;
      STM_EVAL_LEDToggle(LED3);
    }
  }
}

static void CURSOR_INVERT()
{
  ScreenChars[cy][cx] ^= showcursor;
}

static void _video_scrollup()
{
  memmove(&ScreenChars[0],&ScreenChars[1], (SCREEN_HEIGHT-1)*SCREEN_WIDTH);
  memset(&ScreenChars[SCREEN_HEIGHT-1], 0, SCREEN_WIDTH);
}

static void _video_lfwd()
{
  cx = 0;
  if (++cy > SCREEN_HEIGHT-1)
  {
    cy = SCREEN_HEIGHT-1;
    _video_scrollup();
  }
}

static inline void _video_cfwd()
{
  if (++cx > SCREEN_WIDTH-1)
    _video_lfwd();
}

static inline void _video_putc(char c)
{
  /* If the last character printed exceeded the right boundary,
   * we have to go to a new line. */
  if (cx >= SCREEN_WIDTH) _video_lfwd();

  if (c == '\r') cx = 0;
  else if (c == '\n') _video_lfwd();
  else
  {
    ScreenChars[cy][cx] = c;
    _video_cfwd();
  }
}

/*******************************************************************************
* Function Name  : video_show_cursor
* Description    : This function shows the cursor
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_show_cursor()
{
  if (!showcursor)
  {
    showcursor = 0x80;
    CURSOR_INVERT();
  }
}

/*******************************************************************************
* Function Name  : video_hide_cursor
* Description    : This function hides the cursor
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_hide_cursor()
{
  if (showcursor)
  {
    CURSOR_INVERT();
    showcursor = 0;
  }
}

/*******************************************************************************
* Function Name  : video_scrollup
* Description    : This function scrolls the screen up
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_scrollup()
{
  CURSOR_INVERT();
  _video_scrollup();
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_cls
* Description    : This function clears the screen and homes the cursor
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_cls()
{
  CURSOR_INVERT();
  memset(&ScreenChars, 0, SCREEN_HEIGHT*SCREEN_WIDTH);
  cx=0;
  cy=0;
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_cfwd
* Description    : This function 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_cfwd()
{
  CURSOR_INVERT();
  _video_cfwd();
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_lfwd
* Description    : This function handles a crlf
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_lfwd()
{
  CURSOR_INVERT();
  cx = 0;
  if (++cy > SCREEN_HEIGHT-1)
  {
    cy = SCREEN_HEIGHT-1;
    _video_scrollup();
  }
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_lf
* Description    : This function handles a lf
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void video_lf()
{
  CURSOR_INVERT();
  if (++cy > SCREEN_HEIGHT-1)
  {
    cy = SCREEN_HEIGHT-1;
    _video_scrollup();
  }
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_putc
* Description    : This function prints a character
* Input          : Character
* Output         : None
* Return         : None
*******************************************************************************/
void video_putc(char c)
{
  CURSOR_INVERT();
  _video_putc(c);
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_puts
* Description    : This function prints a zero terminated string
* Input          : Zero terminated string
* Output         : None
* Return         : None
*******************************************************************************/
void video_puts(char *str)
{
  /* Characters are interpreted and printed one at a time. */
  char c;
  CURSOR_INVERT();
  while ((c = *str++))
    _video_putc(c);
  CURSOR_INVERT();
}

/*******************************************************************************
* Function Name  : video_puthex
* Description    : This function prints a byte as hex
* Input          : Byte
* Output         : None
* Return         : None
*******************************************************************************/
void video_puthex(u8 n)
{
	static char hexchars[] = "0123456789ABCDEF";
	char hexstr[5];
	hexstr[0] = hexchars[(n >> 4) & 0xF];
	hexstr[1] = hexchars[n & 0xF];
	hexstr[2] = '\r';
	hexstr[3] = '\n';
	hexstr[4] = '\0';
  video_puts(hexstr);
}

/*******************************************************************************
* Function Name  : RCC_Config
* Description    : Configures peripheral clocks
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Config(void)
{
  /* Enable DMA1, GPIOA, GPIOB clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
  /* Enable TIM3, TIM4 and SPI2 clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4 | RCC_APB1Periph_SPI2, ENABLE);
}

/*******************************************************************************
* Function Name  : NVIC_Config
* Description    : Configures interrupts
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* Enable the TIM4 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : GPIO_Config
* Description    : Configures GPIO
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  /* Configure PA1 as output, H-Sync and V-Sync*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* H-Sync and V-Sync signal High */
  GPIO_SetBits(GPIOA,GPIO_Pin_1);

  /* SPI MOSI and SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  /* Connect SPI2 pins */  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
}

/*******************************************************************************
* Function Name  : TIM_Config
* Description    : Configures timers
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM_Config(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 84*64-1;                     // 64uS
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  /* Enable TIM3 Update interrupt */
  TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = (84*470)/100;                // 4,70uS
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  /* Enable TIM4 Update interrupt */
  TIM_ClearITPendingBit(TIM4,TIM_IT_Update);
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
}

/*******************************************************************************
* Function Name  : SPI_Config
* Description    : Configures SPI2 to output pixel data
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Config(void)
{
  SPI_InitTypeDef SPI_InitStructure;

	/* Set up SPI2 port */
  SPI_I2S_DeInit(SPI2);
  SPI_StructInit(&SPI_InitStructure);
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  /* 168/4(4=10,5 */
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_Init(SPI2, &SPI_InitStructure);
  /* Enable the SPI port */
  SPI_Cmd(SPI2, ENABLE);
}

/*******************************************************************************
* Function Name  : DMA_Config
* Description    : Configures DMA
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA_Config(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  DMA_DeInit(DMA1_Stream4);
  DMA_StructInit(&DMA_InitStructure);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (SPI2->DR);
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t) PixelBuff;
  DMA_InitStructure.DMA_BufferSize = (uint32_t)SCREEN_WIDTH/2+1;
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);
  SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
}

/**
  * @brief  This function handles TIM3 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
  uint16_t i,j,k;
  /* Clear the IT pending Bit */
  TIM3->SR=(u16)~TIM_IT_Update;
  /* TIM4 is used to time the H-Sync (4,70uS) */
  /* Reset TIM4 count */
  TIM4->CNT=0;
  /* Enable TIM4 */
  TIM4->CR1=1;
  /* H-Sync or V-Sync low */
  GPIOA->BSRRH = (uint16_t)GPIO_Pin_1;
  if (LineCount>=TOP_MARGIN && LineCount<SCREEN_HEIGHT*TILE_HEIGHT+TOP_MARGIN)
  {
    /* Make a video line. Since the SPI operates in halfword mode
       odd character first then even character stored in pixel buffer. */
    j=k=LineCount-TOP_MARGIN;
    j=j/TILE_HEIGHT;
    k=k-j*TILE_HEIGHT;
    i=0;
    while (i<SCREEN_WIDTH)
    {
      PixelBuff[i]=Font8x10[ScreenChars[j][i+1]][k];
      PixelBuff[i+1]=Font8x10[ScreenChars[j][i]][k];
      i+=2;
    }
  }
}

/**
  * @brief  This function handles TIM4 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM4_IRQHandler(void)
{
  DMA_InitTypeDef       DMA_InitStructure;
  uint32_t tmp;
  /* Disable TIM4 */
  TIM4->CR1=0;
  /* Clear the IT pending Bit */
  TIM4->SR=(u16)~TIM_IT_Update;
  if (LineCount<303)
  {
    /* H-Sync high */
    GPIOA->BSRRL=(u16)GPIO_Pin_1;
    if (LineCount>=TOP_MARGIN && LineCount<SCREEN_HEIGHT*TILE_HEIGHT+TOP_MARGIN)
    {
      /* The time it takes to init the DMA and run the loop is the Front porch */
      tmp=0;
      while (tmp<20)
      {
        tmp++;
      }
      /* Enable the DMA to keep the SPI port fed from the pixelbuffer. */
      DMA1_Stream4->NDTR = (uint32_t)SCREEN_WIDTH/2+1;
      DMA1_Stream4->M0AR = (uint32_t) PixelBuff;
      DMA_Cmd(DMA1_Stream4, ENABLE);
      // tmp=0;
      // while (tmp<20)
      // {
        // /* Wait until transmit register empty*/
        // while(!(SPI2->SR & SPI_I2S_FLAG_TXE));
        // /* Send Data */
        // SPI2->DR = 0xAAAA;
        // tmp++;
      // }
    }
  }
  else if (LineCount==313)
  {
    /* V-Sync high after 313-303=10 lines) */
    GPIOA->BSRRL=(u16)GPIO_Pin_1;
    FrameCount++;
    LineCount=0xffff;
  }
  LineCount++;
}

/*****END OF FILE****/