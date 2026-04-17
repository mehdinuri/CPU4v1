									 //misc header
#include	"data.h"
#include "signalOutputCatch.h"
#include "signalCheck.h"
#include "CANRxTx.h"
#include <string.h>
#include "tim.h"
#include "iwdg.h"
#include "gpio.h"
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "adc.h"
#include "rng.h"
#include "crc.h"

#define CPMP_COMM_MAX_ERRORS 50

const unsigned long laValue2Bit[32] =
{
	0x00000001,
	0x00000002,
	0x00000004,
	0x00000008,
	0x00000010,
	0x00000020,
	0x00000040,
	0x00000080,
	0x00000100,
	0x00000200,
	0x00000400,
	0x00000800,
	0x00001000,
	0x00002000,
	0x00004000,
	0x00008000,
	0x00010000,
	0x00020000,
	0x00040000,
	0x00080000,
	0x00100000,
	0x00200000,
	0x00400000,
	0x00800000,
	0x01000000,
	0x02000000,
	0x04000000,
	0x08000000,
	0x10000000,
	0x20000000,
	0x40000000,
	0x80000000
};

#define WORKING_LAMP_CHANGE_ACCEPT_CNT_300_MS 24
#define WORKING_LAMP_CHANGE_ACCEPT_CNT_800_MS 75
#define WORKING_LAMP_CHANGE_ACCEPT_CNT_2_S 195

typedef enum
{
	LRLF_DETECT_TIME_NONE = 0,
	LRLF_DETECT_TIME_300_MS,
	LRLF_DETECT_TIME_800_MS,
	LRLF_DETECT_TIME_2_S,
	LRLF_DETECT_TIME_MAX,
	
} tELRLFDetectTime;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															Private Data															//
static tSMPProgramData SMPProgramData;
static tSMPProgramRuntimes SMPProgramRuntimes;
static tSCanPSMVoltMeas SCanPSMVoltMeas[SIG_DEV_PSM_MAX];		//	psm measurements states are copied into this buffer
static tSCanSSMVoltCurMeas SCanSSMVoltCurMeas[SIG_DEV_SSM_MAX];	//	ssm measurements states are copied into this buffer
static tSPSMEventCounters SPSMEventCounters;
static uint8_t bWorkingLampChangeAcceptCount = WORKING_LAMP_CHANGE_ACCEPT_CNT_800_MS;
static uint8_t fLampDimmingStateChanged = FALSE;

static tSCPCom SCPCom;
static tSADCBatteryRuntime SBatteryRuntime;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																				Battery Runtime
void BatteryRuntimeSet(tpSADCBatteryRuntime pSRuntime)
{
	memcpy(&SBatteryRuntime, pSRuntime, sizeof(SBatteryRuntime));
}

void BatteryRuntimeGet(tpSADCBatteryRuntime pSRuntime)
{
	memcpy(pSRuntime, &SBatteryRuntime, sizeof(SBatteryRuntime));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																				CP-MP Communication
uint8_t CPCommStateGet(void)
{
	return SCPCom.SFlags.fState;
}

void CPCommStateSet(uint8_t fState)
{
	SCPCom.SFlags.fState = fState;
}

void CPMPCommResetCntr(void)
{
	SCPCom.sErrorCntr = 0;
}

void CPMPCommIncCntr(void)
{
	SCPCom.sErrorCntr++;
	if (SCPCom.sErrorCntr > CPMP_COMM_MAX_ERRORS)
	{
		SCPCom.sErrorCntr = 0;
		CPCommStateSet(FALSE);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																				SSM & PSM Measurements
void	SetSSMVoltCurrentMeasurements(uint8_t bIdx, uint8_t* baData)
{
	memcpy(&SCanSSMVoltCurMeas[bIdx], baData, sizeof(tSCanSSMVoltCurMeas));
}

void	SSMVoltCurrentMeasurementsEvaluate(void)
{
	uint8_t	bSONoInSSM	= 0;
	uint8_t	bSGNoInSSM	= 0;
	uint8_t	bSSMNo		= 0;

	for (bSSMNo = 0; bSSMNo < SIG_DEV_SSM_MAX; bSSMNo++)
	{
		if (SOCatchIsCardActive(bSSMNo))
		{
			for (bSGNoInSSM = 0; bSGNoInSSM < SIGNAL_OUTPUT_CURRENT_GROUPS_PER_SSM; bSGNoInSSM++)
			{
				uint8_t	bSGNo	 = (bSSMNo * SIGNAL_OUTPUT_CURRENT_GROUPS_PER_SSM) + bSGNoInSSM;
				uint16_t	sCurrent = SCanSSMVoltCurMeas[bSSMNo].USOCurrentsL.baCurrentsL[bSGNoInSSM];

				// calculate the high byte of the current measurement, high bytes are 2 bits for all groups
				// and all of them is stores in only one byte so extract the related 2 bits for the bSGNoInSSM,
				// then shift left this value that shift value changes according to bSGNoInSSM
				sCurrent += (uint16_t)
					((((SCanSSMVoltCurMeas[bSSMNo].USOCurrentsH.bCurrentsH & laValue2Bit[2 * bSGNoInSSM]) + 
					(SCanSSMVoltCurMeas[bSSMNo].USOCurrentsH.bCurrentsH & laValue2Bit[2 * bSGNoInSSM + 1])) << (8-(bSGNoInSSM * 2))));

				SetCurrentMeasurement(bSGNo, sCurrent);
			}

			for (bSONoInSSM = 0; bSONoInSSM < SIGNAL_OUTPUTS_PER_SSM; bSONoInSSM++)
			{
				uint8_t	bSONo = (bSSMNo * SIGNAL_OUTPUTS_PER_SSM) + bSONoInSSM;

				// there are three switch states: previous, current, next
				// set switch states, signal check module examines SO states according to the previous states
				// because the data stored in variables belongs to the previous state of SOs
				// for software here, received data includes state info of the current state that will be assigned to
				// previous state below, also, SO's current state will be next state that is sent to SSM in last transmit
				SetSOSwitchState(bSONo, SIGNAL_OUTPUT_PREV_STATE, GetSOSwitchState(bSONo, SIGNAL_OUTPUT_CURRENT_STATE));
				SetSOSwitchState(bSONo,	SIGNAL_OUTPUT_CURRENT_STATE, GetSOSwitchState(bSONo, SIGNAL_OUTPUT_NEXT_STATE));

				// set voltage existence
				if (SCanSSMVoltCurMeas[bSSMNo].USOVoltages.sVoltages & laValue2Bit[bSONoInSSM])
				{
					SetSOVoltage(bSONo, TRUE);
				}
				else
				{
					SetSOVoltage(bSONo, FALSE);
				}
			}
		}
	}
}

void	SetPSMVoltageMeasurements(uint8_t bIdx, uint8_t* baData)
{
	memcpy(&SCanPSMVoltMeas[bIdx], baData, sizeof(tSCanPSMVoltMeas));
}

void	PSMVoltMeasurementsEvaluate(void)
{
	uint16_t sValue;
	uint8_t	bPSMNo;
	for (bPSMNo = 0; bPSMNo < (SIG_DEV_PSM_LAST - SIG_DEV_PSM_FIRST + 1); bPSMNo++)
	{
		if (SOCatchIsCardActive(bPSMNo + SIG_DEV_PSM_FIRST))
		{
			// 24V1
			sValue = SCanPSMVoltMeas[bPSMNo].b24V1L + (SCanPSMVoltMeas[bPSMNo].b24V1H << 8);
			Set24V1(bPSMNo, sValue);

			// 5V1
			sValue = SCanPSMVoltMeas[bPSMNo].b5V1L + (SCanPSMVoltMeas[bPSMNo].b5V1H << 8);
			Set5V1(bPSMNo, sValue);

			// 24V2
			sValue = SCanPSMVoltMeas[bPSMNo].b24V2L + (SCanPSMVoltMeas[bPSMNo].b24V2H << 8);
			Set24V2(bPSMNo, sValue);

			// 5V2
			sValue = SCanPSMVoltMeas[bPSMNo].b5V2L + (SCanPSMVoltMeas[bPSMNo].b5V2H << 8);
			Set5V2(bPSMNo, sValue);

			// Net Voltage
			sValue = SCanPSMVoltMeas[bPSMNo].bNetVoltageL + (SCanPSMVoltMeas[bPSMNo].bNetVoltageH << 8);
			SetNetVoltage(bPSMNo, sValue);

			// Net Frequency
			sValue = SCanPSMVoltMeas[bPSMNo].bNetFrequency;
			SetNetFrequency(bPSMNo, sValue);

			SetIsolatedVoltageState(bPSMNo, SCanPSMVoltMeas[bPSMNo].fIsolatedVoltage);
		}
	}
}

void SORuntimeFlagsGet(tSSORuntimeFlags SaFlags[])
{
	uint8_t bSONo;
	for (bSONo = 0; bSONo < SIGNAL_OUTPUTS_MAX; bSONo++)
	{
		memcpy(&SaFlags[bSONo], &SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags, sizeof(tSSORuntimeFlags));
	}
}

void	SGCurrentsGet(tSCurrentMeasurement SaCurrs[])
{
	uint8_t bSGNo;
	for (bSGNo = 0; bSGNo < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX; bSGNo++)
	{
		memcpy(&SaCurrs[bSGNo], &SMPProgramData.SaCurrents[bSGNo], sizeof(tSCurrentMeasurement));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																				Maintenance
void ClearResetFlags(void)
{
	__HAL_RCC_CLEAR_RESET_FLAGS();
}

void ClearStandbyFlag(void)
{
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
}

void ClearWakeupFlag(void)
{
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

void ClearAllFlags(void)
{
	ClearStandbyFlag();
	ClearWakeupFlag();
	ClearResetFlags();
}

void DisableDebug(void)
{
#ifndef DEBUG
	DBGMCU->CR = 0x00000000;
#endif
}

void EnableDebug(void)
{
#ifdef DEBUG
	HAL_DBGMCU_EnableDBGSleepMode();
	HAL_DBGMCU_EnableDBGStopMode();
	HAL_DBGMCU_EnableDBGStandbyMode();
#endif
}

void DisableInterruptRequests(void)
{
	__disable_irq();
}

void PrepareForStandbyMode(void)
{
	DisableDebug();
	
	CANDeInit(&hfdcan1);
	//CANDeInit(&hfdcan2);
	
	//RNGDeInit();
	
	//CRCDeInit();

	//I2CDeInit(&hi2c3);
	//I2CDeInit(&hi2c4);
	
	//SPIDeInit();
	
	Tim2DeInit();
	
	GPIODeInit();
	
	Tim1DeInit();
	
	HAL_RCC_DeInit();
}

void	EnterStandbyMode(void)
{
	// Enable the PWR clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	
	// Allow access to Backup */
  HAL_PWR_EnableBkUpAccess();
	
	// Disable all used wake-up sources: Pin1(PA.0) */
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

	// Clear PWR wake up Flag */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	// Enable WKUP pin */
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	
	// Enter Standby Mode
	HAL_PWR_EnterSTANDBYMode();
}

void EnterStandbyModeWithPreparation(uint8_t fPrep)
{
	IWDGSetMaxTimeout();
	DisableInterruptRequests();
	ClearAllFlags();
	
	if (fPrep)
	{
		PrepareForStandbyMode();
	}
	
	EnterStandbyMode();
	SystemReset();
}

void CheckWakeupOnReset(void)
{
	if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB))
	{
		IWDGSetMaxTimeout();
		ClearStandbyFlag();
		
		if (!__HAL_PWR_GET_FLAG(PWR_WAKEUP_PIN1))
		{
			ClearWakeupFlag();
			ClearResetFlags();
		
			EnterStandbyModeWithPreparation(FALSE);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															Peripherals														//
void PeripheralStatesSet(tpSPeripheralStates pSStates)
{
	memcpy(&SMPProgramData.SPeripheralStates, pSStates, sizeof(tSPeripheralStates));
}

tpSPeripheralStates PeripheralStatesGet(void)
{
	return &SMPProgramData.SPeripheralStates;
}

void	RelayStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fRelay = fState;
}

uint8_t		RelayStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fRelay;
}

void	GateStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fGate = fState;
}

uint8_t		GateStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fGate;
}

void	HeaterStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fHeater = fState;
}

uint8_t		HeaterStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fHeater;
}

void	ProgramRestartStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fProgramRestart = fState;
}

