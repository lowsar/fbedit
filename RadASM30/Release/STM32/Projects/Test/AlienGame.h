
/* Includes ------------------------------------------------------------------*/
#include "stm32f4_discovery.h"
#include "window.h"
#include "video.h"
#include "keycodes.h"

/* Private define ------------------------------------------------------------*/
#define ALIEN_BOUND_LEFT      10
#define ALIEN_BOUND_TOP       26
#define ALIEN_BOUND_RIGHT     470
#define ALIEN_BOUND_BOTTOM    240

#define ALIEN_SHIELD_TOP      ALIEN_BOUND_BOTTOM-40
#define ALIEN_SHOOT_WAIT      25                      // 25 frames between shots

#define ALIEN_MAX_BOMBS       4
#define ALIEN_ALIEN_COLS      9
#define ALIEN_ALIEN_ROWS      5
#define ALIEN_MAX_ALIEN       ALIEN_ALIEN_COLS*ALIEN_ALIEN_ROWS
#define ALIEN_MAX_SHOTS       4
#define ALIEN_MAX_CANNONS     3
#define ALIEN_MAX_SHIELDS     5

const uint8_t Alien1Icon[16][16] = {
{2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2},
{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1},
{1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1},
{1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1},
{1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1},
{1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1},
{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
{2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2},
{2,2,2,1,1,2,2,2,2,2,2,1,1,2,2,2},
{2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2},
{2,1,1,2,2,2,2,2,2,2,2,2,2,1,1,2},
{1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1}
};

const uint8_t Alien2Icon[16][16] = {
{2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2},
{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1},
{1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1},
{1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1},
{1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1},
{1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1},
{2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2},
{2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2},
{2,2,2,1,1,2,2,2,2,2,2,1,1,2,2,2},
{2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2},
{2,2,1,1,2,2,2,2,2,2,2,2,1,1,2,2},
{2,2,1,1,1,1,2,2,2,2,1,1,1,1,2,2}
};

const uint8_t AlienCannonIcon[16][20] = {
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2},
{2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

const uint8_t AlienShieldIcon[16][36] = {
{2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2},
{2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2},
{2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2},
{2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2},
{2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2},
{2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2},
{2,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,2},
{2,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2},
{1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1},
{1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1}
};

const uint8_t AlienShotIcon[8][3] = {
{1,1,1},
{1,1,1},
{1,1,1},
{1,1,1},
{1,1,1},
{1,1,1},
{1,1,1},
{1,1,1}
};

typedef struct tagALIEN_GAME
{
  volatile uint8_t DemoMode;          // Demo mode flag
  volatile uint8_t GameOver;          // Game over flag
  volatile uint8_t Quit;              // Quit flag
  volatile int8_t Cannons;            // Number of spare Cannons
  volatile uint8_t Bombs;             // Number of active bombs
  volatile uint8_t Aliens;            // Number of active aliens
  volatile uint8_t Shots;             // Number of active shots
  volatile uint8_t ShootWait;         // Number of frames between shots
  volatile uint16_t Points;           // Points
  volatile int16_t sdir,slen,adir;    // Cannon move
  RECT AlienBound;                    // Game bounds
  SPRITE Alien[ALIEN_MAX_ALIEN];      // Alien sprites
  SPRITE Cannon;                      // Cannon sprite
  SPRITE Bomb[ALIEN_MAX_BOMBS];       // Bomb sprites
  SPRITE Shot[ALIEN_MAX_SHOTS];       // Shot sprites
  ICON Shield;                        // Shield icon
  WINDOW* hmsgbox;                    // Handle to message box
} ALIEN_GAME;