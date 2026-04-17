#ifndef __DATA_H__
#define __DATA_H__

#include "common.h"
#include "main.h"
#include "fdcan.h"
#include "adc.h"

// Version
#define		MAESTRO_MP_VERSION_ARG0	3
#define		MAESTRO_MP_VERSION_ARG1	4
#define		MAESTRO_MP_VERSION_ARG2	0
#define		MAESTRO_MP_VERSION_ARG3	1
#define		MAESTRO_MP_VERSION_ARG4	'B'

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Intersection Sets																	//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*(1)
*/

#define SIGNAL_SETS_MAX										3

/*(2)
	name: set data
	expl: one set can be divided into sets. All sets are managed by one signal program but sets are evaluated one by one. Sets can have 
		signaling mode different from each other's.
*/
typedef struct _SSetRuntime
{
	uint8_t	bSignalingMode;						//signaling mode of set
	
} tSSetRuntime, *tpSSetRuntime;

/*(3)
	name: signal modes
	expl: 
*/
#define	SIGNALING_MODE_NONE							0x00
#define	SIGNALING_MODE_NORMAL						0x01
#define SIGNALING_MODE_FLASH						0x02
#define SIGNALING_MODE_EMERGENCY_FLASH	0x08
#define SIGNALING_MODE_EMERGENCY_DARK		0x0A

#define SIGNALING_MODE_EMERGENCY_FLAG		0x08	//'bitwise and' with signaling method to understand if signaling mode is emergency one


/*(4)
	name: cause of signal modes
	expl: there are a number of cause of emergency methods invocation. Therefore, we need identifying this cause.
		we use events to identify these causes.
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Signals

/*(1)
	name: signal maximum values
	expl: there are three types of subsignal: red, yellow, and green. Below, referring to subsignals via indexing is provided. You can use
		these indexing mechanism while accessing subsignals in signal definition (see (3))
*/
#define SUBSIGNALS_MAX						3		//red, yellow and green
#define SUBSIGNAL_RED							0		//refer to red subsignal via this index
#define SUBSIGNAL_YELLOW					1		//refer to yellow subsignal via this index
#define SUBSIGNAL_GREEN						2		//refer to green subsignal via this index
#define SIGNALS_MAX								16	//user can define this number of custom signals

/*(2)
	name: sub-signal definition
	expl: sub signals have one of the colors: red, yellow and green.
*/
typedef struct _SSubSignalDef
{
	uint16_t	sPeriod;							//period for the color
	
} __attribute__((packed)) tSSubSignalDef, *tpSSubSignalDef;

/*(3)
	name: signal definition
	expl: A signal is the composition of three subsignals whose colors are red, yellow and green. All of them has a specific period. A signal
		may be valid for flash or emergency flash mode.

			- Examples

				valid				duration unlimited			min duration		max duration
				---------			------------------			-------------		-------------
				true				false						5					25
				true				true						5					not used
				false				not used					not used			not used

			- These signals can be called as custom signals. In other words, they are designed by the MCT user. 
			- Follower signals must also be defined by MCT user. The maximum number of custom signals is 16 at the time of programming. 
			Therefore, 2 bytes (16 bits) are used to show which signals can follow defined signal.
*/
typedef struct _SSignalDef
{
	struct
	{
		uint8_t	fValid									: 1;	//if it is usable for Maestro product, assign TRUE
		uint8_t	fValidForFlash					: 1;	//shows if it is valid for flash mode
		uint8_t	fValidForEmergencyFlash	: 1;	//shows if it is valid for emergency flash mode
		uint8_t	fDurationUnlimited			: 1;	//shows if there is no maxmimum duration limit for this signal
		uint8_t	fReserved								: 4;

	} __attribute__((packed)) SFlags;

	uint8_t bDummy;		//1 byte space for 2 bytes packing here
	tSSubSignalDef	SaSignal[SUBSIGNALS_MAX];	//all signals is a composition of three subsignals: red, yellow, green, use subsignal definitions to access the subsignal
	uint16_t sFollowers;			//signals that can follow this signal
	uint8_t bMinDur;				//this signal cannot have a duration smaller than this - in terms of seconds
	uint8_t bMaxDuration;		//this signal cannot have a duration greater than this - in terms of seconds

} __attribute__((packed)) tSSignalDef, *tpSSignalDef;


/*(4)
	name: predefined signals
	expl: some signals are predefined like red, green, green flash, and dark.
*/
typedef struct _SSignalsDefined
{
	uint8_t	bBlocking			: 4;	//red signal
	uint8_t	bFree					: 4;	//green signal
	uint8_t	bGreenFlash		: 4;	//green flash signal
	uint8_t	bDark					: 4;	//dark signal

} __attribute__((packed)) tSSignalsDefined, *tpSSignalsDefined;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//										Current/Voltage/Slope Reference Values															//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*(1)
	- For each lamp type, we have reference values which are sent from MCT user
	- At the time of programming, known lamp types are E27, Halogen, and LED.
*/

#define LAMP_CURRENT_PERCENTAGE				(0.70)

/*(2)
	name: types of lamps
	expl: these values are also used for access via indexing for the arrays in the following CVS definition.
*/
#define LAMP_TYPE_MAX						3

/*(3)
	name: current/voltage/slope reference values for each lamp type
	expl: 
*/
typedef struct _SCVSDef
{
	uint16_t saCurrents[LAMP_TYPE_MAX];
	uint16_t saVoltages[LAMP_TYPE_MAX];
	double raSlopes[LAMP_TYPE_MAX];

} __attribute__((packed)) tSCVSDef, *tpSCVSDef;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Signal Outputs																			//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*(1)
	- An array of signal outputs are defined in data.c. The length of this array is signal outputs maximum value, 96 at the time of 
	programming. This array is one dimensional. Besides, signal outputs on SSMs and SSMs have signal groups on them. An index value for this
	array refers to a physical output on the device. Examples are given about physical output access via this indexing mechanism.
		
		- Examples:
			
			index				SSM					signal group number on SSM				signal output on signal group
			-------				------				--------------------------				-----------------------------
			0					0					0										0
			22					1					3										1
			27					2					1										0
			50					4					0										2
			89					8					2										2
*/

//maximum number of physical outputs on the device
#define SIGNAL_OUTPUTS_MAX					96

//signal outputs are grouped on SSMs. Each output group consist of 3 outputs and these share the same current measurement circuit
#define SIGNAL_OUTPUTS_PER_SSM					12
#define SIGNAL_OUTPUT_CURRENT_GROUPS_MAX		32
#define SIGNAL_OUTPUT_CURRENT_GROUPS_PER_SSM	4
#define SIGNAL_OUTPUTS_PER_CURRENT_GROUP		3