uint8_t		ProgramRestartStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fProgramRestart;
}

void	ClearSOPowersStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fClearSOPowers = fState;
}

uint8_t		ClearSOPowersStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fClearSOPowers;
}

void	LampDimmingStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fLampDimming = fState;
}

uint8_t		LampDimmingStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fLampDimming;
}

void 		LampDimmingStateChangedSet(uint8_t fState)
{
	fLampDimmingStateChanged = fState;
}

uint8_t 	LampDimmingStateChangedGet(void)
{
	return fLampDimmingStateChanged;
}

void	ExternalBatteryStateSet(uint8_t fState)
{
	SMPProgramData.SPeripheralStates.fExternalBattery = fState;
}

uint8_t	ExternalBatteryStateGet(void)
{
	return SMPProgramData.SPeripheralStates.fExternalBattery;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															Working Lamp Change Accept														//
uint8_t	GetWorkingLampChangesAcceptCount(void)
{
	return bWorkingLampChangeAcceptCount;
}

void	SetWorkingLampChangeAcceptCount(uint8_t bTime)
{
	switch(bTime)
	{
		case LRLF_DETECT_TIME_300_MS:
		{
			bWorkingLampChangeAcceptCount = WORKING_LAMP_CHANGE_ACCEPT_CNT_300_MS;
		}
		break;
		
		case LRLF_DETECT_TIME_800_MS:
		{
			bWorkingLampChangeAcceptCount = WORKING_LAMP_CHANGE_ACCEPT_CNT_800_MS;
		}
		break;
		
		case LRLF_DETECT_TIME_2_S:
		{
			bWorkingLampChangeAcceptCount = WORKING_LAMP_CHANGE_ACCEPT_CNT_2_S;
		}
		break;
		
		default:
		{
			bWorkingLampChangeAcceptCount = WORKING_LAMP_CHANGE_ACCEPT_CNT_800_MS;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															Public Methods															//
void	DataInit(void)
{
	memset(&SMPProgramData, 0, sizeof(tSMPProgramData));
	memset(&SMPProgramRuntimes, 0, sizeof(tSMPProgramRuntimes));
	memset(&SPSMEventCounters, 0, sizeof(SPSMEventCounters));

	uint8_t	bIndex;
	for (bIndex = 0; bIndex < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX; bIndex++)
	{
		SMPProgramData.SaCurrents[bIndex].sMin = 1024;
	}
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//signaling Mode
void	SetSignalingModeSet(uint8_t bSetNo, uint8_t bNewMode)
{
	if (bSetNo < SIGNAL_SETS_MAX)
		SMPProgramRuntimes.SaSetRuntime[bSetNo].bSignalingMode = bNewMode;
}

uint8_t	SetSignalingModeGet(uint8_t bSetNo)
{
	if (bSetNo < SIGNAL_SETS_MAX)
		return	SMPProgramRuntimes.SaSetRuntime[bSetNo].bSignalingMode;
	else
		return 0;
}

void	SetEmergencyMethodSet(uint8_t bMethod, uint8_t bSetNo)
{
	switch(bMethod)
	{
		case EMERGENCY_METHOD_DARK:
			if (bSetNo < SIGNAL_SETS_MAX)
				SetSignalingModeSet(bSetNo, SIGNALING_MODE_EMERGENCY_DARK);
		break;

		case EMERGENCY_METHOD_FLASH:
			if (bSetNo < SIGNAL_SETS_MAX)
				SetSignalingModeSet(bSetNo, SIGNALING_MODE_EMERGENCY_FLASH);
		break;
	}
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//signals
void	SignalSet(uint8_t bSignal, tpSSignalDef pSSignalDefBuffer)
{
	if (bSignal <= SIGNALS_MAX)
		memcpy(&SMPProgramData.SaSignalDefs[bSignal], pSSignalDefBuffer, sizeof(tSSignalDef));
}

/*
	name: is signal valid
	expl: first three bits are used to determined voltage values of so in sg, bit0 is red, bit1 is yellow, bit2 is green
*/
uint8_t	VoltagesValid(uint8_t bSGNo, uint8_t bVoltages)
{
	uint8_t	bSignal;
	uint8_t	bSetNo = SMPProgramData.SaSGDefs[bSGNo].bOwner;
	
	//scan signals, find a signal which has voltages
	for (bSignal = 1; bSignal <= SIGNALS_MAX; bSignal++)
	{
		//find signal according to voltages
		if (SignalMatchVoltages(bSignal, bVoltages))
		{
			//we have find a corresponding signal for the given voltages
			//provide validity according to current signaling mode of set
			switch (SetSignalingModeGet(bSetNo))
			{
				case SIGNALING_MODE_NORMAL:
					if(SMPProgramData.SaSignalDefs[bSignal - 1].SFlags.fValid)
						return TRUE;
				break;

				case SIGNALING_MODE_FLASH:
					if(SMPProgramData.SaSignalDefs[bSignal - 1].SFlags.fValidForFlash)
						return TRUE;
				break;

				case SIGNALING_MODE_EMERGENCY_FLASH:
					if(SMPProgramData.SaSignalDefs[bSignal - 1].SFlags.fValidForEmergencyFlash)
						return TRUE;
				break;

				case SIGNALING_MODE_EMERGENCY_DARK:
					if (SignalsDefinedDarkGet() == bSignal)
						return TRUE;
				break;

				default:
					break;
			}
		}	
	}
	return FALSE;
}

uint8_t	VoltagesCanFollowVoltages(uint8_t bCurrentVoltages, uint8_t bNextVoltages)
{
	if (SMPProgramData.SaSignalDefs[bCurrentVoltages].sFollowers & laValue2Bit[bNextVoltages])
		return TRUE;
	
	return FALSE;
}

uint8_t	SignalMatchVoltages(uint8_t bSignal, uint8_t bVoltages)
{
	if ((VoltagesHas(bVoltages, SIGNAL_OUTPUT_TYPE_RED) && (SignalHasRed(bSignal) == FALSE))	||
			((VoltagesHas(bVoltages, SIGNAL_OUTPUT_TYPE_RED) == FALSE) && SignalHasRed(bSignal))	||
			(VoltagesHas(bVoltages, SIGNAL_OUTPUT_TYPE_YELLOW) && (SignalHasYellow(bSignal) == FALSE))	||
			((VoltagesHas(bVoltages, SIGNAL_OUTPUT_TYPE_YELLOW) == FALSE) && SignalHasYellow(bSignal))	||
			(VoltagesHas(bVoltages, SIGNAL_OUTPUT_TYPE_GREEN) && (SignalHasGreen(bSignal) == FALSE))	||
			((VoltagesHas(bVoltages, SIGNAL_OUTPUT_TYPE_GREEN) == FALSE) && SignalHasGreen(bSignal)))
			return FALSE;
	return TRUE;
}

//return flash period of the subsignal
uint16_t	SubSignalHasFlash(uint8_t bSignal, uint8_t bOutputType)
{
	switch (bOutputType)
	{
		case SIGNAL_OUTPUT_TYPE_RED:
			if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_RED].sPeriod > FLASH_PERIOD_INFINITE)
				return SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_RED].sPeriod;
			break;

		case SIGNAL_OUTPUT_TYPE_YELLOW:
			if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_YELLOW].sPeriod > FLASH_PERIOD_INFINITE)
				return SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_YELLOW].sPeriod;
			break;

		case SIGNAL_OUTPUT_TYPE_GREEN:
			if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_GREEN].sPeriod > FLASH_PERIOD_INFINITE)
				return SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_GREEN].sPeriod;
			break;
	}
	return 0;
}

