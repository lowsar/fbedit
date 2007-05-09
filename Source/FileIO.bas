
Function StreamIn(ByVal hFile As HANDLE,ByVal pBuffer As ZString ptr,ByVal NumBytes As Long,ByVal pBytesRead As Long ptr) As Boolean

	Return ReadFile(hFile,pBuffer,NumBytes,pBytesRead,0) Xor 1

End Function

Function StreamOut(ByVal hFile As HANDLE,ByVal pBuffer As ZString ptr,ByVal NumBytes As Long,ByVal pBytesWritten As Long ptr) As Boolean

	Return WriteFile(hFile,pBuffer,NumBytes,pBytesWritten,0) Xor 1

End Function

Function GetFileMem(ByVal sFile As String) As HGLOBAL
	Dim tci As TCITEM
	Dim lpTABMEM As TABMEM ptr
	Dim i As Integer
	Dim nlen As Integer
	Dim hMem As HGLOBAL
	Dim hFile As HANDLE
	Dim bread As Integer
	Dim hEdit As HWND

	tci.mask=TCIF_PARAM
	i=0
	Do While TRUE
		If SendMessage(ah.htabtool,TCM_GETITEM,i,Cast(Integer,@tci)) Then
			lpTABMEM=Cast(TABMEM ptr,tci.lParam)
			If lstrcmpi(@sFile,lpTABMEM->filename)=0 Then
				hEdit=lpTABMEM->hedit
				Exit Do
			EndIf
		Else
			Exit Do
		EndIf
		i=i+1
	Loop
	If hEdit Then
		nlen=SendMessage(hEdit,WM_GETTEXTLENGTH,0,0)+1
		hMem=MyGlobalAlloc(GMEM_FIXED Or GMEM_ZEROINIT,nlen+1)
		SendMessage(hEdit,WM_GETTEXT,nlen,Cast(Integer,hMem))
	Else
		hFile=CreateFile(@sFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)
		If hFile<>INVALID_HANDLE_VALUE Then
			nlen=GetFileSize(hFile,NULL)+1
			hMem=MyGlobalAlloc(GMEM_FIXED Or GMEM_ZEROINIT,nlen+1)
			ReadFile(hFile,hMem,nlen,@bread,NULL)
			CloseHandle(hFile)
		EndIf
	EndIf
	Return hMem

End Function

Sub ParseFile(ByVal hWin As HWND,ByVal hEdit As HWND,ByVal sFile As String)
	Dim nlen As Integer
	Dim hMem As HGLOBAL
	Dim fParse As Boolean
	Dim nInx As Integer
	Dim hFile As HANDLE
	Dim bread As Integer
	Dim chrg As CHARRANGE

	fParse=FALSE
	If fProject Then
		nInx=IsProjectFile(sFile)
		If nInx Then
			fParse=TRUE
		EndIf
	Else
		nInx=Cast(Integer,hEdit)
		fParse=TRUE
	EndIf
	If fParse Then
		If hEdit Then
			nlen=SendMessage(hEdit,WM_GETTEXTLENGTH,0,0)+1
			hMem=MyGlobalAlloc(GMEM_FIXED Or GMEM_ZEROINIT,nlen+1)
			SendMessage(hEdit,WM_GETTEXT,nlen,Cast(Integer,hMem))
			SetWindowLong(hEdit,GWL_USERDATA,0)
		Else
			hFile=CreateFile(sFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)
			If hFile<>INVALID_HANDLE_VALUE Then
				nlen=GetFileSize(hFile,NULL)+1
				hMem=MyGlobalAlloc(GMEM_FIXED Or GMEM_ZEROINIT,nlen+1)
				ReadFile(hFile,hMem,nlen,@bread,NULL)
				CloseHandle(hFile)
			EndIf
		EndIf
		If hMem Then
			SendMessage(ah.hpr,PRM_DELPROPERTY,nInx,0)
			SendMessage(ah.hpr,PRM_PARSEFILE,nInx,Cast(Integer,hMem))
			GlobalFree(hMem)
		EndIf
	EndIf

End Sub