/*(2)
	name: signal output definition
	expl: 
		- At the time of programming there are 3 types of lamps (See Current/Voltage/Slope Reference Values (2))
		- A signal may have one of the types: red, yellow, green (see (3))
		- A signal output belongs to a signal group
		- A signal output has a number of lamps. At the time of programming, the maximum number of these lamps is 5 (see (4))
		- A signal output consumes power and this value must be stored in its definition for the following comparision done for lamp-broken
		decisiob. At any time, MCT or LCD user can request to update this power value. In this case, device measures power consumed by signal
		output and record it again.
*/
typedef struct _SSODef
{
	uint8_t bLampType;						//	type of lamps at this output
	uint8_t bType;								//	one of the types defined below
	uint8_t bOwner;								//	owner signal group of this output (1.. SIGNAL_GROUPS_MAX)
	uint8_t bNextOutput;					//	index of the next output structure that belong to the same signal group
	uint8_t bNoOfLamps;						//	number of lamps connected to this output
	uint16_t sPower[2]  ;					//	total power consumed by the lamps connected to this output
	uint16_t sPowerRecordNet[2];		//	net voltage when sPower is recorded. Used later when determining lamp failures
	short sSlope;								//	slope for voltage-current line
	
	struct
	{
		uint8_t	bSOFailureEM : 2;	//	if bNoOfLamps is 1, use of this emergency method is meaningful
		uint8_t	bEMReserved : 2;
		uint8_t	bPowerRecorded0 : 1;
		uint8_t	bPowerRecorded1 : 1;
		uint8_t	bPowerReserved :	2;
		
	} __attribute__((packed)) SFlags;

} __attribute__((packed)) tSSODef, *tpSSODef; // 16 bytes

/*(3)
	name: signal output types
	expl: 
		- There are three types: red, yellow, green
		- Type values are defined so that each output type is represented with a bit. This makes matching outputs with signal group color 
		easier
*/
#define SIGNAL_OUTPUT_TYPE_NONE				0
#define SIGNAL_OUTPUT_TYPE_RED				1
#define SIGNAL_OUTPUT_TYPE_YELLOW			2
#define SIGNAL_OUTPUT_TYPE_GREEN			4

/*(4)
	name: the maximum number of lamps that can be connected to an output
	expl:
*/
#define LAMPS_PER_OUTPUT_MAX				5


/*(5)
	name: bit size for the event counters
	expl: 
		- Event counters are used to accept an event as triggered (or valid). 
		- When bit size value is modified, structure alignment must be considered in the following definition. Namely, reserved bits must
		be re-considered.
*/
#define EVENT_COUNTER_BIT_SIZE				8

/*(6)
	name: signal output runtime data
	expl: 
*/
typedef struct _SSORuntimeFlags
{
	uint8_t	fSwitchPrevious	: 1;			//one of the states listed below
	uint8_t	fSwitchCurrent	: 1;			//one of the states listed below
	uint8_t	fSwitchNext			: 1;			//one of the states listed below
	uint8_t	fVoltage				: 1;			//TRUE if lamp-on voltage is detected at this output
	uint8_t	fOn							: 1;			//TRUE if meets the criteria to be accepted as on
	uint8_t	fRecordPower		: 1;			//if TRUE, the power consumption from the output will be recorded instead of checking for failures
	uint8_t	fStartRecord		: 1;
	uint8_t	fStartMean			: 1;
	
} __attribute__((packed)) tSSORuntimeFlags, *tpSSORuntimeFlags;
	
typedef struct _SSORuntime
{
	tSSORuntimeFlags SFlags; //TRUE if turned on by the controller. previous state: the state for which mp has received measurements for
	
	struct
	{
		uint8_t	bShortCircuitFirst						: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bShortCircuitSecond						: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOpenCircuit									: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bLampsDrivenExternallyFirst		: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bLampsDrivenExternallySecond	: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bVoltageSensorFailureFirst		: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bVoltageSensorFailureSecond		: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOffOff												: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOffOffOn											: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOffOn												: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOnOn													: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOnOffOff											: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOnOffOn											: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOnOnOffSecond								: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bAllYellowLampsBroken					: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bAllGreenLampsBroken					: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bOnOnOffFirst									: EVENT_COUNTER_BIT_SIZE;
		uint8_t	bAllRedLampsBroken						: EVENT_COUNTER_BIT_SIZE;
	
	}	__attribute__((packed)) SEventCounters;

	struct 
	{
		uint8_t	fSCFirst			 							: 1;	// Short Circuit
		uint8_t	fSCSecond										: 1;
		uint8_t	fOpenCircuit								: 1;	// Open Circuit
		uint8_t	fLampsDEFirst								: 1;	// Driven Externally
		uint8_t	fLampsDESecond							: 1;
		uint8_t	fVSFFirst										: 1;	// Voltage Sensor Failure
		uint8_t	fVSFSecond									: 1;
		uint8_t	fReserved										: 1;
		
	} __attribute__((packed)) SEvents;

	uint8_t	bNoOfWorkingLamps;					//number of working lamps left on this output
	uint8_t	bNoOfLastDetectedWorkingLamps;		//count of last detected working lamps
	uint8_t	bWorkingLampChangeCounter;			//counter for working lamp change decision
	uint8_t	bPowerRecordSampleCounter;				//counter to be used while recording power. the minimum of this much samples will be recorded.

} __attribute__((packed)) tSSORuntime, *tpSSORuntime;

//signal output switch state periods
#define SIGNAL_OUTPUT_NEXT_STATE			0
#define SIGNAL_OUTPUT_CURRENT_STATE		1
#define SIGNAL_OUTPUT_PREV_STATE			2

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Current Measurement

#define SO_MIN_POWER									25

typedef struct _SCurrentMeasurement
{
	uint16_t	sMin;
	uint16_t	sNow;
	uint16_t	sMax;
	uint16_t	sPrev;
	
} __attribute__((packed)) tSCurrentMeasurement, *tpSCurrentMeasurement;

#define CURRENT_MIN										0
#define CURRENT_NOW										1
#define CURRENT_MAX										2
#define CURRENT_PREV									3

#define MIN_CURRENT_RECORD_SAMPLES_FOR_RED_LAMPS 48
#define	MIN_CURRENT_RECORD_SAMPLES_FOR_NONE_RED_LAMPS		48
#define MIN_CURRENT_RECORD_SAMPLES_BEFORE_START 20

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Signal Groups
#define SIGNAL_GROUPS_MAX							32