uint8_t	SignalHasFlash(uint8_t bSignal)
{
	uint8_t	bIndex;
	for (bIndex = 0; bIndex < SUBSIGNALS_MAX; bIndex++)
	{
		if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[bIndex].sPeriod > FLASH_PERIOD_INFINITE)
			return TRUE;
	}
	return FALSE;
}

uint8_t	SignalHasGreen(uint8_t bSignal)
{
	if (SMPProgramData.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_GREEN].sPeriod != FLASH_PERIOD_INFINITE)
		return TRUE;
	return FALSE;
}

uint8_t	SignalHasYellow(uint8_t bSignal)
{
	if (SMPProgramData.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_YELLOW].sPeriod != FLASH_PERIOD_INFINITE)
		return TRUE;
	return FALSE;
}

uint8_t	SignalHasRed(uint8_t bSignal)
{
	if (SMPProgramData.SaSignalDefs[bSignal - 1].SaSignal[SUBSIGNAL_RED].sPeriod != FLASH_PERIOD_INFINITE)
		return TRUE;
	return FALSE;
}

uint8_t	SignalVoltagesGet(uint8_t bSignal)
{
	uint8_t	bVoltages = 0;
	if (bSignal && (bSignal <= SIGNALS_MAX))
	{
		if (SMPProgramData.SaSignalDefs[bSignal-1].SFlags.fValid)
		{
			//this is a valid signal
			if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_RED].sPeriod != FLASH_PERIOD_INFINITE)
				bVoltages |= SIGNAL_OUTPUT_TYPE_RED;	//this signal is not off
			if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_YELLOW].sPeriod != FLASH_PERIOD_INFINITE)
				bVoltages |= SIGNAL_OUTPUT_TYPE_YELLOW;	//this signal is not off
			if (SMPProgramData.SaSignalDefs[bSignal-1].SaSignal[SUBSIGNAL_GREEN].sPeriod != FLASH_PERIOD_INFINITE)
				bVoltages |= SIGNAL_OUTPUT_TYPE_GREEN;	//this signal is not off
		}
	}
	return bVoltages;
}

