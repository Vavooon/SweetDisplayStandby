// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SweetDisplayStandby.h"

#define MAX_LOADSTRING 100
#define DEBUG 0

using namespace std;



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//int wmId, wmEvent;
	//PAINTSTRUCT ps;
	//HDC hdc;

	switch (message)
	{
		case WM_POWERBROADCAST:
		
			MessageBox(hWnd, _T("Alerto!"), _T("Alerto!"), MB_OK );

		break;
		case IDM_EXIT:
		
			MessageBox(hWnd, _T("Alerto!"), _T("Alerto!"), MB_OK );

		break;
	}
	return 0;
}


void AddToMonHandles(HANDLE h)
{
	int i, cnt;
	cnt = MonHandles.size();
	for (i = 0; i < cnt; i++)	
		if (MonHandles[i] == h) return;
	MonHandles.push_back(h);
}




const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
	localtime_s(&tstruct, &now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%X %d.%m.%Y", &tstruct);

    return buf;
}
 
void write_text_to_log_file( const std::string &text )
{
    std::ofstream log_file("debug.log", std::ios_base::out | std::ios_base::app );
    log_file << currentDateTime() << std::string(" : ") << text << std::endl;
	//<< std::end;
}


BOOL CALLBACK EnumProc(
  HMONITOR hMonitor,
  HDC hdcMonitor,     
  LPRECT lprcMonitor, 
  LPARAM dwData       
)
{
	LPPHYSICAL_MONITOR pMons = NULL;
	DWORD i, mcnt;
	if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &mcnt))
	{
		return TRUE;
	}
    pMons = (LPPHYSICAL_MONITOR)malloc(mcnt * sizeof(PHYSICAL_MONITOR));
	if (GetPhysicalMonitorsFromHMONITOR(hMonitor, mcnt, pMons))
	{
		for (i = 0; i < mcnt; i++)
		{
			AddToMonHandles(pMons[i].hPhysicalMonitor);
		}
	}
	free(pMons);
	return TRUE;
}

void setDisplayTurnedOff(bool state)
{
	int i, cnt;
	cnt = MonHandles.size();
	for (i = 0; i < cnt; i++)
	{
		SetVCPFeature(MonHandles[i], 0xD6, state ? POWER_OFF : POWER_ON);
	}
}

void setDisplayDimmed(bool state)
{
	int i, cnt;
	cnt = MonHandles.size();
	for (i = 0; i < cnt; i++)
	{
		SetMonitorBrightness(MonHandles[i], state ? 0 : 40);
	}
}




int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	


	displayDimTime = 30;
	displayOffTime = 5 * 60;

	if (DEBUG)
	{
		displayDimTime = 5;
		displayOffTime = 10;
	}

	displayWasDimmed = false;
	displayWasTurnedOff = false;
	SetTimer(NULL, IDT_TIMER1, 1000, TimerProc);
	EnumDisplayMonitors(NULL, NULL, EnumProc, NULL);

	HMODULE hSelf = GetModuleHandle(nullptr);
	HWND sHWnd = CreateWindow(L"MozillaBatteryClass", L"Battery Watcher", 0, 0, 0, 0, 0, nullptr, nullptr, hSelf, nullptr);
	RegisterPowerSettingNotification(sHWnd, &GUID_MONITOR_POWER_ON, 0);

	ShowWindow(sHWnd, 0);
	SetWindowLongPtr(sHWnd, GWLP_WNDPROC, (LONG)WndProc);


	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = className;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc))
		return FALSE;


	HWnd = CreateWindowEx(
		0,                   /* Extended possibilites for variation */
		className,         /* Classname */
		L"Powerstatus",       /* Title Text */
		WS_OVERLAPPEDWINDOW, /* default window */
		CW_USEDEFAULT,       /* Windows decides the position */
		CW_USEDEFAULT,       /* where the window ends up on the screen */
		544,                 /* The programs width */
		375,                 /* and height in pixels */
		HWND_DESKTOP,        /* The window is a child-window to desktop */
		NULL,                /* No menu */
		hSelf,       /* Program Instance handler */
		NULL                 /* No Window Creation data */
		);


	HINSTANCE hinst = GetModuleHandle(NULL);

	WNDCLASS wnd;

	memset(&wnd, 0, sizeof(wnd));
	wnd.cbSize = sizeof(wnd);
	wnd.lpszClassName = "MainWClass";
	wnd.lpfnWndProc = MainWProc;
	wnd.hInstance = hinst;
	result = RegisterClassEx(&wnd);

	hwnd = CreateWindowEx
		(
		0, //extended styles
		wnd.lpszClassName, //class name
		"Main Window", //window name
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL | WS_MINIMIZEBOX, //style tags
		CW_USEDEFAULT, //horizontal position
		CW_USEDEFAULT, //vertical position
		CW_USEDEFAULT, //width
		CW_USEDEFAULT, //height
		(HWND)NULL, //parent window
		(HMENU)NULL, //class menu
		(HINSTANCE)wnd.hInstance, //some HINSTANCE pointer
		NULL //Create Window Data?
		);

	HICON HIcon = LoadIcon(0, IDI_WINLOGO);

	NOTIFYICONDATA NotifyIconData;
	memset(&NotifyIconData, 0, sizeof(NotifyIconData));
	NotifyIconData.cbSize = sizeof(NotifyIconData);
	NotifyIconData.uID = 1;
	NotifyIconData.hWnd = HWnd;
	NotifyIconData.hIcon = HIcon;
	NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE;
	//NotifyIconData.uCallbackMessage = WM_APP + 123;
	wcscpy_s(NotifyIconData.szTip, L"TrayTip");
	Shell_NotifyIcon(NIM_ADD, &NotifyIconData);
	ErrorExit(L"Win");


	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}



VOID CALLBACK TimerProc(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  UINT_PTR idEvent,
  _In_  DWORD dwTime
)
{
	LASTINPUTINFO LastInput = {};
	LastInput.cbSize = sizeof(LastInput);
	GetLastInputInfo(&LastInput);
	int idleTime = (GetTickCount() - LastInput.dwTime)/1000;

	if (idleTime>displayOffTime)
	{
		if (!displayWasTurnedOff)
		{
			write_text_to_log_file("Turn off");
			EXECUTION_STATE cap;
			NTSTATUS status;
			status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
			if (!(cap & ES_DISPLAY_REQUIRED))
			{
				displayWasTurnedOff=true;
				setDisplayTurnedOff(true);
				Sleep(1000);
			}
		}
	}
	else if (idleTime>displayDimTime)
	{
		if (!displayWasDimmed)
		{
			write_text_to_log_file("Dim");
			EXECUTION_STATE cap;
			NTSTATUS status;
			status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
			if (!(cap & ES_DISPLAY_REQUIRED))
			{
				displayWasDimmed=true;
				setDisplayDimmed(true);
				Sleep(2000);
			}
		}
	}
	else
	{
		if (displayWasTurnedOff)
		{
			write_text_to_log_file("Turn on");
			displayWasTurnedOff=false;
			setDisplayTurnedOff(false);
			Sleep(1000);
		}
		if (displayWasDimmed)
		{
			write_text_to_log_file("Undim");
			displayWasDimmed=false;
			setDisplayDimmed(false);
			Sleep(1000);
		}
	}
}


void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}