typedef struct _SSGDef
{
	uint8_t	bOwner;													//owner set of this group
	uint8_t	bType;
	uint8_t	bOpeningSignal;									//the signal displayed at the signal group while opening (after RED before GREEN)
	uint8_t	bClosingSignal;									//the signal displayed at the signal group while closing (after GREEN before RED)
	uint8_t	bOpeningDuration;								//opening (transition to green) color duration
	uint8_t	bClosingDur;										//closing (transition to red) color duration
	uint8_t	bGreenFlashDur;									//will display GREEN_FLASH this much seconds before closing
	uint8_t	bFlashSignal;										//signal to display in flash mode
	uint8_t	bFailureFlashSignal;						//signal to display in flash mode
	uint8_t	bFirstOutput;										//index of the first output that is in this signal group
	uint8_t	bRedLampFailureNumber;					//a failure is registered if this number of red lamps have failed on this signal group

	struct
	{
		uint8_t	fInUse									: 1;
		uint8_t	bRedLampFailureNumberEM	: 2;	//red lamp failure number emergency method
		uint8_t	bLastRedLampFailureEM		: 2;	//last red lamp failure emergency method
		uint8_t	bReserved								: 3;

	} __attribute__((packed)) SEmergencyMethods;

	struct
	{
		uint8_t	fConflict		: 1;							//if true, conflict checking will be made
		uint8_t	bClearance	: 7;							//duration needed before the sg related to this struct can finish opening state after conflicting one has finished closing state

	} __attribute__((packed)) SaConflicts[SIGNAL_GROUPS_MAX];			//definitions on conflicts with other signal groups

} __attribute__((packed)) tSSGDef, *tpSSGDef; // 44 bytes

typedef struct _SSGRuntime
{
	uint32_t	lYYConflictDedected;
	uint32_t	lYGConflictDedected;
	uint32_t	lGGConflictDedected;

	uint8_t	fLampFailRed				: 1;
	uint8_t	fLampFailYellow			: 1;
	uint8_t	fLampFailGreen			: 1;
	uint8_t	fLampFailRedAll			: 1;
	uint8_t	fLampFailYellowAll	: 1;
	uint8_t	fLampFailGreenAll		: 1;
	uint8_t	fLampFailLastRed		: 1;
	uint8_t	fLampFailAll				: 1;
	
	uint8_t	fLampFailNumberOfRed	:	1;
	uint8_t	fReserved							: 7;

} __attribute__((packed)) tSSGRuntime, *tpSSGRuntime;

#define SIGNAL_GROUP_TYPE_NONE 0
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Conflicts																				//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _SSGConflict
{
	uint8_t bSG1;
	uint8_t bSG2;
	uint8_t	bConflict;
	
} tSSGConflict, *tpSSGConflict;

typedef  struct _SConflictsEM
{
	uint8_t	bGreenGreenEM							: 2;
	uint8_t	bYellowGreenEM						: 2;
	uint8_t	bYellowYellowEM						: 2;
	uint8_t	bMalfunctionEM						: 2;

	uint8_t	bVoltageLimitsEM					: 2;
	uint8_t	bFrequencyErrorEM					: 2;
	uint8_t	bInvalidSignalEM					: 2;
	uint8_t	bInvalidSignalSequenceEM	: 2;

} __attribute__((packed)) tSConflictsEM, *tpSConflictsEM;

#define CONFLICT_NONE											0
#define CONFLICT_GREEN_GREEN							1
#define CONFLICT_YELLOW_GREEN							2
#define CONFLICT_YELLOW_YELLOW						3
#define CONFLICT_MALFUNCTION							4
#define CONFLICT_VOLTAGE_LIMIT						5
#define CONFLICT_FREQUENCY_ERROR					6
#define CONFLICT_INVALID_SIGNAL						7
#define CONFLICT_INVALID_SIGNAL_SEQUENCE	8

//emergency methods
#define EMERGENCY_METHOD_NONE				0
#define EMERGENCY_METHOD_DARK				1
#define EMERGENCY_METHOD_FLASH			2
#define EMERGENCY_METHODS_MAX				2
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														Flash Period																	//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FLASH_PERIOD_CONSTANT				10										//define in terms of 10ms
#define FLASH_PERIOD_0ms						0											//always on
#define FLASH_PERIOD_500ms					50
#define FLASH_PERIOD_1000ms					100
#define FLASH_PERIOD_2000ms					200
#define FLASH_PERIOD_4000ms					400
#define FLASH_PERIOD_MAX						FLASH_PERIOD_4000ms
#define FLASH_PERIOD_INFINITE				1											//always off

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														Peripheral States																//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// flags that control some peripherals which are controlled by the cards on the mp bus
typedef  struct _SPeripheralStates
{
	uint8_t	fRelay					: 1;
	uint8_t	fGate						: 1;
	uint8_t	fClearSOPowers	: 1;
	uint8_t	fCPUinProgress	: 1;	//Indicates the download / upload status of CPU
	uint8_t	fProgramRestart	: 1;
	uint8_t	fSystemInEM			: 1;
	uint8_t	fHeater					: 1;
	uint8_t	fLampDimming		: 1;
	
	uint8_t	fExternalBattery	: 1;
	uint8_t	fReserved : 7;

} __attribute__((packed)) tSPeripheralStates, *tpSPeripheralStates;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														Power Supply																	//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _SPowerSupply
{
	uint16_t	sPrevNet;
	uint16_t	sNet;
	uint16_t	s24V1;
	uint16_t	s5V1;
	uint16_t	s24V2;
	uint16_t	s5V2;
	
	struct _SFlags2
	{
		uint8_t	fIsolatedVoltage 	: 1;
		uint8_t	fReserved 				: 7;
		uint8_t	fReserved2;
		
	} SFlags;
	
	uint16_t	sPrevFrequency;
	uint16_t	sFrequency;
	
} tSPowerSupply, *tpSPowerSupply;

typedef struct _SPSMEventCounters
{
	uint8_t	bLowerBound : EVENT_COUNTER_BIT_SIZE;
	uint8_t	bUpperBound : EVENT_COUNTER_BIT_SIZE;
	uint8_t	bNormalLevel;
	uint8_t	bLowerBoundFrequency : EVENT_COUNTER_BIT_SIZE;
	uint8_t	bUpperBoundFrequency : EVENT_COUNTER_BIT_SIZE;
	uint8_t	bNormalLevelFrequency;
	
} tSPSMEventCounters, *tpSPSMEventCounters;

#define PSMS_MAX														2
#define VOLTAGE_VALUE_MAX										1024
#define FREQUENCY_VALUE_MAX									1024

#define NET_VOLTAGE_COEFF										0.332

// net voltage, 220V -> ADC output is 302
#define VOLTAGE_VALUE_HYSTERESIS						14		// 10 V
#define VOLTAGE_VALUE_LOWER_BOUND						241		// 176 V
#define VOLTAGE_VALUE_UPPER_BOUND						364		// 265 V

// net freqeuncy
#define FREQUENCY_VALUE_HYSTERESIS					3			// when it is exactly 100 hz again restart program
#define FREQUENCY_VALUE_LOWER_BOUND					95		// 4%
#define FREQUENCY_VALUE_UPPER_BOUND					105		// 4%

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//														Events																			//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _SEvent
{
	uint8_t		bEvent;
	uint8_t		bParam;
	uint16_t	sParam;
	uint32_t		lParam;	// it was uint32_t type, but this adds 2 additional bytes to the size of tSCPMPCommPacket
	
} tSEvent, *tpSEvent;