uint8_t	VoltagesHas(uint8_t bVoltages, uint8_t bVoltage)
{
	if (bVoltages & bVoltage)
		return TRUE;
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Signals Defined																		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*(1)
	name: get/set methods
	expl: get or send signals defined
*/
void	SignalsDefinedSet(tpSSignalsDefined pSSignalsDefined)
{
	memcpy(&(SMPProgramData.SSignalsDefined), pSSignalsDefined, sizeof(tSSignalsDefined));
}

void	SignalsDefinedGet(tpSSignalsDefined pSSignalsDefined)
{
	memcpy(pSSignalsDefined, &(SMPProgramData.SSignalsDefined), sizeof(tSSignalsDefined));
}

/*(2)
	name: get blocking signal no
	expl: find blocking signal no between signals
*/
uint8_t	SignalsDefinedBlockingGet(void)
{
	return SMPProgramData.SSignalsDefined.bBlocking;
}

/*(3)
	name: get signal no used in phase
	expl: find phase signal no between signals
*/
uint8_t	SignalsDefinedFreeGet(void)
{
	return SMPProgramData.SSignalsDefined.bFree;
}

/*(4)
	name: get dark signal no
	expl: find dark signal no between signals
*/
uint8_t	SignalsDefinedGreenFlashGet(void)
{
	return SMPProgramData.SSignalsDefined.bGreenFlash;
}

/*(5)
	name: get dark signal no
	expl: find dark signal no between signals
*/
uint8_t	SignalsDefinedDarkGet(void)
{
	return SMPProgramData.SSignalsDefined.bDark;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// current voltage slope triples
void	GetCVSDef(tpSCVSDef pSCVSDef)
{
	memcpy(pSCVSDef, &SMPProgramData.SCVSDef, sizeof(tSCVSDef));
}

void	SetCVSDef(tpSCVSDef pSCVSDef)
{
	memcpy(&SMPProgramData.SCVSDef, pSCVSDef, sizeof(tSCVSDef));
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//signal outputs - definition
void		SetSODef(uint8_t bSONo, tpSSODef pSSODef)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		memcpy(&SMPProgramData.SaSODefs[bSONo], pSSODef, sizeof(tSSODef));
		SMPProgramRuntimes.SaSORuntimes[bSONo].bNoOfWorkingLamps = SMPProgramData.SaSODefs[bSONo].bNoOfLamps;
	}
}

uint8_t		GetSOType(uint8_t bSONo)
{
	return SMPProgramData.SaSODefs[bSONo].bType;
}

uint8_t		GetSOOwner(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramData.SaSODefs[bSONo].bOwner;
	return 0;
}

uint8_t		GetSONextOutput(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramData.SaSODefs[bSONo].bNextOutput;
	return 0;
}

uint8_t		GetSONoOfLamps(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramData.SaSODefs[bSONo].bNoOfLamps;
	return 0;
}

uint16_t	GetSOPower(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramData.SaSODefs[bSONo].sPower[(LampDimmingStateGet()) ? 0 : 1];
	return 0;
}

uint16_t	GetVoltage(void)
{
	uint8_t bIndex;

	for (bIndex = SIG_DEV_PSM_FIRST; bIndex <= SIG_DEV_PSM_LAST; bIndex++)
	{
		uint8_t bPSMNo = bIndex - SIG_DEV_PSM_FIRST;

		if (SOCatchIsCardActive(bIndex))
			return SMPProgramData.SaPSMs[bPSMNo].sNet;
	}
	return 0;
}

uint16_t	GetFrequency(void)
{
	uint8_t bIndex;

	for (bIndex = SIG_DEV_PSM_FIRST; bIndex <= SIG_DEV_PSM_LAST; bIndex++)
	{
		uint8_t bPSMNo = bIndex - SIG_DEV_PSM_FIRST;

		if (SOCatchIsCardActive(bIndex))
			return SMPProgramData.SaPSMs[bPSMNo].sFrequency;
	}
	return 0;
}

uint16_t	GetExpectedCurrent(uint8_t bSONo)
{
	uint8_t bSODimIdx = (LampDimmingStateGet()) ? 0 : 1;
	return	(
		((double)(SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] - GetVoltage()))	* 
		((double)(SMPProgramData.SaSODefs[bSONo].sSlope) / (double)1000)				+ 
		(SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx])
	);
}

void		SetOnlySOPower(uint8_t bSONo, uint16_t sNewPower)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		uint8_t bSODimIdx = (LampDimmingStateGet()) ? 0 : 1;
		SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx] = sNewPower;
		if (SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartMean)
		{
			SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] += GetVoltage();
		}
		else
			SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] = 0;
	}
}

uint16_t	GetSOMinPower(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		uint8_t bSODimIdx = (LampDimmingStateGet()) ? 0 : 1;
		if ((SMPProgramData.SaSODefs[bSONo].bNoOfLamps != 0) && (SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx] > SO_MIN_POWER))
			return ((uint16_t)
						(
							(((double)(GetVoltage() - SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] ))
								* ((double)(SMPProgramData.SaSODefs[bSONo].sSlope) / (double)(1000)) + (double)(SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx])) / 
							(SMPProgramData.SaSODefs[bSONo].bNoOfLamps)
						)
				   );
		else
			return	SO_MIN_POWER;
	}
	
	return 0;
}

uint8_t		RecordingPower(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		return (LampDimmingStateGet()) ? SMPProgramData.SaSODefs[bSONo].SFlags.bPowerRecorded0 : SMPProgramData.SaSODefs[bSONo].SFlags.bPowerRecorded1;
	}
	return FALSE;
}

void		SetPowerRecorded(uint8_t bSONo, uint8_t bPowerRecorded)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		if (LampDimmingStateGet())
			SMPProgramData.SaSODefs[bSONo].SFlags.bPowerRecorded0 = bPowerRecorded;
		else
			SMPProgramData.SaSODefs[bSONo].SFlags.bPowerRecorded1 = bPowerRecorded;
	}
}
	
uint8_t		GetStartRecord(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartRecord;
}

uint8_t		GetStartMean(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartMean;
}

void		SetStartRecord(uint8_t bSONo, uint8_t fValue)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartRecord = fValue;
}

void		SetStartMean(uint8_t bSONo, uint8_t fValue)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartMean = fValue;
}

uint8_t		GetMinCurrentRecordSamples(uint8_t bSONo)
{
	if(GetSOType(bSONo) == SIGNAL_OUTPUT_TYPE_RED)
		return MIN_CURRENT_RECORD_SAMPLES_FOR_RED_LAMPS;
	else
		return MIN_CURRENT_RECORD_SAMPLES_FOR_NONE_RED_LAMPS;
}

uint8_t		IncreasePowerRecordSamples(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		if (RecordingPower(bSONo)== FALSE) // if power is not recorded yet
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].bPowerRecordSampleCounter++; //increase recorded sample cout

			if (SMPProgramRuntimes.SaSORuntimes[bSONo].bPowerRecordSampleCounter >= GetMinCurrentRecordSamples(bSONo)) //if enough samples have been taken
			{
				uint8_t bSODimIdx = (LampDimmingStateGet()) ? 0 : 1 ;
				SMPProgramRuntimes.SaSORuntimes[bSONo].bPowerRecordSampleCounter = 0;
				SetPowerRecorded(bSONo, TRUE);

				SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx] /= (GetMinCurrentRecordSamples(bSONo) - MIN_CURRENT_RECORD_SAMPLES_BEFORE_START); // calculate mean current
				SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] /= GetMinCurrentRecordSamples(bSONo);// - MIN_CURRENT_RECORD_SAMPLES_BEFORE_START; // calculate mean net power
				SMPProgramData.SaSODefs[bSONo].sSlope = (int16_t)
					(((double)(
							((double)SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx] / 
											(
												(double)(SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] - SMPProgramData.SCVSDef.saVoltages[SMPProgramData.SaSODefs[bSONo].bLampType])	* 
												(SMPProgramData.SCVSDef.raSlopes[SMPProgramData.SaSODefs[bSONo].bLampType]) + 
												(double)(SMPProgramData.SCVSDef.saCurrents[SMPProgramData.SaSODefs[bSONo].bLampType])
											) 
							) * 
							(SMPProgramData.SCVSDef.raSlopes[SMPProgramData.SaSODefs[bSONo].bLampType])
							)
					)* 
					(double)1000);
												
				LogRequest(EVENT_SO_POWER_RECORD, (bSONo + 1), SMPProgramData.SaSODefs[bSONo].sPower[bSODimIdx], SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[bSODimIdx] * 0.73029);
			}
		}
		
		return RecordingPower(bSONo);
	}
	return FALSE;
}

void		ClearSOPower(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		SMPProgramData.SaSODefs[bSONo].sPower[0] = 65535;
		SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[0] = 65535;
		SMPProgramData.SaSODefs[bSONo].sPower[1] = 65535;
		SMPProgramData.SaSODefs[bSONo].sPowerRecordNet[1] = 65535;		

		SMPProgramRuntimes.SaSORuntimes[bSONo].bPowerRecordSampleCounter = 0;
		SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fRecordPower = FALSE;
		SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartRecord = FALSE;
		SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fStartMean = FALSE;
		
		SetPowerRecorded(bSONo, FALSE);
	}
}

uint8_t		GetSOEM(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramData.SaSODefs[bSONo].SFlags.bSOFailureEM;
	return EMERGENCY_METHOD_NONE;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//signal outputs - runtime
uint8_t		GetSONoOfWorkingLamps(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].bNoOfWorkingLamps;
	return 0;
}

void		SetSONoOfWorkingLamps(uint8_t bSONo, uint8_t bLampCount)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		if (bLampCount <= SMPProgramData.SaSODefs[bSONo].bNoOfLamps)
			SMPProgramRuntimes.SaSORuntimes[bSONo].bNoOfWorkingLamps = bLampCount;
	}
}

