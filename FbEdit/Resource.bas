#define IDD_DLGRESED            1300
#define IDC_RARESED             1301

Dim Shared ressize As WINSIZE=(300,170,0,52,100,100)

Function ResEdProc(ByVal hWin As HWND,ByVal uMsg As UINT,ByVal wParam As WPARAM,ByVal lParam As LPARAM) As Integer
	Dim rect As RECT
	Dim As Integer nInx,x,y
	Dim pt As Point
	Dim hMnu As HMENU
	Dim hDll As HMODULE
	Dim nBtn As Integer
	Dim tbxwt As Integer
	Dim lpCTLDBLCLICK As CTLDBLCLICK Ptr
	Dim fbcust As FBCUSTSTYLE
	Dim cust As CUSTSTYLE
	Dim fbrstype As FBRSTYPE
	Dim sType As ZString*32
	Dim sExt As ZString*64
	Dim sEdit As ZString*128
	Dim rarstype As RARSTYPE

	Select Case uMsg
		Case WM_INITDIALOG
			ah.hraresed=GetDlgItem(hWin,IDC_RARESED)
			SendMessage(ah.hraresed,DEM_SETSIZE,0,Cast(LPARAM,@ressize))
			SetDialogOptions(hWin)
			SendMessage(ah.hraresed,DEM_SETPOSSTATUS,Cast(Integer,ah.hsbr),0)
			nInx=1
			x=0
			While nInx<=32
				GetPrivateProfileString(StrPtr("CustCtrl"),Str(nInx),@szNULL,@buff,260,@ad.IniFile)
				If Len(buff) Then
					hDll=Cast(HMODULE,SendMessage(ah.hraresed,DEM_ADDCONTROL,0,Cast(Integer,@buff)))
					If hDll Then
						hCustDll(x)=hDll
						x=x+1
					EndIf
				EndIf
				nInx=nInx+1
			Wend
			nInx=1
			While nInx<=64
				fbcust.lpszStyle=@buff
				buff=""
				LoadFromIni(StrPtr("CustStyle"),Str(nInx),"044",@fbcust,FALSE)
				If Len(buff) Then
					cust.szStyle=buff
					cust.nValue=fbcust.nValue
					cust.nMask=IIf(fbcust.nMask,fbcust.nMask,fbcust.nValue)
					SendMessage(ah.hraresed,DEM_ADDCUSTSTYLE,0,Cast(LPARAM,@cust))
				EndIf
				nInx+=1
			Wend
			nInx=1
			While nInx<=32
				fbcust.lpszStyle=@buff
				fbrstype.lpsztype=@sType
				fbrstype.nid=0
				fbrstype.lpszext=@sExt
				fbrstype.lpszedit=@sEdit
				LoadFromIni(StrPtr("ResType"),Str(nInx),"0400",@fbrstype,FALSE)
				If Len(sType)<>0 Or fbrstype.nid<>0 Then
					ZStrReplace(@sExt,Asc("!"),Asc(","))
					rarstype.sztype=sType
					rarstype.nid=fbrstype.nid
					rarstype.szext=sExt
					rarstype.szedit=sEdit
					SendMessage(ah.hraresed,PRO_SETCUSTOMTYPE,nInx-1,@rarstype)
				EndIf
				nInx+=1
			Wend
			'
		Case WM_CLOSE
			DestroyWindow(hWin)
			'
		Case WM_DESTROY
			DestroyWindow(ah.hraresed)
			'
		Case WM_SIZE
			GetClientRect(hWin,@rect)
			MoveWindow(ah.hraresed,0,0,rect.right,rect.bottom,TRUE)
'		Case EM_GETMODIFY
'			Return SendMessage(ah.hraresed,PRO_GETMODIFY,0,0)
'			'
		Case EM_SETMODIFY
			SendMessage(ah.hraresed,PRO_SETMODIFY,wParam,0)
			'
		Case EM_UNDO
			SendMessage(ah.hraresed,DEM_UNDO,0,0)
			'
		Case EM_REDO
			SendMessage(ah.hraresed,DEM_REDO,0,0)
			'
		Case WM_CUT
			SendMessage(ah.hraresed,DEM_CUT,0,0)
			'
		Case WM_COPY
			SendMessage(ah.hraresed,DEM_COPY,0,0)
			'
		Case WM_PASTE
			SendMessage(ah.hraresed,DEM_PASTE,0,0)
			'
		Case WM_CLEAR
			SendMessage(ah.hraresed,DEM_DELETECONTROLS,0,0)
			'
		Case WM_NOTIFY
			lpCTLDBLCLICK=Cast(CTLDBLCLICK Ptr,lParam)
'			If (GetKeyState(VK_LBUTTON) And &H80)=0 Then
'				fTimer=1
'			EndIf
			If lpCTLDBLCLICK->nmhdr.code=NM_DBLCLK Then
				'TextToOutput(*lpCTLDBLCLICK->lpCtlName)
				'TextToOutput(*lpCTLDBLCLICK->lpDlgName)
				CallAddins(hWin,AIM_CTLDBLCLK,0,lParam,HOOK_CTLDBLCLK)
			EndIf
			If lpCTLDBLCLICK->nmhdr.code=NM_CLICK Then
				'TextToOutput(*lpCTLDBLCLICK->lpCtlName)
				'TextToOutput(*lpCTLDBLCLICK->lpDlgName)
				CallAddins(hWin,AIM_CTLDBLCLK,0,lParam,HOOK_CTLDBLCLK)
			EndIf
			ah.hrareseddlg=Cast(HWND,SendMessage(ah.hraresed,PRO_GETDIALOG,0,0))
			fTimer=1
			'
		Case WM_CONTEXTMENU
			If CallAddins(hWin,AIM_CONTEXTMEMU,wParam,lParam,HOOK_CONTEXTMEMU)=FALSE Then
				If lParam=-1 Then
					GetWindowRect(hWin,@rect)
					pt.x=rect.left+90
					pt.y=rect.top+90
				Else
					pt.x=Cast(Short,LoWord(lParam))
					pt.y=Cast(Short,HiWord(lParam))
				EndIf
				hMnu=GetSubMenu(ah.hcontextmenu,4)
				TrackPopupMenu(hMnu,TPM_LEFTALIGN Or TPM_RIGHTBUTTON,pt.x,pt.y,0,ah.hwnd,0)
			EndIf
		Case WM_SHOWWINDOW
			If ah.hfullscreen<>0 And fInUse=FALSE Then
				fInUse=TRUE
				If wParam Then
					If GetParent(hWin)<>ah.hfullscreen Then
						SetFullScreen(hWin)
					EndIf
				Else
					If GetParent(hWin)=ah.hfullscreen Then
						SetParent(hWin,ah.hwnd)
					EndIf
				EndIf
				fInUse=FALSE
			EndIf
		Case Else
			Return FALSE
			'
	End Select
	Return TRUE

End Function