//event codes in the system
#define EVENT_NONE																0
#define EVENT_POWER_ON														1			//power on
#define EVENT_SO_SWITCH_SHORT_CIRCUIT							2			//signal outputs
#define EVENT_SO_SWITCH_OPEN_CIRCUIT							3
#define EVENT_SO_VOLTAGE_SENSOR_FAILURE						4
#define EVENT_SO_LAMPS_DRIVEN_EXTERNALLY					5
#define EVENT_SO_WORKING_LAMP_TOTAL_CHANGE				6
#define EVENT_SIGNAL_AT_SG												7			//if the dedected signal for an assigned signal changes
#define EVENT_INVALID_SIGNAL											8			//if an invalid signal that is not TRUE for intersection is on SO
#define EVENT_INVALID_SIGNAL_SEQUENCE							9			//if signal is not in followers
#define EVENT_SG_RED_LAMP_FAILURE									10		//when a red lamp failure exist
#define EVENT_SG_LAST_RED_LAMP_FAILURE						11		//the last red lamp failure
#define EVENT_SG_NUMBER_OF_RED_LAMPS_FAILURE			12		//bRedLampFailureNumberTH failure
#define EVENT_SG_YELLOW_LAMP_FAILURE							13
#define EVENT_SG_GREEN_LAMP_FAILURE								14
#define EVENT_YELLOW_YELLOW_CONFLICT							15		//conflicts
#define EVENT_YELLOW_GREEN_CONFLICT								16
#define EVENT_GREEN_GREEN_CONFLICT								17
#define EVENT_SO_POWER_RECORD											18		//power record
#define EVENT_MODULE_MISSING											19		//module missing
#define EVENT_MODULE_RESPONDS											20		//module responds
#define EVENT_INFORMATION													21		//random information
#define EVENT_CPMP_COMM_CP_CHECKSUM_ERROR					22		//cpmp comm checksum errors
#define EVENT_CPMP_COMM_MP_CHECKSUM_ERROR					23
#define EVENT_CPMP_COMM_CP_RECEIVE_ERROR					24		//cpmp comm receive/transmit error
#define EVENT_CPMP_COMM_CP_TRANSMIT_ERROR					25
#define EVENT_CPMP_COMM_MP_RECEIVE_ERROR					26
#define EVENT_CPMP_COMM_MP_TRANSMIT_ERROR					27
#define EVENT_POWER_NORMAL_TO_STAND_BY						28		//switching from normal mode to stand by
#define EVENT_POWER_STAND_BY_TO_NORMAL						29		//switching from stand by to normal mode
#define EVENT_CHECKSUM_FLASH_ERROR								30		//storage error
#define EVENT_CPMP_COMM_CP_TIMEOUT								31		//cpmp comm timeouts
#define EVENT_CPMP_COMM_MP_TIMEOUT								32
#define EVENT_MCT_CONFIGURATION_ERROR							33		//program loading
#define EVENT_PROGRAM_LOADING_ERROR								34
#define EVENT_PROGRAM_DAMAGED											35
#define EVENT_VOLTAGE_VALUE_LOWER_BOUND						36		//voltage bounds
#define EVENT_VOLTAGE_VALUE_UPPER_BOUND						37
#define EVENT_WORK_MODE_CHANGE										38		//work mode, EVENT_WORK_MODE_CHANGE
/* RESET EVENTS */
#define EVENT_RESET_WINDOW_WATCHDOG								39		//reset sources
#define EVENT_RESET_INDEPENDENT_WATCHDOG					40
#define EVENT_RESET_LOW_POWER											41
#define EVENT_RESET_SOFTWARE											100
#define EVENT_RESET_PIN														101
#define EVENT_RESET_PORRST												102
/*--------------*/
#define EVENT_SSM_LOG															42		//module logs
#define EVENT_PSM_LOG															43
#define EVENT_IO_LOG															44
#define EVENT_SG_ALL_RED_LAMPS_BROKEN							45		//lamps broken
#define EVENT_SG_ALL_YELLOW_LAMPS_BROKEN					46
#define EVENT_SG_ALL_GREEN_LAMPS_BROKEN						47
#define EVENT_SET_SIGNALING_MODE_CHANGE						48		//signaling mode
#define EVENT_MAIN_STORAGE_BROKEN									49		//main and backup storages
#define EVENT_BACKUP_STORAGE_BROKEN								50
#define EVENT_BACKUP_TO_MAIN_COPY_ERROR						51
#define EVENT_BACKUP_STORAGE_GET_ERROR						52
#define EVENT_BACKUP_STORAGE_SET_ERROR						53
#define EVENT_MAIN_STORAGE_GET_ERROR							54
#define EVENT_MAIN_STORAGE_SET_ERROR							55
#define EVENT_BACKUP_TO_MAIN_COPY_SUCCESS					56
#define EVENT_MAIN_STORAGE_IN_USE									57
#define EVENT_BACKUP_STORAGE_IN_USE								58
#define EVENT_MAIN_TO_BACKUP_COPY_SUCCESS					59
#define EVENT_MAIN_TO_BACKUP_COPY_ERROR						60
#define EVENT_RESET_POWER_ON_CLEAR_CIRCUIT				61		//reset source is power on clear circuit
#define EVENT_BATTERY_LOW													62		//battery
#define EVENT_BATTERY_NORMAL											63
#define EVENT_DOOR_OPEN														64		//door
#define EVENT_DOOR_CLOSED													65
#define EVENT_MCT_CONFIGURATION_STARTS						66		//Maestro Configuration Tool (MCT)
#define EVENT_MCT_CONFIGURATION_ENDS							67
#define EVENT_DEFAULT_LCD_USER_ADD_SUCCESS				68		//LCD user
#define EVENT_DEFAULT_LCD_USER_ADD_ERROR					69
#define EVENT_VOLTAGE_VALUE_NORMAL								70		//voltage normal value
#define EVENT_FREQUENCY_VALUE_LOWER_BOUND					71		//frequency
#define EVENT_FREQUENCY_VALUE_UPPER_BOUND					72
#define EVENT_FREQUENCY_VALUE_NORMAL							73
#define EVENT_USER_REQ_WORK_MODE_TO_ALL_RED				74		//user requests
#define EVENT_USER_REQ_WORK_MODE_TO_DARK					75
#define EVENT_USER_REQ_WORK_MODE_TO_FLASH					76
#define EVENT_USER_REQ_WORK_MODE_TO_WORK_PLAN			77
#define EVENT_USER_REQ_POWER_LEARNING							78
#define EVENT_USER_REQ_SSM_TEST_STARTS						79
#define EVENT_USER_REQ_SSM_TEST_ENDS							80
#define EVENT_USER_REQ_SP_TEST_STARTS							81
#define EVENT_USER_REQ_SP_TEST_ENDS								82
#define EVENT_USER_REQ_TIME_SET										83
#define EVENT_USER_REQ_RELAY_SET_ON								84
#define EVENT_USER_REQ_RELAY_SET_OFF							85
#define EVENT_USER_REQ_LCD_LOG_IN									86		//user account operations
#define EVENT_USER_REQ_LCD_LOG_OUT								87
#define EVENT_USER_REQ_LCD_LOG_IN_USERNAME_ERR		88
#define EVENT_USER_REQ_LCD_LOG_IN_PASSWORD_ERR		89
#define EVENT_SIGNAL_DURATION_LESS_THAN_MIN				90		//signal duration
#define EVENT_SIGNAL_DURATION_GREATER_THAN_MAX		91
#define EVENT_DETECTOR_BROKEN											92		//detector states
#define EVENT_DETECTOR_SAFE												93
#define EVENT_WORK_PLAN_CHANGE									94
#define EVENT_SIGNAL_PROGRAM_PLAN_CHANGE						95
#define EVENT_SIGNAL_PROGRAM_CHANGE								96
#define EVENT_SG_ALL_RED_LAMPS_SAFE								97		//lamps safe
#define EVENT_SG_ALL_YELLOW_LAMPS_SAFE							98
#define EVENT_SG_ALL_GREEN_LAMPS_SAFE							99
#define EVENT_RESET_SOFTWARE									100
#define EVENT_RESET_PIN											101
#define EVENT_RESET_PORRST										102
#define	EVENT_MCTS_ACTIVE										103
#define EVENT_MCTS_CONNECTED									104
#define EVENT_MCTS_DISCONNECTED									105
#define EVENT_MCTS_CONNECTION_TIMEOUT							106
#define	EVENT_MCTS_USER_REQUEST_SP_CHANGE						107
#define	EVENT_MCTS_USER_REQUEST_DATE_TIME_ADJUST				108
#define	EVENT_MCTS_USER_REQUEST_RESET							109
#define	EVENT_MCTS_USER_REQUEST_DOWNLOAD						110
#define	EVENT_MCTS_USER_REQUEST_UPLOAD							111
#define	EVENT_USER_REQ_PSM_TEST_STARTS							112
#define	EVENT_USER_REQ_PSM_TEST_ENDS							113
#define	EVENT_GREEN_WAVE_SYNCH_STARTS							114
#define	EVENT_GREEN_WAVE_SYNCH_ENDS								115
#define	EVENT_MCTS_USER_REQ_WORK_MODE_TO_ALL_RED				116
#define	EVENT_MCTS_USER_REQ_WORK_MODE_TO_DARK					117
#define	EVENT_MCTS_USER_REQ_WORK_MODE_TO_FLASH					118
#define	EVENT_MCTS_USER_REQ_WORK_MODE_TO_WORK_PLAN				119
#define EVENT_MCTS_USER_REQ_START_IAP							120
#define	EVENT_MCTS_RESUMED										121
#define	EVENT_USER_REQ_START_IAP								122
#define	EVENT_USER_REQ_RESET												123
#define EVENT_DIGITAL_INPUT_BROKEN									124		//digital states
#define EVENT_DIGITAL_INPUT_SAFE										125
#define EVENT_TASK_NOT_RUNNING											126
#define EVENT_TASK_STACK_OVERFLOW											127
#define EVENT_LAST												EVENT_TASK_STACK_OVERFLOW