uint8_t		GetWorkingLampChangeCounter(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].bWorkingLampChangeCounter;
	return 0;
}

uint8_t		IncreaseWorkingLampChangeCounter(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].bWorkingLampChangeCounter++;
		return SMPProgramRuntimes.SaSORuntimes[bSONo].bWorkingLampChangeCounter;
	}
	return 0;
}

void		ClearWorkingLampChangeCounter(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		SMPProgramRuntimes.SaSORuntimes[bSONo].bWorkingLampChangeCounter = 0;
}

uint8_t		GetNoOfLastDetectedWorkingLamps(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].bNoOfLastDetectedWorkingLamps;
	return 0;
}

void		SetNoOfLastDetectedWorkingLamps(uint8_t bSONo, uint8_t bLampCount)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		if (bLampCount <= SMPProgramData.SaSODefs[bSONo].bNoOfLamps)
			SMPProgramRuntimes.SaSORuntimes[bSONo].bNoOfLastDetectedWorkingLamps = bLampCount;
	}
}

uint8_t		GetSOSwitchState(uint8_t bSONo, uint8_t bTimeSlice)
{
	if (bTimeSlice == SIGNAL_OUTPUT_PREV_STATE)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fSwitchPrevious;
	else if (bTimeSlice == SIGNAL_OUTPUT_CURRENT_STATE)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fSwitchCurrent;
	else if (bTimeSlice == SIGNAL_OUTPUT_NEXT_STATE)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fSwitchNext;
	return FALSE;
}

void		SetSOSwitchState(uint8_t bSONo, uint8_t bTimeSlice, uint8_t fSwitchOn)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
	{
		if (bTimeSlice == SIGNAL_OUTPUT_PREV_STATE)
			SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fSwitchPrevious = fSwitchOn;
		else if (bTimeSlice == SIGNAL_OUTPUT_CURRENT_STATE)
			SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fSwitchCurrent = fSwitchOn;
		else if (bTimeSlice == SIGNAL_OUTPUT_NEXT_STATE)
			SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fSwitchNext = fSwitchOn;
	}
}

void	SOIdentification(uint8_t	bSSMGroupNo, tpSCanCpuSO pSCanCpu)
{
	uint8_t	bSONo = 0, bLastSO0No = 0, bFirstSO0No = 0;

	switch(bSSMGroupNo)
	{
		case SO_SSM_GROUP_1:
		{
			bLastSO0No = 48,
			bFirstSO0No = 0;
		}
		break;

		case SO_SSM_GROUP_2:
		{
			bLastSO0No = 96,
			bFirstSO0No = 48;
		}
		break; 
	}

	for(bSONo = bFirstSO0No; bSONo < bLastSO0No; bSONo++)
	{
		if(bSONo < (bFirstSO0No + 32))
		{
			if(pSCanCpu->USOOn0.lSOOn & laValue2Bit[bSONo - bFirstSO0No])
				SetSOSwitchState(bSONo, SIGNAL_OUTPUT_NEXT_STATE, TRUE);
			else
				SetSOSwitchState(bSONo, SIGNAL_OUTPUT_NEXT_STATE, FALSE);
		}
		else
		{
			if(pSCanCpu->USOOn1.lSOOn & laValue2Bit[bSONo - (bFirstSO0No + 32)])
				SetSOSwitchState(bSONo, SIGNAL_OUTPUT_NEXT_STATE, TRUE);
			else
				SetSOSwitchState(bSONo, SIGNAL_OUTPUT_NEXT_STATE, FALSE);
		}
	}
}

uint8_t		GetSOVoltage(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fVoltage;
	
	return FALSE;
}

void		SetSOVoltage(uint8_t bSONo, uint8_t fLevel)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fVoltage = fLevel;
}

uint8_t		GetSOOnOffState(uint8_t bSONo)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		return SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fOn;
	
	return FALSE;
}

void		SetSOOnOffState(uint8_t bSONo, uint8_t fOn)
{
	if (bSONo < SIGNAL_OUTPUTS_MAX)
		SMPProgramRuntimes.SaSORuntimes[bSONo].SFlags.fOn = fOn;
}

uint16_t	GetCurrentMeasurement(uint8_t bCurrentGroupNo, uint8_t bOption)
{
	if (bCurrentGroupNo < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX)
	{
		switch (bOption)
		{
			case CURRENT_MIN:
				return SMPProgramData.SaCurrents[bCurrentGroupNo].sMin;

			case CURRENT_NOW:
				return SMPProgramData.SaCurrents[bCurrentGroupNo].sNow;

			case CURRENT_MAX:
				return SMPProgramData.SaCurrents[bCurrentGroupNo].sMax;

			case CURRENT_PREV:
				return SMPProgramData.SaCurrents[bCurrentGroupNo].sPrev;

			default:
				return 0;
		}
	}
	
	return 0;
}

void		SetCurrentMeasurement(uint8_t bCurrentGroupNo, uint16_t sNewValue)
{
	if (bCurrentGroupNo < SIGNAL_OUTPUT_CURRENT_GROUPS_MAX)
	{
		SMPProgramData.SaCurrents[bCurrentGroupNo].sPrev = SMPProgramData.SaCurrents[bCurrentGroupNo].sNow;
		SMPProgramData.SaCurrents[bCurrentGroupNo].sNow = sNewValue;
		if (sNewValue < SMPProgramData.SaCurrents[bCurrentGroupNo].sMin)
			SMPProgramData.SaCurrents[bCurrentGroupNo].sMin = sNewValue;
		if (sNewValue > SMPProgramData.SaCurrents[bCurrentGroupNo].sMax)
			SMPProgramData.SaCurrents[bCurrentGroupNo].sMax = sNewValue;
	}
}

