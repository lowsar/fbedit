
;ProjectOption.dlg
IDD_DLGOPTION					equ 3000
IDC_EDTTEXT						equ 1003
IDC_EDTBINARY					equ 1004

.code

ProjectOptionProc proc hWin:HWND,uMsg:UINT,wParam:WPARAM,lParam:LPARAM

	mov		eax,uMsg
	.if eax==WM_INITDIALOG
		invoke SendDlgItemMessage,hWin,IDC_EDTTEXT,EM_LIMITTEXT,255,0
		invoke SetDlgItemText,hWin,IDC_EDTTEXT,offset szTxt
		invoke SendDlgItemMessage,hWin,IDC_EDTBINARY,EM_LIMITTEXT,255,0
		invoke SetDlgItemText,hWin,IDC_EDTBINARY,offset szBin
	.elseif eax==WM_COMMAND
		mov		edx,wParam
		movzx	eax,dx
		shr		edx,16
		.if edx==BN_CLICKED
			.if eax==IDOK
				invoke SendMessage,hWin,WM_CLOSE,NULL,NULL
			.elseif eax==IDCANCEL
				invoke SendMessage,hWin,WM_CLOSE,NULL,NULL
			.endif
		.endif
	.elseif eax==WM_CLOSE
		invoke EndDialog,hWin,NULL
	.else
		mov		eax,FALSE
		ret
	.endif
	mov		eax,TRUE
	ret

ProjectOptionProc endp