// counter labels, for counters a new methodology should be constructed
#define SO_STATE_NONE															50
#define SO_STATE_OFF_OFF													51
#define SO_STATE_OFF_OFF_ON												52
#define SO_STATE_OFF_ON														53
#define SO_STATE_ON_OFF_OFF												54
#define SO_STATE_ON_OFF_ON												55
#define SO_STATE_ON_ON														56
#define SO_STATE_ON_ON_OFF_FIRST									57
#define SO_STATE_ON_ON_OFF_SECOND									58

#define SO_EVENT_SWITCH_SHORT_CIRCUIT_FIRST				59
#define SO_EVENT_SWITCH_SHORT_CIRCUIT_SECOND			60
#define SO_EVENT_SWITCH_OPEN_CIRCUIT							61
#define SO_EVENT_VOLTAGE_SENSOR_FAILURE_FIRST			63
#define SO_EVENT_VOLTAGE_SENSOR_FAILURE_SECOND		64
#define SO_EVENT_LAMPS_DRIVEN_EXTERNALLY_FIRST		65
#define SO_EVENT_LAMPS_DRIVEN_EXTERNALLY_SECOND		66

#define PSM_EVENT_VOLTAGE_VALUE_LOWER_BOUND				67
#define PSM_EVENT_VOLTAGE_VALUE_UPPER_BOUND				68
#define PSM_EVENT_VOLTAGE_VALUE_NORMAL						69

#define PSM_EVENT_FREQUENCY_VALUE_LOWER_BOUND			70
#define PSM_EVENT_FREQUENCY_VALUE_UPPER_BOUND			71
#define PSM_EVENT_FREQUENCY_VALUE_NORMAL					72

//when an event is repeated for these times, it is accepted that event is occured
#define EVENT_REPEAT_MAX							0x0F 	// original value is 0x07
#define EVENT_REPEAT_MAX_2						0x0F	 // original value is 250 - 250ms

#define EVENT_LDE_FIR_REPEAT_MAX 			0x2D // EVENT_REPEAT_MAX * 3;
#define EVENT_LDE_SEC_REPEAT_MAX			0x2D // EVENT_REPEAT_MAX_2 * 3;

//there are counters for the following events
#define EC_SSC_FIR							bit0	//switch short circuit, FIR=first, SEC=second
#define EC_SSC_SEC							bit1
#define EC_SOC									bit2	//switch open circuit
#define EC_LDE_FIR							bit3	//lamps driven externally
#define EC_LDE_SEC							bit4
#define EC_VSF_FIR							bit5	//voltage sensor failure
#define EC_VSF_SEC							bit6
#define EC_SOS_FF								bit7	//signal output states, F=off, N=on
#define EC_SOS_FFN							bit8
#define EC_SOS_FN								bit9
#define EC_SOS_NFF							bit10	
#define EC_SOS_NFN							bit11
#define EC_SOS_NN								bit12
#define EC_SOS_NNF							bit13
#define EC_SOS_NNF_FIR					bit14
#define EC_SOS_NNF_SEC					bit15
#define EC_VLB									bit16	//psm voltage value lower bound
#define EC_VUB									bit17	//psm voltage value upper bound
#define EC_VN										bit18	//psm voltage value normal value
#define EC_FLB									bit19	//psm frequency value lower bound
#define EC_FUB									bit20	//psm frequency value upper bound
#define EC_FN										bit21	//psm frequency value normal value
#define EC_RLB									bit22	//all red lamps broken
#define EC_YLB									bit23	//all yellow lamps broken
#define EC_GLB									bit24	//all green lamps broken

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Modules	(=Cards)																	//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*(1)
	name: cards in device
	expl: device may have psm, ssm, io. Also, there are loop detector cards which are third-party products
*/
#define MODULES_PSM_MAX						2
#define MODULES_SSM_MAX						8
#define MODULES_IO_MAX						2

