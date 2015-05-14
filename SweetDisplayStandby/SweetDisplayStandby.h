#pragma once

#include "resource.h"

#include <vector>
#include <Windows.h>
#include <lmaccess.h>
#include <string>
#include <strsafe.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <time.h>
#include <powrprof.h>
#include <shellapi.h>

#include "PhysicalMonitorEnumerationAPI.h"  
#include "HighLevelMonitorConfigurationAPI.h" 
#include <lowlevelmonitorconfigurationapi.h>
#pragma comment(lib,"dxva2.lib")
#pragma comment(lib,"PowrProf.lib")

#define POWER_ON                    0x01 
#define POWER_STANDBY               0x02
#define POWER_SUSPEND               0x03
#define POWER_OFF                   0x04

#define IDT_TIMER1 1001

std::vector<HANDLE> MonHandles;


int screensaverTimeout;

int displayDimTime;
int displayOffTime;

bool displayWasDimmed;
bool displayWasTurnedOff;

const wchar_t className[] = L"PowerStatusClass";

HWND HWnd;

VOID CALLBACK TimerProc(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  UINT_PTR idEvent,
  _In_  DWORD dwTime
);

void ErrorExit(LPTSTR lpszFunction);