Sub ReadTextFile(ByVal hWin As HWND,ByVal hFile As HANDLE,ByVal lpFilename As ZString ptr)
	Dim editstream As EDITSTREAM
	Dim szItem As ZString*260
	
	SendMessage(hWin,WM_SETTEXT,0,Cast(Integer,StrPtr("")))
	editstream.dwCookie=Cast(Integer,hFile)
	editstream.pfnCallback=Cast(Any ptr,@StreamIn)
	SendMessage(hWin,EM_STREAMIN,SF_TEXT,Cast(Integer,@editstream))
	SendMessage(hWin,REM_SETBLOCKS,0,0)
	SendMessage(hWin,EM_SETMODIFY,FALSE,0)
	lstrcpy(@szItem,lpFilename)
	If FileType(szItem)=1 Then
		' Set comment block definition
		SendMessage(ah.hred,REM_SETCOMMENTBLOCKS,Cast(Integer,StrPtr("/'")),Cast(Integer,StrPtr("'/")))
		UpdateAllTabs(3)
		If fProject<>FALSE And lstrlen(@ad.resexport) Then
			buff=MakeProjectFileName(ad.resexport)
			If lstrcmpi(@buff,lpFileName)=0 Then
				SetWindowLong(hWin,GWL_STYLE,GetWindowLong(hWin,GWL_STYLE) Or STYLE_READONLY)
			EndIf
		EndIf
	EndIf

End Sub

Sub ReadTheFile(ByVal hWin As HWND,ByVal lpFile As ZString ptr)
	Dim hFile As HANDLE
	Dim nSize As Integer
	Dim dwRead As Integer
	Dim hMem As HGLOBAL
	Dim lpRESMEM As RESMEM ptr

	hFile=CreateFile(lpFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)
	If hFile<>INVALID_HANDLE_VALUE Then
		If hWin=ah.hres Then
			nSize=GetFileSize(hFile,NULL)
			hMem=MyGlobalAlloc(GMEM_FIXED Or GMEM_ZEROINIT,nSize+1)
			ReadFile(hFile,hMem,nSize,@dwRead,NULL)
			CloseHandle(hFile)
			lpRESMEM=Cast(RESMEM ptr,GetWindowLong(hWin,0))
			SendMessage(lpRESMEM->hProject,PRO_OPEN,Cast(Integer,lpFile),Cast(Integer,hMem))
		Else
			ReadTextFile(hWin,hFile,lpFile)
			nLastLine=0
			CloseHandle(hFile)
		EndIf
	EndIf

End Sub

Sub BackupFile(ByVal szFileName As String,ByVal nBackup As Integer)
	Dim szBackup As ZString*260
	Dim szFile As ZString*260
	Dim szN As ZString*32
	Dim x As Integer

	szFile=GetFileName(szFileName,TRUE)
	If nBackup=1 Then
		szN="(1)"
		x=InStr(szFile,".")
		If x Then
			szFile=Left(szFile,x-1) & szN & Mid(szFile,x)
		Else
			szFile=szFile & szN
		EndIf
	Else
		x=InStr(szFile,"(" & Str(nBackup-1) & ")")
		If x Then
			szFile=Left(szFile,x) & Str(nBackup) & Mid(szFile,x+2)
		EndIf
	EndIf
	szBackup=ad.ProjectPath & "\Bak\" & szFile
	If nBackup<edtopt.backup Then
		If GetFileAttributes(@szBackup)<>-1 Then
			' File exist
			BackupFile(szBackup,nBackup+1)
		EndIf
	EndIf
	CopyFile(@szFileName,@szBackup,FALSE)

End Sub

