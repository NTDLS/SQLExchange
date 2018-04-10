///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _StatusDlg_H
#define _StatusDlg_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CStatusDlg{
	public:
		HWND Load(HWND hOwner, char *sTitle, int iIcon, bool bAllowCancel);
		HWND Hwnd(void);
		bool Unload(void);
		bool IsDlg(void);
		bool IsDlgVisible(void);
		bool SetIcon(int iIcon);
		bool SetCaption(char *sCaption);
		bool Show(void);
		bool Hide(void);
		bool SetText(char *sText);
		bool EnableButton(void);
		bool DisableButton(void);
		bool SetButtonText(char *sText);
		bool SetProgressPos(long lPos);
		bool SetProgressRange(long lMin, long lMax);

		CStatusDlg();
		~CStatusDlg();

	private:
		friend DWORD WINAPI CStatusDlg_Thread(LPVOID lpData);
		HANDLE CStatusDlg_Thread_Handle;
		DWORD CStatusDlg_Thread_ID;

		HWND hDialogHandle;
		HWND hOwnerHandle;
		HWND hText_hWnd;
		HWND hProgress_hWnd;
		HWND hCmdCancel_hWnd;

		HANDLE hEvent;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CStatusDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI CStatusDlg_Thread(LPVOID lpData);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

