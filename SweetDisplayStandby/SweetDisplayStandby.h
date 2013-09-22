#pragma once

#include "resource.h"

#include <vector>
#include <Windows.h>
#include <lmaccess.h>

#include <powrprof.h>

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
using namespace std;

bool debug;

LASTINPUTINFO LastInput;
int idleTime;
int displayDimTime;
int displayOffTime;

bool displayWasDimmed;
bool displayWasTurnedOff;

BOOL pwrStatus;
EXECUTION_STATE cap;
NTSTATUS status;