Sub WriteTheFile(ByVal hWin As HWND,ByVal szFileName As String)
	Dim editstream As EDITSTREAM
	Dim hFile As HANDLE
	Dim hMem As HGLOBAL
	Dim nSize As Integer
	Dim lpRESMEM As RESMEM ptr
	Dim tpe As Integer
	Dim hREd As HWND

	If fProject=TRUE And edtopt.backup<>0 Then
		BackupFile(szFileName,1)
	EndIf
	hFile=CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0)
	If hFile<>INVALID_HANDLE_VALUE Then
		If hWin=ah.hres Then
			lpRESMEM=Cast(RESMEM ptr,GetWindowLong(ah.hres,0))
			hMem=MyGlobalAlloc(GMEM_FIXED Or GMEM_ZEROINIT,256*1024)
			SendMessage(lpRESMEM->hProject,PRO_EXPORT,0,Cast(Integer,hMem))
			nSize=lstrlen(Cast(ZString ptr,hMem))
			WriteFile(hFile,hMem,nSize,@nSize,NULL)
			CloseHandle(hFile)
			SendMessage(lpRESMEM->hProject,PRO_SETMODIFY,FALSE,0)
			GlobalFree(hMem)
			If fProject<>FALSE And lstrlen(ad.resexport)>0 Then
				SendMessage(lpRESMEM->hProject,PRO_SETEXPORT,(0 Shl 16)+nmeexp.nType,Cast(LPARAM,@ad.resexport))
				SendMessage(lpRESMEM->hProject,PRO_EXPORTNAMES,1,Cast(Integer,ah.hout))
				SendMessage(lpRESMEM->hProject,PRO_SETEXPORT,(nmeexp.nOutput Shl 16)+nmeexp.nType,Cast(LPARAM,@nmeexp.szFileName))
				buff=MakeProjectFileName(ad.resexport)
				If IsProjectFile(buff) Then
					ParseFile(ah.hwnd,0,buff)
					SendMessage(ah.hpr,PRM_REFRESHLIST,0,0)
				EndIf
				hREd=IsFileOpen(ah.hwnd,buff,FALSE)
				If hREd Then
					hFile=CreateFile(buff,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0)
					If hFile<>INVALID_HANDLE_VALUE Then
						ReadTextFile(hREd,hFile,buff)
						CloseHandle(hFile)
						nLastLine=0
					EndIf
				EndIf
			Else
				If nmeexp.fAuto Then
					SendMessage(lpRESMEM->hProject,PRO_EXPORTNAMES,1,Cast(Integer,ah.hout))
				EndIf
			EndIf
		Else
			tpe=FileType(szFileName)
			editstream.dwCookie=Cast(Integer,hFile)
			editstream.pfnCallback=Cast(Any ptr,@StreamOut)
			SendMessage(hWin,EM_STREAMOUT,SF_TEXT,Cast(Integer,@editstream))
			CloseHandle(hFile)
			If tpe=1 Then
				SetWindowLong(hWin,GWL_ID,IDC_CODEED)
				UpdateAllTabs(3)
			EndIf
		EndIf
		SendMessage(hWin,EM_SETMODIFY,FALSE,0)
	EndIf

End Sub

Sub SaveTempFile(ByVal hWin As HWND,ByVal szFileName As String)
	Dim editstream As EDITSTREAM
	Dim hFile As HANDLE

	If hWin<>ah.hres Then
		hFile=CreateFile(szFileName,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0)
		If hFile<>INVALID_HANDLE_VALUE Then
			editstream.dwCookie=Cast(Integer,hFile)
			editstream.pfnCallback=Cast(Any ptr,@StreamOut)
			SendMessage(hWin,EM_STREAMOUT,SF_TEXT,Cast(Integer,@editstream))
		EndIf
		CloseHandle(hFile)
	EndIf

End Sub

Function IsFileOpen(ByVal hWin As HWND,ByVal fn As String,ByVal fShow As Boolean) As HWND
	Dim tci As TCITEM
	Dim hOld As HWND
	Dim lpTABMEM As TABMEM ptr
	Dim i As Integer

	tci.mask=TCIF_PARAM
	i=0
	Do While TRUE
		If SendMessage(ah.htabtool,TCM_GETITEM,i,Cast(Integer,@tci)) Then
			lpTABMEM=Cast(TABMEM ptr,tci.lParam)
			If lstrcmpi(fn,lpTABMEM->filename)=0 Then
				If fShow Then
					hOld=ah.hred
					ah.hred=lpTABMEM->hedit
					ad.filename=lpTABMEM->filename
					SendMessage(hWin,WM_SIZE,0,0)
					If hOld<>ah.hred Then
						If ah.hfullscreen Then
							SetFullScreen(ah.hred)
						Else
							ShowWindow(ah.hred,SW_SHOW)
						EndIf
						ShowWindow(hOld,SW_HIDE)
					EndIf
					SendMessage(ah.htabtool,TCM_SETCURSEL,i,0)
					SetWinCaption(hWin)
					SetFocus(ah.hred)
				EndIf
				Return lpTABMEM->hedit
			EndIf
		Else
			Exit Do
		EndIf
		i=i+1
	Loop
	Return 0

End Function