/*
	name: devices
	expl: 
*/
#define SIG_DEV_SSM_FIRST					0
#define SIG_DEV_SSM_0						0
#define SIG_DEV_SSM_1						1
#define SIG_DEV_SSM_2						2
#define SIG_DEV_SSM_3						3
#define SIG_DEV_SSM_4						4
#define SIG_DEV_SSM_5						5
#define SIG_DEV_SSM_6						6
#define SIG_DEV_SSM_7						7
#define SIG_DEV_SSM_LAST					7
#define SIG_DEV_SSM_MAX						8

#define SIG_DEV_PSM_FIRST					16
#define SIG_DEV_PSM_0						16
#define SIG_DEV_PSM_1						17
#define SIG_DEV_PSM_LAST					17
#define SIG_DEV_PSM_MAX						2

#define SIG_DEV_MAX							32

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Program Data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _tSMPProgramData
{
	tSSignalDef SaSignalDefs[SIGNALS_MAX];
	tSSignalsDefined SSignalsDefined;
	tSSGDef SaSGDefs[SIGNAL_GROUPS_MAX];
	tSSODef SaSODefs[SIGNAL_OUTPUTS_MAX]; 
	tSCVSDef SCVSDef;
	tSConflictsEM SConflictsEM;
	uint16_t sFlashPerInEm;
	uint8_t bSGTotal;
	
	tSPeripheralStates SPeripheralStates;
	tSCurrentMeasurement SaCurrents[SIGNAL_OUTPUT_CURRENT_GROUPS_MAX];
	tSPowerSupply SaPSMs[PSMS_MAX];
	
	uint8_t bVoltageState;
	uint8_t bFrequencyState;

} tSMPProgramData, *tpSMPProgramData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Program Runtimes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _tSMPProgramRuntimes
{
	tSSetRuntime SaSetRuntime[SIGNAL_SETS_MAX];
	tSSGRuntime SaSGRuntimes[SIGNAL_GROUPS_MAX];
	tSSORuntime SaSORuntimes[SIGNAL_OUTPUTS_MAX];
	
} tSMPProgramRuntimes, *tpSMPProgramRuntimes;

typedef struct _tSNewMeasurements
{
	tSSORuntimeFlags	SaSORuntimeFlags[SIGNAL_OUTPUTS_MAX];
	tSCurrentMeasurement SaSGCurrents[SIGNAL_OUTPUT_CURRENT_GROUPS_MAX];
	
} tSNewMeasurements, *tpSNewMeasurements;

