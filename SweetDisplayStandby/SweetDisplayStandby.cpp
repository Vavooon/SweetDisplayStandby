// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SweetDisplayStandby.h"

#define MAX_LOADSTRING 100
#define DEBUG 0

using namespace std;


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
	MSG msg;

	

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

	if (DEBUG)
	{
		ofstream debugFile;

		debugFile.open("log.txt");
		debugFile << idleTime << endl;
		debugFile.close();
	}

	if (idleTime>displayOffTime)
	{
		if (!displayWasTurnedOff)
		{
			EXECUTION_STATE cap;
			NTSTATUS status;
			status = CallNtPowerInformation(SystemExecutionState, NULL, 0, &cap, sizeof(cap));
			if (!(cap & ES_DISPLAY_REQUIRED))
			{
				displayWasTurnedOff=true;
				setDisplayTurnedOff(true);
				Sleep(2000);
			}
		}
	}
	else if (idleTime>displayDimTime)
	{
		if (!displayWasDimmed)
		{
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
			displayWasTurnedOff=false;
			setDisplayTurnedOff(false);
		}
		if (displayWasDimmed)
		{
			displayWasDimmed=false;
			setDisplayDimmed(false);
		}
	}
}