uint8_t		IncrementEventCounter(uint8_t bSONo, uint32_t lIncrementEvents)
{
	//switch short circuit
	if (lIncrementEvents & EC_SSC_FIR)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitFirst < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitFirst++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitFirst >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	if (lIncrementEvents & EC_SSC_SEC)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitSecond < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitSecond++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitSecond >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	//switch open circuit
	if (lIncrementEvents & EC_SOC)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOpenCircuit < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOpenCircuit++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOpenCircuit >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
	
	//lamps driven externally
	if (lIncrementEvents & EC_LDE_FIR)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallyFirst < EVENT_LDE_FIR_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallyFirst++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallyFirst >= EVENT_LDE_FIR_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	if (lIncrementEvents & EC_LDE_SEC)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallySecond < EVENT_LDE_SEC_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallySecond++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallySecond >= EVENT_LDE_SEC_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	//voltage sensor failure
	if (lIncrementEvents & EC_VSF_FIR)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureFirst < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureFirst++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureFirst >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	if (lIncrementEvents & EC_VSF_SEC)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureSecond < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureSecond++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureSecond >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
	
	//signal output states
	if (lIncrementEvents & EC_SOS_FF)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOff < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOff++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOff >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	if (lIncrementEvents & EC_SOS_FFN)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOffOn < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOffOn++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOffOn >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_SOS_FN)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOn < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOn++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOn >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_SOS_NFF)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOff < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOff++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOff >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_SOS_NFN)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOn < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOn++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOn >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_SOS_NN)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOn < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOn++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOn >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}

	if (lIncrementEvents & EC_SOS_NNF_FIR)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffFirst < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffFirst++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffFirst >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_SOS_NNF_SEC)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffSecond < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffSecond++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffSecond >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	//all lamps broken
	if (lIncrementEvents & EC_RLB)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllRedLampsBroken < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllRedLampsBroken++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllRedLampsBroken >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_YLB)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllYellowLampsBroken < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllYellowLampsBroken++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllYellowLampsBroken >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_GLB)
	{
		if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllGreenLampsBroken < EVENT_REPEAT_MAX)
		{
			SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllGreenLampsBroken++;
			if(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllGreenLampsBroken >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	//voltage values
	if (lIncrementEvents & EC_VLB)
	{
		if(SPSMEventCounters.bLowerBound < EVENT_REPEAT_MAX)
		{
			SPSMEventCounters.bLowerBound++;
			if(SPSMEventCounters.bLowerBound >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_VUB)
	{
		if(SPSMEventCounters.bUpperBound < EVENT_REPEAT_MAX)
		{
			SPSMEventCounters.bUpperBound++;
			if(SPSMEventCounters.bUpperBound >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_VN)
	{
		if(SPSMEventCounters.bNormalLevel < EVENT_REPEAT_MAX)
		{
			SPSMEventCounters.bNormalLevel++;
			if(SPSMEventCounters.bNormalLevel >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	//frequency values
	if (lIncrementEvents & EC_FLB)
	{
		if(SPSMEventCounters.bLowerBoundFrequency < EVENT_REPEAT_MAX)
		{
			SPSMEventCounters.bLowerBoundFrequency++;
			if(SPSMEventCounters.bLowerBoundFrequency >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_FUB)
	{
		if(SPSMEventCounters.bUpperBoundFrequency < EVENT_REPEAT_MAX)
		{
			SPSMEventCounters.bUpperBoundFrequency++;
			if(SPSMEventCounters.bUpperBoundFrequency >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
		
	if (lIncrementEvents & EC_FN)
	{																  
		if(SPSMEventCounters.bNormalLevelFrequency < EVENT_REPEAT_MAX)
		{
			SPSMEventCounters.bNormalLevelFrequency++;
			if(SPSMEventCounters.bNormalLevelFrequency >= EVENT_REPEAT_MAX)
				return TRUE;
		}
		else
			return TRUE;
	}
	
	return FALSE;
}

void		InitEventCounters(uint8_t bSONo, uint32_t lInitEvents)
{
	//switch short circuit
	if (lInitEvents & EC_SSC_FIR)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitFirst = 0;
	}

	if (lInitEvents & EC_SSC_SEC)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bShortCircuitSecond = 0;
	}

	//switch open circuit
	if (lInitEvents & EC_SOC)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOpenCircuit = 0;
	}

	//lamps driven externally
	if (lInitEvents & EC_LDE_FIR)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallyFirst = 0;
	}

	if (lInitEvents & EC_LDE_SEC)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bLampsDrivenExternallySecond = 0;
	}

	//voltage sensor failure
	if (lInitEvents & EC_VSF_FIR)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureFirst = 0;
	}

	if (lInitEvents & EC_VSF_SEC)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bVoltageSensorFailureSecond = 0;
	}

	//signal output states
	if (lInitEvents & EC_SOS_FF)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOff = 0;
	}

	if (lInitEvents & EC_SOS_FFN)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOffOn = 0;
	}

	if (lInitEvents & EC_SOS_FN)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOn = 0;
	}

	if (lInitEvents & EC_SOS_NFF)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOff = 0;
	}

	if (lInitEvents & EC_SOS_NFN)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOn = 0;
	}

	if (lInitEvents & EC_SOS_NN)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOn = 0;
	}

	if (lInitEvents & EC_SOS_NNF_FIR)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffFirst = 0;
	}

	if (lInitEvents & EC_SOS_NNF_SEC)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffSecond = 0;
	}

	//all lamps broken
	if (lInitEvents & EC_RLB)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllRedLampsBroken = 0;
	}

	if (lInitEvents & EC_YLB)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllYellowLampsBroken = 0;
	}

	if (lInitEvents & EC_GLB)
	{
		SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bAllGreenLampsBroken = 0;
	}

	//voltage values
	if (lInitEvents & EC_VLB)
	{
		SPSMEventCounters.bLowerBound = 0;
	}

	if (lInitEvents & EC_VUB)
	{
		SPSMEventCounters.bUpperBound = 0;
	}

	if (lInitEvents & EC_VN)
	{
		SPSMEventCounters.bNormalLevel = 0;
	}

	//frequency values
	if (lInitEvents & EC_FLB)
	{
		SPSMEventCounters.bLowerBoundFrequency = 0;
	}
	
	if (lInitEvents & EC_FUB)
	{
		SPSMEventCounters.bUpperBoundFrequency = 0;
	}

	if (lInitEvents & EC_FN)
	{
		SPSMEventCounters.bNormalLevelFrequency = 0;
	}
}

uint8_t		GetEventOccured(uint8_t bEventCounter)
{
	if (bEventCounter >= EVENT_REPEAT_MAX)
		return TRUE;
	return FALSE;
}

uint8_t		GetEventOccured2(uint8_t bEventCounter)
{
	if (bEventCounter >= EVENT_REPEAT_MAX_2)
		return TRUE;
	return FALSE;
}

uint8_t		SOStateDetermined(uint8_t bSONo)
{
	if (	GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOff)		||
				GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOffOn)	||
				GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOffOn)		||
				GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOff)	||
				GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOffOn)	||
				GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOn)		||
				GetEventOccured2(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffFirst)||
				GetEventOccured(SMPProgramRuntimes.SaSORuntimes[bSONo].SEventCounters.bOnOnOffSecond)
		 )
			return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//signal groups - definition
uint8_t		SetSGDefinition(uint8_t bSGNo, tpSSGDef pSSGBuffer)
{
	if (bSGNo >= SIGNAL_GROUPS_MAX)
		return FALSE;
	if ((SMPProgramData.SaSGDefs[bSGNo].bType == SIGNAL_GROUP_TYPE_NONE) && (pSSGBuffer->bType == SIGNAL_GROUP_TYPE_NONE))
		return FALSE;
	else
	{
		if ((SMPProgramData.SaSGDefs[bSGNo].bType == SIGNAL_GROUP_TYPE_NONE) &&
			(pSSGBuffer->bType != SIGNAL_GROUP_TYPE_NONE))
			SMPProgramData.bSGTotal++;
		else if ((SMPProgramData.SaSGDefs[bSGNo].bType != SIGNAL_GROUP_TYPE_NONE) &&
			(pSSGBuffer->bType == SIGNAL_GROUP_TYPE_NONE))
			SMPProgramData.bSGTotal--;
		memcpy(&SMPProgramData.SaSGDefs[bSGNo], pSSGBuffer, sizeof(tSSGDef));
	}
	return TRUE;
}

uint8_t		SetSGConflict(tpSSGConflict pSSGConflict)
{
	if (pSSGConflict->bSG1 >= SIGNAL_GROUPS_MAX || pSSGConflict->bSG2 >= SIGNAL_GROUPS_MAX)
		return FALSE;
	else
	{
		SMPProgramData.SaSGDefs[pSSGConflict->bSG1].SaConflicts[pSSGConflict->bSG2].fConflict = pSSGConflict->bConflict;
		SMPProgramData.SaSGDefs[pSSGConflict->bSG2].SaConflicts[pSSGConflict->bSG1].fConflict = pSSGConflict->bConflict;
	}
	return TRUE;
}

uint8_t		ConflictingSG(uint8_t bSGNo1, uint8_t bSGNo2)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
			return (SMPProgramData.SaSGDefs[bSGNo1].SaConflicts[bSGNo2].fConflict) || (SMPProgramData.SaSGDefs[bSGNo2].SaConflicts[bSGNo1].fConflict);
	}
	return FALSE;
}

uint8_t		GetSGClearance(uint8_t bSGNo1, uint8_t bSGNo2)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
			return SMPProgramData.SaSGDefs[bSGNo1].SaConflicts[bSGNo2].bClearance;
	}
	return 0;
}

uint8_t		GetSGFirstOutput(uint8_t bSGNo)
{
	if (bSGNo < SIGNAL_GROUPS_MAX)
		return SMPProgramData.SaSGDefs[bSGNo].bFirstOutput;
	return 0;
}

uint8_t		GetSGRedLampFailureNumber(uint8_t bSGNo)
{
	if (bSGNo < SIGNAL_GROUPS_MAX)
		return SMPProgramData.SaSGDefs[bSGNo].bRedLampFailureNumber;
	return 0;
}

uint8_t		GetSGRedLampFailureNumberEM(uint8_t bSGNo)
{
	if (bSGNo < SIGNAL_GROUPS_MAX)
		return SMPProgramData.SaSGDefs[bSGNo].SEmergencyMethods.bRedLampFailureNumberEM;
	return EMERGENCY_METHOD_NONE;
}