typedef struct _tSCPCom
{
	uint16_t sErrorCntr;
	
	 struct
	{
		uint8_t fState : 1;
		uint8_t fReserved : 7;
		
	} __attribute__((packed)) SFlags;
	
} tSCPCom, *tpSCPCom;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Battery Runtime																	//
extern void BatteryRuntimeSet(tpSADCBatteryRuntime pSRuntime);
extern void BatteryRuntimeGet(tpSADCBatteryRuntime pSRuntime);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												CP Comm																	//
extern uint8_t CPCommStateGet(void);
extern void CPCommStateSet(uint8_t fState);
extern void CPMPCommResetCntr(void);
extern void CPMPCommIncCntr(void);
extern uint8_t HeaterIsSetSafe(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//												Peripherals																	//
extern void PeripheralStatesSet(tpSPeripheralStates pSStates);
extern tpSPeripheralStates PeripheralStatesGet(void);
extern	void	GateStateSet(uint8_t fState);
extern	uint8_t		GateStateGet(void);
extern	void	RelayStateSet(uint8_t fState);
extern	uint8_t		RelayStateGet(void);
extern	void	HeaterStateSet(uint8_t fState);
extern	uint8_t		HeaterStateGet(void);
extern void	ProgramRestartStateSet(uint8_t fState);
extern uint8_t		ProgramRestartStateGet(void);
extern void	ClearSOPowersStateSet(uint8_t fState);
extern uint8_t		ClearSOPowersStateGet(void);
extern	void	LampDimmingStateSet(uint8_t fState);
extern	uint8_t		LampDimmingStateGet(void);
extern void 		LampDimmingStateChangedSet(uint8_t fState);
extern uint8_t 	LampDimmingStateChangedGet(void);
extern	void	ExternalBatteryStateSet(uint8_t fState);
extern	uint8_t		ExternalBatteryStateGet(void);

///////////////////////////////////////////////////////////
//										Extern Data

extern	void	RuntimesInit(void);

extern uint8_t	GetWorkingLampChangesAcceptCount(void);
extern void	SetWorkingLampChangeAcceptCount(uint8_t bTime);

//	Set Runtimes
extern	void	SetRuntimeInvalidSignalDetectedSet(uint8_t bSetNo, uint8_t fState);
extern	uint8_t	SetRuntimeInvalidSignalDetectedGet(uint8_t bSetNo);
extern	void	SetRuntimeInvalidSignalSequenceDetectedSet(uint8_t bSetNo, uint8_t fState);
extern	uint8_t	SetRuntimeInvalidSignalSequenceDetectedGet(uint8_t bSetNo);

//	SG Runtimes
extern	void	SGRuntimeLampFailRedSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailRedGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailYellowSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailYellowGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailGreenSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailGreenGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailRedAllSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailRedAllGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailYellowAllSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailYellowAllGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailGreenAllSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailGreenAllGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailLastRedSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailLastRedGet(uint8_t bSGNo);
extern	void	SGRuntimeLampFailNumberOfRedSet(uint8_t bSGNo, uint8_t fState);
extern	uint8_t	SGRuntimeLampFailNumberOfRedGet(uint8_t bSGNo);
extern 	void	SGRuntimeFailInvalidSignalSet(uint8_t bSGNo, uint8_t bVoltage, uint8_t fState);
extern 	uint8_t	SGRuntimeFailInvalidSignalGet(uint8_t bSGNo, uint8_t bVoltage);
extern	void	SGRuntimeFailInvalidSignalSequenceSet(uint8_t bSGNo, uint8_t bPrevVoltage, uint8_t fState);
extern	uint8_t	SGRuntimeFailInvalidSignalSequenceGet(uint8_t bSGNo, uint8_t bPrevVoltage);

//	SO Runtimes
extern	void	SORuntimeFailNoneSet(uint8_t bSONo);
extern	void	SORuntimeFailShortCircuitFirstSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeFailShortCircuitFirstGet(uint8_t bSONo);
extern	void	SORuntimeFailShortCircuitSecondSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeFailShortCircuitSecondGet(uint8_t bSONo);
extern	void	SORuntimeFailOpenCircuitSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeFailOpenCircuitGet(uint8_t bSONo);
extern	void	SORuntimeFailDrivenExternallyFirstSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeFailDrivenExternallyFirstGet(uint8_t bSONo);
extern	void	SORuntimeFailDrivenExternallySecondSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeFailDrivenExternallySecondGet(uint8_t bSONo);
extern	void	SORuntimeVoltageSensorFailureFirstSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeVoltageSensorFailureFirstGet(uint8_t bSONo);
extern	void	SORuntimeVoltageSensorFailureSecondSet(uint8_t bSONo, uint8_t fState);
extern	uint8_t	SORuntimeVoltageSensorFailureSecondGet(uint8_t bSONo);
extern	uint8_t		SORuntimePowerRecordSampleGet(uint8_t bSONo);

//	Maintenance
extern void EnableDebug(void);
extern void DisableDebug(void);
extern void DisableInterruptRequests(void);
extern void ClearResetFlags(void);
extern void ClearStandbyFlag(void);
extern void ClearWakeupFlag(void);
extern void ClearAllFlags(void);
extern void PrepareForStandbyMode(void);
extern void EnterStandbyModeWithPreparation(uint8_t fPrep);
extern	void	EnterStandbyMode(void);
extern void CheckWakeupOnReset(void);

// Conflicts

#define MIN_CONFLICT 40

#define	SO_SSM_GROUP_1						0x00	// states the signal outputs of SSM 1 - 4
#define	SO_SSM_GROUP_2						0x01	// states the signal outputs of SSM 5 - 8
///////////////////////////////////////////////////////////////////////////////////////////////////
//	Module Version

typedef	struct	_tSVersion
{
	uint8_t		bArg1;
	uint8_t		bArg2;
	uint8_t		bArg3;
	char	bArg4;
	
} __attribute__((packed)) tSVersion, *tpSVersion;

void	DataInit(void);

void	SetSSMVoltCurrentMeasurements(uint8_t bIdx, uint8_t* baData);
void	SSMVoltCurrentMeasurementsEvaluate(void);
void	SetPSMVoltageMeasurements(uint8_t bIdx, uint8_t* baData);
void	PSMVoltMeasurementsEvaluate(void);
void SORuntimeFlagsGet(tSSORuntimeFlags SaFlags[]);
void	SGCurrentsGet(tSCurrentMeasurement SaCurrs[]);
//signaling Mode
void	SetEmergencyMethodSet(uint8_t bEM, uint8_t bSetNo);
void	SetSignalingModeSet(uint8_t bSetNo, uint8_t bNewMode);
uint8_t	SetSignalingModeGet(uint8_t bSetNo);

//signals
uint8_t	VoltagesValid(uint8_t,						//sg owner
					  uint8_t);						//voltages
uint8_t	VoltagesCanFollowVoltages(uint8_t,				//signal
							  uint8_t);				//next signal
uint8_t	VoltagesHas(uint8_t,					//voltages
					uint8_t);					//look for this voltage
void	SignalGet(uint8_t,					//signal
				  tpSSignalDef);			//buffer to copy signal's definition to
void	SignalSet(uint8_t,					//signal
				  tpSSignalDef);			//buffer to copy signal's definition from
uint8_t	SignalValidGet(uint8_t);				//signal
uint8_t	SignalValidForFlashGet(uint8_t);		//signal
uint8_t	SignalValidForEmergencyFlashGet(uint8_t);	//signal
uint8_t	SignalDurationUnlimitedGet(uint8_t);	//signal
uint8_t	SignalMinDurationGet(uint8_t);		//signal
uint8_t	SignalMaxDurationGet(uint8_t);		//signal
uint8_t	SignalMatchVoltages(uint8_t,			//signal
							uint8_t);			//voltages
uint8_t	SignalHasFlash(uint8_t);				//signal
uint8_t	SignalHasGreen(uint8_t);				//signal
uint8_t	SignalHasYellow(uint8_t);				//signal
uint8_t	SignalHasRed(uint8_t);				//signal

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													Signals Defined																		//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void	SignalsDefinedSet(tpSSignalsDefined);	//defined signals
void	SignalsDefinedGet(tpSSignalsDefined);	//defined signals
uint8_t	SignalsDefinedBlockingGet(void);
uint8_t	SignalsDefinedFreeGet(void);
uint8_t	SignalsDefinedGreenFlashGet(void);
uint8_t	SignalsDefinedDarkGet(void);


// current,voltage,slope constants
void	GetCVSDef(tpSCVSDef);						//triple definition
void	SetCVSDef(tpSCVSDef);						//triple definition

//signal outputs - definition
extern	void		SetSODef(	uint8_t,											//so no, 0 based
													tpSSODef);									//buffer to copy definition from
extern	uint8_t		GetSOType(uint8_t);											//so no, 0 based
extern	uint8_t		GetSOOwner(uint8_t);										//so no, 0 based, returns sg no 1 based
extern	uint8_t		GetSONextOutput(uint8_t);								//so no, 0 based, returns so no 1 based
extern	uint8_t		GetSONoOfLamps(uint8_t);								//so no, 0 based
extern	uint16_t	GetSOPower(uint8_t);										//so no, 0 based
extern	void		SetSOPower(	uint8_t,										//so no, 0 based
														uint16_t);									//new power
extern	void		SetOnlySOPower(	uint8_t,								//so no, 0 based
																uint16_t);							//new power
extern	uint16_t	GetSOMinPower(uint8_t);									//so no, 0 based
extern	void		ClearSOPower(uint8_t);									//so no, 0 based
extern	uint8_t		GetSOEM(uint8_t);												//so no

extern	uint8_t		RecordingPower(uint8_t);								//so no, 0 based
extern	void		StartRecordingPower(uint8_t bSONo);
extern	void		StopRecordingPower(uint8_t bSONo);
extern	uint8_t		GetMinCurrentRecordSamples(uint8_t bSONo);
extern	uint8_t		IncreasePowerRecordSamples(uint8_t);		//so no, 0 based
extern	uint8_t		GetStartRecord(uint8_t);								//so no
extern	uint8_t		GetStartMean(uint8_t);									//so no
extern	void		SetStartRecord(	uint8_t,								//so no
																uint8_t);								//new state
extern	void		SetStartMean(	uint8_t,									//so no
															uint8_t);									//new state

//signal outputs - runtime
uint8_t	GetSONoOfWorkingLamps(uint8_t);				//so no, 0 based
void	SetSONoOfWorkingLamps(uint8_t,				//so no, 0 based
							  uint8_t);				//new lamp count
uint8_t	GetWorkingLampChangeCounter(uint8_t);			//so no, 0 based
uint8_t	IncreaseWorkingLampChangeCounter(uint8_t);	//so no, 0 based
void	ClearWorkingLampChangeCounter(uint8_t);		//so no, 0 based
uint8_t	GetNoOfLastDetectedWorkingLamps(uint8_t);		//so no, 0 based
void	SetNoOfLastDetectedWorkingLamps(uint8_t,		//so no, 0 based
										  uint8_t);	//new lamp count
uint8_t	GetSOSwitchState(uint8_t,						//so no
						 uint8_t);					//time slice
void	SOIdentification(uint8_t	bSSMGroupNo, tpSCanCpuSO pSCanCpu);
void	SetSOSwitchState(uint8_t,						//so no
						 uint8_t,						//time slice
						 uint8_t);						//switch state
uint8_t	GetSOVoltage(uint8_t);						//so no, 0 based
void	SetSOVoltage(uint8_t,							//so no, 0 based
					 uint8_t);							//voltage level
uint8_t	GetSOOnOffState(uint8_t);						//so no, 0 based
void	SetSOOnOffState(uint8_t,						//so no, 0 based
						uint8_t);						//on off state

//currents
uint16_t	GetCurrentMeasurement(uint8_t,				//current measurement group no
							  uint8_t);				//current measurement option (now, min, max)
void	SetCurrentMeasurement(uint8_t,				//current measurement group no
							  uint16_t);				//new measurement value
uint16_t	GetExpectedCurrent(uint8_t);

//events

uint8_t	IncrementEventCounter(uint8_t,				//bSONo
							  uint32_t);				//lIncrementEvents
void	InitEventCounters(uint8_t,					//bSONo
						  uint32_t);					//lInitEvents
uint8_t	SOStateDetermined(uint8_t);					//so no

//signal groups - definition
uint8_t	SetSGDefinition(uint8_t,						//sg no, 0 based
						tpSSGDef);					//sg definition buffer to copy data from
uint8_t	SetSGConflict(tpSSGConflict pSSGConflict);
uint8_t	ConflictingSG(uint8_t,						//sg no 1, 0 based
					  uint8_t);						//sg no 2, 0 based
uint8_t	GetSGClearance(uint8_t,						//sg no 1, 0 based
					   uint8_t);						//sg no 2, 0 based
uint8_t	GetSGFirstOutput(uint8_t);					//sg no, 0 based: returns 1 based

uint8_t	GetSGRedLampFailureNumber(uint8_t);			//sg no, 0 based
uint8_t	GetSGRedLampFailureNumberEM(uint8_t);			//sg no
uint8_t	GetSGLastRedLampFailureEM(uint8_t);			//sg no
uint8_t	GetSGNoOfLamps(uint8_t,						//sg no, 0 based
					   uint8_t);						//output type
uint8_t	GetSGTotal(void);
uint8_t	GetSGFlashSignal(uint8_t);					// sg no
uint8_t	GetSGOwner(uint8_t);							// sg no
uint8_t	IsValidSGOwnerSet(uint8_t);					// sg owner set

// conflict emergency methods
void	SetConflictsEMDef(tpSConflictsEM);			// conflicts emergency method def
void	SetFlashPeriodInEmergency(uint16_t sPeriod);
uint8_t	GetYYEM(void);
uint8_t	GetYGEM(void);
uint8_t	GetGGEM(void);
void	ApplyVoltageEM(uint8_t);						// emergency method
uint8_t	GetMalfunctionEM(void);
uint8_t	GetVoltageLimitsEM(void);
uint8_t	GetMalfunctionState(void);
uint8_t	GetVoltageLimitsState(void);
void	SetMalfunctionState(uint8_t);					// new state
void	SetVoltageLimitsState(uint8_t);				// new state
uint8_t	GetSignalErrorEM(void);
uint8_t	GetSignalErrorState(uint8_t);					// bSetNo
void	SetSignalErrorState(uint8_t, uint8_t);			// bSetNo, new state

//signal groups - runtime
uint8_t	GetSignal(uint8_t);							//sg no, 0 based
void	SetSignal(uint8_t,							//sg no, 0 based
				  uint8_t);							//signal
uint8_t	GetYYConflictState(uint8_t,					//sg no 1, 0 based
						   uint8_t);					//sg no 2, 0 based
void	SetYYConflictState(uint8_t,					//sg no 1, 0 based
						   uint8_t,					//sg no 2, 0 based
						   uint8_t);					//new conflict state
uint8_t	GetYGConflictState(uint8_t,					//sg no 1, 0 based
						   uint8_t);					//sg no 2, 0 based
void	SetYGConflictState(uint8_t,					//sg no 1, 0 based
						   uint8_t,					//sg no 2, 0 based
						   uint8_t);					//new conflict state
uint8_t	GetGGConflictState(uint8_t,					//sg no 1, 0 based
						   uint8_t);					//sg no 2, 0 based
void	SetGGConflictState(uint8_t,					//sg no 1, 0 based
						   uint8_t,					//sg no 2, 0 based
						   uint8_t);					//new conflict state
uint8_t	GetSGNoOfFailedLamps(uint8_t,					//sg no, 0 based
							 uint8_t);				//output type

//flash
uint16_t	FlashPerInEmGet(void);
void	FlashCntrInc(void);
void	FlashCntrClear(void);
uint8_t	FlashOnGet(uint16_t);							//subsignal period

//power
uint16_t	GetVoltage(void);
uint16_t	GetFrequency(void);
uint16_t	GetNetVoltage(uint8_t);						//psm no. returns actual voltage value calculated from measurement value
uint16_t	GetNetPrevVoltage(uint8_t);					//psm no
uint8_t	SetNetVoltage(uint8_t,						//psm no
					  uint16_t);						//new voltage (measurement value)
uint8_t	SetNetFrequency(uint8_t,						//psm no
					  uint16_t);						//new frequency (measurement value)
uint16_t	Get24V1(uint8_t);								//psm no
uint8_t	Set24V1(uint8_t,								//psm no
				uint16_t);							//new voltage
uint16_t	Get24V2(uint8_t);								//psm no
uint8_t	Set24V2(uint8_t,								//psm no
				uint16_t);							//new voltage
uint16_t	Get5V1(uint8_t);								//psm no
uint8_t	Set5V1(uint8_t,								//psm no
				uint16_t);							//new voltage
uint16_t	Get5V2(uint8_t);								//psm no
uint8_t	Set5V2(uint8_t,								//psm no
				uint16_t);							//new voltage
uint8_t	GetIsolatedVoltageState(uint8_t);				//psm no
uint8_t	SetIsolatedVoltageState(uint8_t,				//psm no
								uint8_t);				//new voltage

// voltage state
uint8_t	GetVoltageState(void);
void	SetVoltageState(uint8_t);						// new state
// frequency state
uint8_t	GetFrequencyState(void);
void	SetFrequencyState(uint8_t);					// new state

extern	uint8_t	SignalVoltagesGet(uint8_t bSignal);
extern	uint16_t	SubSignalHasFlash(uint8_t bSignal, uint8_t bOutputType);
extern	void	FlashPerInEmSet(uint16_t sNewPeriod);
extern	uint8_t	GetRestartProgramRequest(void);
extern	void	SetRestartProgramRequest(uint8_t	fNewState);
extern	void	GetPowerSupplies(tpSPowerSupply pSPowerSupply);
extern	uint8_t	IsSSMBelongsToSet(uint8_t bSetNo, uint8_t bSSMNo);
extern	uint8_t	GetInvalidSignalEM(void);
extern	uint8_t	GetInvalidSignalSequenceEM(void);

extern void	LogRequest(uint8_t bEvent, uint8_t bParam, uint16_t sParam, uint32_t lParam);

#endif
