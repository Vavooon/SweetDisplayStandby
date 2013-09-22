// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SweetDisplayStandby.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


void AddToMonHandles(HANDLE h)
{
	int i, cnt;
	cnt = MonHandles.size();
	for (i = 0; i < cnt; i++)	
		if (MonHandles[i] == h) return;
	MonHandles.push_back(h);
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
    displayWasTurnedOff=state;
}

void setDisplayDimmed(bool state)
{
	int i, cnt;
	cnt = MonHandles.size();
	for (i = 0; i < cnt; i++)
	{
		SetMonitorBrightness(MonHandles[i], state ? 0 : 40);
	}
    displayWasDimmed=state;
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SWEETDISPLAYSTANDBY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SWEETDISPLAYSTANDBY));




	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SWEETDISPLAYSTANDBY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_SWEETDISPLAYSTANDBY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	displayDimTime = 30;
	displayOffTime = 5 * 60;

	debug = false;

	if (debug)
	{
		displayDimTime = 5;
		displayOffTime = 10;
	}

	displayWasDimmed = false;
	displayWasTurnedOff = false;
	SetTimer(hWnd, IDT_TIMER1, 1000, (TIMERPROC)NULL);
	EnumDisplayMonitors(NULL, NULL, EnumProc, NULL);


	//ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	return TRUE;
}




//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_TIMER:
	{
		LASTINPUTINFO LastInput = {};
		LastInput.cbSize = sizeof(LastInput);
		::GetLastInputInfo(&LastInput);
		int idleTime = (::GetTickCount() - LastInput.dwTime)/1000;
		if (idleTime>displayOffTime)
		{
			if (!displayWasTurnedOff)
			{
				status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
				if (!(cap & ES_DISPLAY_REQUIRED) && !(cap & ES_SYSTEM_REQUIRED))
				{
					setDisplayTurnedOff(true);
				}
			}
		}
		else if (idleTime>displayDimTime)
		{
			if (!displayWasDimmed)
			{
				status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
				if (!(cap & ES_DISPLAY_REQUIRED) && !(cap & ES_SYSTEM_REQUIRED))
				{
					setDisplayDimmed(true);
				}
			}
		}
		else
		{
			if (displayWasTurnedOff)
			{
				setDisplayTurnedOff(false);
			}
			if (displayWasDimmed)
			{
				setDisplayDimmed(false);
			}
		}
	}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