uint8_t		GetSGLastRedLampFailureEM(uint8_t bSGNo)
{
	if (bSGNo < SIGNAL_GROUPS_MAX)
		return SMPProgramData.SaSGDefs[bSGNo].SEmergencyMethods.bLastRedLampFailureEM;
	return EMERGENCY_METHOD_NONE;
}

uint8_t		GetSGNoOfLamps(uint8_t bSGNo, uint8_t bOutputType)
{
	if (bSGNo < SIGNAL_GROUPS_MAX)
	{
		uint8_t	bLamps = 0;
		uint8_t	bSONo = SMPProgramData.SaSGDefs[bSGNo].bFirstOutput;
		while (bSONo)
		{
			if (SMPProgramData.SaSODefs[bSONo - 1].bType == bOutputType)
				bLamps += SMPProgramData.SaSODefs[bSONo - 1].bNoOfLamps;
			bSONo = SMPProgramData.SaSODefs[bSONo - 1].bNextOutput;
		}
		
		return bLamps;
	}
	
	return 0;
}

uint8_t		GetSGTotal(void)
{
	return SMPProgramData.bSGTotal;
}

uint8_t		GetSGOwner(uint8_t bSGNo)
{
	return SMPProgramData.SaSGDefs[bSGNo].bOwner;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//signal groups - runtime
uint8_t	GetYYConflictState(uint8_t bSGNo1, uint8_t bSGNo2)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
			if (SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lYYConflictDedected & laValue2Bit[bSGNo2])
				return TRUE;
	}
	return FALSE;
}

void	SetYYConflictState(uint8_t bSGNo1, uint8_t bSGNo2, uint8_t fState)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
		{
			if (fState)
				SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lYYConflictDedected |= laValue2Bit[bSGNo2];
			else
				SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lYYConflictDedected &= (~laValue2Bit[bSGNo2]);
		}
	}
}

uint8_t	GetYGConflictState(uint8_t bSGNo1, uint8_t bSGNo2)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
			if (SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lYGConflictDedected & laValue2Bit[bSGNo2])
				return TRUE;
	}
	return FALSE;
}

void	SetYGConflictState(uint8_t bSGNo1, uint8_t bSGNo2, uint8_t fState)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
		{
			if (fState)
				SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lYGConflictDedected |= laValue2Bit[bSGNo2];
			else
				SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lYGConflictDedected &= (~laValue2Bit[bSGNo2]);
		}
	}
}

uint8_t	GetGGConflictState(uint8_t bSGNo1, uint8_t bSGNo2)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
			if (SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lGGConflictDedected & laValue2Bit[bSGNo2])
				return TRUE;
	}
	return FALSE;
}

void	SetGGConflictState(uint8_t bSGNo1, uint8_t bSGNo2, uint8_t fState)
{
	if (bSGNo1 < SIGNAL_GROUPS_MAX)
	{
		if (bSGNo2 < SIGNAL_GROUPS_MAX)
		{
			if (fState)
				SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lGGConflictDedected |= laValue2Bit[bSGNo2];
			else
				SMPProgramRuntimes.SaSGRuntimes[bSGNo1].lGGConflictDedected &= (~laValue2Bit[bSGNo2]);
		}
	}
}

uint8_t	GetSGNoOfFailedLamps(uint8_t bSGNo, uint8_t bOutputType)
{
	if (bSGNo < SIGNAL_GROUPS_MAX)
	{
		uint8_t	bFailedLamps = 0;
		uint8_t	bSONo = SMPProgramData.SaSGDefs[bSGNo].bFirstOutput;
		while (bSONo)
		{
			if (SMPProgramData.SaSODefs[bSONo - 1].bType == bOutputType)
				bFailedLamps += SMPProgramData.SaSODefs[bSONo - 1].bNoOfLamps - SMPProgramRuntimes.SaSORuntimes[bSONo - 1].bNoOfWorkingLamps;
			bSONo = SMPProgramData.SaSODefs[bSONo - 1].bNextOutput;
		}
		
		return bFailedLamps;
	}
	
	return 0;
}

uint8_t	GetSGFlashSignal(uint8_t bSGNo)
{
	if (bSGNo < SMPProgramData.bSGTotal)
		return SMPProgramData.SaSGDefs[bSGNo].bFlashSignal;
	return 0;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// emergency methods
void	SetConflictsEMDef(tpSConflictsEM pSConflictsEM)
{
	memcpy(&SMPProgramData.SConflictsEM, pSConflictsEM, sizeof(tSConflictsEM));
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Flash Period in Emergency
void	SetFlashPeriodInEmergency(uint16_t sPeriod)
{
	SMPProgramData.sFlashPerInEm = sPeriod;
}

// conflicts
uint8_t	GetYYEM(void)
{
	return SMPProgramData.SConflictsEM.bYellowYellowEM;
}

uint8_t	GetYGEM(void)
{
	return SMPProgramData.SConflictsEM.bYellowGreenEM;
}

uint8_t	GetGGEM(void)
{
	return SMPProgramData.SConflictsEM.bGreenGreenEM;
}

// voltage error
uint8_t	GetVoltageLimitsEM(void)
{
	return SMPProgramData.SConflictsEM.bVoltageLimitsEM;
}

// invalid signals and invalid signal sequences
uint8_t	GetInvalidSignalEM(void)
{
	return SMPProgramData.SConflictsEM.bInvalidSignalEM;
}

uint8_t	GetInvalidSignalSequenceEM(void)
{
	return SMPProgramData.SConflictsEM.bInvalidSignalSequenceEM;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// set validness

uint8_t	IsSSMBelongsToSet(uint8_t bSetNo, uint8_t bSSMNo)
{
	uint8_t bSONo;

	//scan outputs on this ssm
	for (bSONo = (bSSMNo * SIGNAL_OUTPUTS_PER_SSM); bSONo < (bSSMNo+1) * SIGNAL_OUTPUTS_PER_SSM; bSONo++)
	{
		//find set of the sg this so belongs to
		if (SMPProgramData.SaSGDefs[SMPProgramData.SaSODefs[bSONo].bOwner-1].bOwner == bSetNo)
			return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//power
void	GetPowerSupplies(tpSPowerSupply pSPowerSupply)
{
	memcpy(pSPowerSupply, SMPProgramData.SaPSMs, sizeof(SMPProgramData.SaPSMs));
}

uint16_t	GetNetVoltage(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return SMPProgramData.SaPSMs[bPSMNo].sNet;
	else
		return 0;
}

uint16_t	GetNetPrevVoltage(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return (uint16_t)(SMPProgramData.SaPSMs[bPSMNo].sPrevNet);
	else
		return 0;
}

uint8_t	SetNetVoltage(uint8_t bPSMNo, uint16_t sNewVoltage)
{
	if (bPSMNo < PSMS_MAX)
	{
		if (sNewVoltage < VOLTAGE_VALUE_MAX)
		{
			SMPProgramData.SaPSMs[bPSMNo].sPrevNet = SMPProgramData.SaPSMs[bPSMNo].sNet;
			SMPProgramData.SaPSMs[bPSMNo].sNet = sNewVoltage;
			return TRUE;
		}
	}
	
	return FALSE;
}

uint8_t	SetNetFrequency(uint8_t bPSMNo, uint16_t sNewFrequency)
{
	if (bPSMNo < PSMS_MAX)
	{
		if (sNewFrequency < FREQUENCY_VALUE_MAX)
		{
			SMPProgramData.SaPSMs[bPSMNo].sPrevFrequency = SMPProgramData.SaPSMs[bPSMNo].sFrequency;
			SMPProgramData.SaPSMs[bPSMNo].sFrequency = sNewFrequency;
			return TRUE;
		}
	}
	
	return FALSE;
}

uint16_t	Get24V1(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return SMPProgramData.SaPSMs[bPSMNo].s24V1;
	else
		return 0;
}

uint8_t	Set24V1(uint8_t bPSMNo, uint16_t sNewVoltage)
{
	if (bPSMNo < PSMS_MAX)
	{
		if (sNewVoltage < VOLTAGE_VALUE_MAX)
		{
			SMPProgramData.SaPSMs[bPSMNo].s24V1 = sNewVoltage;
			return TRUE;
		}
	}
	
	return FALSE;
}

uint16_t	Get24V2(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return SMPProgramData.SaPSMs[bPSMNo].s24V2;
	else
		return 0;
}

uint8_t	Set24V2(uint8_t bPSMNo, uint16_t sNewVoltage)
{
	if (bPSMNo < PSMS_MAX)
	{
		if (sNewVoltage < VOLTAGE_VALUE_MAX)
		{
			SMPProgramData.SaPSMs[bPSMNo].s24V2 = sNewVoltage;
			return TRUE;
		}
	}
	
	return FALSE;
}

uint16_t	Get5V1(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return SMPProgramData.SaPSMs[bPSMNo].s5V1;
	else
		return 0;
}

uint8_t	Set5V1(uint8_t bPSMNo, uint16_t sNewVoltage)
{
	if (bPSMNo < PSMS_MAX)
	{
		if (sNewVoltage < VOLTAGE_VALUE_MAX)
		{
			SMPProgramData.SaPSMs[bPSMNo].s5V1 = sNewVoltage;
			return TRUE;
		}
	}
	return FALSE;
}

uint16_t	Get5V2(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return SMPProgramData.SaPSMs[bPSMNo].s5V2;
	else
		return 0;
}

uint8_t	Set5V2(uint8_t bPSMNo, uint16_t sNewVoltage)
{
	if (bPSMNo < PSMS_MAX)
	{
		if (sNewVoltage < VOLTAGE_VALUE_MAX)
		{
			SMPProgramData.SaPSMs[bPSMNo].s5V2 = sNewVoltage;
			return TRUE;
		}
	}
	return FALSE;
}

uint8_t	GetIsolatedVoltageState(uint8_t bPSMNo)
{
	if (bPSMNo < PSMS_MAX)
		return SMPProgramData.SaPSMs[bPSMNo].SFlags.fIsolatedVoltage;
	else
		return FALSE;
}

uint8_t	SetIsolatedVoltageState(uint8_t bPSMNo, uint8_t fNewState)
{
	if (bPSMNo < PSMS_MAX)
	{
		SMPProgramData.SaPSMs[bPSMNo].SFlags.fIsolatedVoltage = fNewState;
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// voltage state
uint8_t	GetVoltageState(void)
{
	return SMPProgramData.bVoltageState;
}

void	SetVoltageState(uint8_t bNewState)
{
	SMPProgramData.bVoltageState = bNewState;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// frequency state
uint8_t	GetFrequencyState(void)
{
	return SMPProgramData.bFrequencyState;
}

void	SetFrequencyState(uint8_t bNewState)
{
	SMPProgramData.bFrequencyState = bNewState;
}

void	RuntimesInit(void)
{
	uint8_t	bSONo = 0, bSGNo = 0;

	memset(&(SMPProgramRuntimes), 	0, sizeof(tSMPProgramRuntimes));
	for(bSONo = 0; bSONo < SIGNAL_OUTPUTS_MAX; bSONo++)
		SMPProgramRuntimes.SaSORuntimes[bSONo].bNoOfWorkingLamps = SMPProgramData.SaSODefs[bSONo].bNoOfLamps;

	SMPProgramData.bSGTotal = 0;
	for(bSGNo = 0; bSGNo < SIGNAL_GROUPS_MAX; bSGNo++)
	{
		if (SMPProgramData.SaSGDefs[bSGNo].bType != 0)
		{
			uint8_t	bSetNo = SMPProgramData.SaSGDefs[bSGNo].bOwner;
			SetSignalingModeSet(bSetNo, SIGNALING_MODE_NORMAL);
			SMPProgramData.bSGTotal++;
		}
	}
	
	SignalCheckRuntimeInit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															SG Runtimes																//
void	SGRuntimeLampFailRedSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailRed = fState;
}

uint8_t	SGRuntimeLampFailRedGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailRed;
}	

void	SGRuntimeLampFailYellowSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailYellow = fState;
}

uint8_t	SGRuntimeLampFailYellowGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailYellow;
}

void	SGRuntimeLampFailGreenSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailGreen = fState;
}

uint8_t	SGRuntimeLampFailGreenGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailGreen;
}

void	SGRuntimeLampFailRedAllSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailRedAll = fState;
}

uint8_t	SGRuntimeLampFailRedAllGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailRedAll;
}

void	SGRuntimeLampFailYellowAllSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailYellowAll = fState;
}

uint8_t	SGRuntimeLampFailYellowAllGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailYellowAll;
}

void	SGRuntimeLampFailGreenAllSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailGreenAll = fState;
}

uint8_t	SGRuntimeLampFailGreenAllGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailGreenAll;
}

void	SGRuntimeLampFailLastRedSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailLastRed = fState;
}

uint8_t	SGRuntimeLampFailLastRedGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailLastRed;
}

void	SGRuntimeLampFailNumberOfRedSet(uint8_t bSGNo, uint8_t fState)
{
	SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailNumberOfRed = fState;
}

uint8_t	SGRuntimeLampFailNumberOfRedGet(uint8_t bSGNo)
{
	return SMPProgramRuntimes.SaSGRuntimes[bSGNo].fLampFailNumberOfRed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//															SO Runtimes
void	SORuntimeFailNoneSet(uint8_t bSONo)
{
	memset(&SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents, 0, sizeof(SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents));
}

void	SORuntimeFailShortCircuitFirstSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fSCFirst = fState;
}

uint8_t	SORuntimeFailShortCircuitFirstGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fSCFirst;
}

void	SORuntimeFailShortCircuitSecondSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fSCSecond = fState;
}

uint8_t	SORuntimeFailShortCircuitSecondGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fSCSecond;
}

void	SORuntimeFailOpenCircuitSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fOpenCircuit = fState;
}

uint8_t	SORuntimeFailOpenCircuitGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fOpenCircuit;
}

void	SORuntimeFailDrivenExternallyFirstSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fLampsDEFirst = fState;
}

uint8_t	SORuntimeFailDrivenExternallyFirstGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fLampsDEFirst;
}

void	SORuntimeFailDrivenExternallySecondSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fLampsDESecond = fState;
}

uint8_t	SORuntimeFailDrivenExternallySecondGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fLampsDESecond;
}

void	SORuntimeVoltageSensorFailureFirstSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fVSFFirst = fState;
}

uint8_t	SORuntimeVoltageSensorFailureFirstGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fVSFFirst;
}

void	SORuntimeVoltageSensorFailureSecondSet(uint8_t bSONo, uint8_t fState)
{
	SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fVSFSecond = fState;
}

uint8_t	SORuntimeVoltageSensorFailureSecondGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].SEvents.fVSFSecond;
}

uint8_t	SORuntimePowerRecordSampleGet(uint8_t bSONo)
{
	return SMPProgramRuntimes.SaSORuntimes[bSONo].bPowerRecordSampleCounter;
}

void	LogRequest(uint8_t bEvent, uint8_t bParam, uint16_t sParam, uint32_t lParam)
{
	if (bEvent > EVENT_NONE && bEvent < EVENT_LAST)
	{
		tpSEvent pSEvent = (tpSEvent)osMemoryPoolAlloc(LogReqsMemPoolHandle, 0);
		if (pSEvent != NULL)
		{
			memset(pSEvent, 0, sizeof(tSEvent));
			
			pSEvent->bEvent = bEvent;
			pSEvent->bParam = bParam;
			pSEvent->sParam = sParam;
			pSEvent->lParam = lParam;
			
			if (osMessageQueuePut(LogReqsQueueHandle, &pSEvent, 0, 0) != osOK)
			{
				osMemoryPoolFree(LogReqsMemPoolHandle, pSEvent);
			}
		}
	}
}
