#ifndef _UI
#define _UI

/* this header consists of macro and type definitions of UI task, */
/* which is responsible of managing a user interface, via LCD display, */
/* keypad and a serial port (PC connection) */

/* Serial Connection */
/* Maestro provides an interface, through which, ui data can be passed to and */
/* fro Communication is always initiated by the device that is connected to */
/* Maestro (ie. PC) Protocol: Communication is performed at 38400 bps using data */
/* packets that consist of ASCII characters. Each packet is made up of these */
/* fields: */
/* - Start of Packet */
/* - Packet Name */
/* - Data */
/* - Checksum */
/* - End of Packet */
/* Start and End of Packet fields are always a single character */
/* Other fields have alternating lengths so a separator character is */
/* inserted between them. If a field does not exist, there is no need */
/* for its separator. For example if there is no data field, there is */
/* no need for the packet name - data separator */
/* There are 3 diferrent separator characters, these separate */
/* packet name and data, data within data, data and checksum */
/* start of packet, end of packet and separator characters are */
/* never used in the other fields */

#include "Ports/ISerialPort.h"

#define UI_COMM_MAX_PACKET_LENGTH 255

#define UI_REQ_MAX 0x05

#define UI_REQ_TYPE_NONE 0x00
#define UI_REQ_TYPE_FIRST 0x01
#define UI_REQ_TYPE_SERIAL 0x01
#define UI_REQ_TYPE_USB 0x02
#define UI_REQ_TYPE_TCP_CLIENT 0x03
#define UI_REQ_TYPE_LAST 0x03

typedef struct _tSUIRequest
{
  uint8_t bReqId; /* request id */
  char strData[UI_COMM_MAX_PACKET_LENGTH + 1]; /* pointer to data buffer */
  uint16_t sDataSize; /* data size */
} tSUIRequest, *tpSUIRequest;

/* ////////////////////////////////////////////// */
/* Saving MCT Stream to Flash States */
#define UI_COMM_BACKUP_SCP_STATE_NONE 0
#define UI_COMM_BACKUP_SCP_STATE_SAVING 1
#define UI_COMM_BACKUP_SCP_STATE_ERROR 2
#define UI_COMM_BACKUP_SCP_STATE_COMPLETED 3
#define UI_COMM_BACKUP_SCP_STATE_READING 4

#define UI_COMM_BACKUP_SCP_MAX_SIZE 32768

/* ////////////////////////////////////////////// */
/* Start of Packet */
#define UI_COMM_START_OF_PACKET '-'
#define UI_COMM_START_OF_PACKET_STR "-"

/* Packet Name - Data separator */
#define UI_COMM_DATA_FIELD_SEPARATOR ':'
#define UI_COMM_DATA_FIELD_SEPARATOR_STR ":"

/* ////////////////////////////////////////////// */
/* Data - Data separator */
#define UI_COMM_DATA_SEPARATOR ','
#define UI_COMM_DATA_SEPARATOR_STR ","

/* ////////////////////////////////////////////// */
/* Packet separator */
#define UI_COMM_PACKET_SEPARATOR ';'
#define UI_COMM_PACKET_SEPARATOR_STR ";"

/* ////////////////////////////////////////////// */
/* Checksum separator */
#define UI_COMM_CHECKSUM_SEPARATOR ';'
#define UI_COMM_CHECKSUM_SEPARATOR_STR ";"

/* ////////////////////////////////////////////// */
/* IP separator */
#define UI_COMM_IP_SEPARATOR '.'
#define UI_COMM_IP_SEPARATOR_STR "."

/* ////////////////////////////////////////////// */
/* MAC separator */
#define UI_COMM_MAC_SEPARATOR ':'
#define UI_COMM_MAC_SEPARATOR_STR ":"

/* ////////////////////////////////////////////// */
/* Checksum */
/* The value in the checksum field is the mod 256 sum of */
/* the other fields, including start of packet, data field and */
/* data separators, excluding checksum separator and (obviously) end of packet. */

/* ////////////////////////////////////////////// */
/* End of Packet */
#define UI_COMM_END_OF_PACKET 0x0D
#define UI_COMM_END_OF_PACKET_STR "\r"

/* ////////////////////////////////////////////// */
/* misc definitons about packets */
#define UI_COMM_MCS_PACKET_PROTOCOL_LENGTH 8
#define UI_COMM_MAX_MCS_PACKET_LENGTH                                         \
        UI_COMM_MAX_PACKET_LENGTH - UI_COMM_MCS_PACKET_PROTOCOL_LENGTH

/* ////////////////////////////////////////////// */
/* packet parsing mechanism */
#define UI_COMM_PACKET_FIELD_NONE 0
#define UI_COMM_PACKET_FIELD_NAME 1
#define UI_COMM_PACKET_FIELD_DATA 2
#define UI_COMM_PACKET_FIELD_CHECKSUM 3
#define UI_COMM_PACKET_FIELD_EOP 4

#define UI_COMM_DATA_TYPE_STRING 0
#define UI_COMM_DATA_TYPE_INTEGER 1

#define UI_SAFETY_TIMEOUT 60 /* 60s */

/* ////////////////////////////////////////////// */
/* Packet Name - main packets */
#define UI_COMM_PACKET_NAME_INDEX 1

/* clear flash */
#define UI_COMM_PACKET_CLEAR_FLASH_STR "CFL"
#define UI_COMM_PACKET_CLEAR_FLASH                                             \
        ((uint32_t) 'C' | ((uint32_t) 'F' << 8) | ((uint32_t) 'L' << 16))

/* load default configuration */
#define UI_COMM_PACKET_LOAD_DEFAULT_STR "LDD"
#define UI_COMM_PACKET_LOAD_DEFAULT                                            \
        ((uint32_t) 'L' | ((uint32_t) 'D' << 8) | ((uint32_t) 'D' << 16))

/* start configuration */
#define UI_COMM_PACKET_START_CONF_STR "STC"
#define UI_COMM_PACKET_START_CONF                                              \
        ((uint32_t) 'S' | ((uint32_t) 'T' << 8) | ((uint32_t) 'C' << 16))

/* end configuration */
#define UI_COMM_PACKET_END_CONF_STR "ENC"
#define UI_COMM_PACKET_END_CONF                                                \
        ((uint32_t) 'E' | ((uint32_t) 'N' << 8) | ((uint32_t) 'C' << 16))

/* write configuration to flash */
#define UI_COMM_PACKET_FLASH_CONF_STR "FLS"
#define UI_COMM_PACKET_FLASH_CONF                                              \
        ((uint32_t) 'F' | ((uint32_t) 'L' << 8) | ((uint32_t) 'S' << 16))

/* load signal program to ram */
#define UI_COMM_PACKET_LOAD_SP_STR "LSP"
#define UI_COMM_PACKET_LOAD_SP                                                 \
        ((uint32_t) 'L' | ((uint32_t) 'S' << 8) | ((uint32_t) 'P' << 16))

/*  lamp dimming & heater configuration */
/* H&D Commented */

/*
 #define  UI_COMM_PACKET_SETTINGS_STR     "SET"
 #define  UI_COMM_PACKET_SETTINGS       ((uint32_t)'S' | ((uint32_t)'E' << 8) |
 *  ((uint32_t)'T' << 16))
 */

/* time */
/* data: "monthDay","month","year","hours","minutes","seconds" */
#define UI_COMM_PACKET_TIME_STR "TIM"
#define UI_COMM_PACKET_TIME                                                    \
        ((uint32_t) 'T' | ((uint32_t) 'I' << 8) | ((uint32_t) 'M' << 16))

/* Intersection info */
/* data: "countryName","cityName","intersectionName","timeZone","deviceType", */
/* "bCrossNo" */
#define UI_COMM_PACKET_DEVICE_INFO_STR "INT"
#define UI_COMM_PACKET_DEVICE_INFO                                             \
        ((uint32_t) 'I' | ((uint32_t) 'N' << 8) | ((uint32_t) 'T' << 16))

/* signal info */
/* data: "signal", "fValid", "fValidForFlash", "fValidForEmergencyFlash", */
/* "followerSignal0", .., .., "followerSignal3" */
#define UI_COMM_PACKET_SIGNAL_STR "SIN"
#define UI_COMM_PACKET_SIGNAL                                                  \
        ((uint32_t) 'S' | ((uint32_t) 'I' << 8) | ((uint32_t) 'N' << 16))

/* implement configuration file */
#define UI_COMM_PACKET_IMP_CONF_STR "SNC"
#define UI_COMM_PACKET_IMP_CONF                                                \
        ((uint32_t) 'S' | ((uint32_t) 'N' << 8) | ((uint32_t) 'C' << 16))

/* signals defined */
#define UI_COMM_PACKET_SIGNALS_DEFINED_STR "SND"
#define UI_COMM_PACKET_SIGNALS_DEFINED                                         \
        ((uint32_t) 'S' | ((uint32_t) 'N' << 8) | ((uint32_t) 'D' << 16))

/* signal group definition */
/* data: */
/* "sgNo","sgType","openingSignal","openingDuration","closingSignal","closingDuration","flashSignal","greenFlashDuration", */
/* "redLampFailureNumber", "redLampFailureNumberEM", "LastRedLampFailureEM" */
#define UI_COMM_PACKET_SG_STR "SGI"
#define UI_COMM_PACKET_SG                                                      \
        ((uint32_t) 'S' | ((uint32_t) 'G' << 8) | ((uint32_t) 'I' << 16))

/* signal output definition */
/* data: "ownerSGNo", "bSONo", "SSMNo","outputNo","type","bNoOfLamps", */
/* "bSOFailureEM" */
#define UI_COMM_PACKET_SO_STR "SOX"
#define UI_COMM_PACKET_SO                                                      \
        ((uint32_t) 'S' | ((uint32_t) 'O' << 8) | ((uint32_t) 'X' << 16))

/* clearance (conflict) */
/* request packet format: "sgNo","iTHconflict" */
/* data: "sgNo","conflictingSGNo","clearanceDuration" */
#define UI_COMM_PACKET_CLEARANCE_STR "CFX"
#define UI_COMM_PACKET_CLEARANCE                                               \
        ((uint32_t) 'C' | ((uint32_t) 'F' << 8) | ((uint32_t) 'X' << 16))

/* conflict emergency method */
/* data: "conflictName", "method" */
#define UI_COMM_PACKET_CONFLICT_EM_STR "CEM"
#define UI_COMM_PACKET_CONFLICT_EM                                             \
        ((uint32_t) 'C' | ((uint32_t) 'E' << 8) | ((uint32_t) 'M' << 16))

/* phase definition */
/* data: "phaseNo","minDuration","sgList" */
#define UI_COMM_PACKET_PHASE_STR "PHX"
#define UI_COMM_PACKET_PHASE                                                   \
        ((uint32_t) 'P' | ((uint32_t) 'H' << 8) | ((uint32_t) 'X' << 16))

/* transition definition */
/* data: "transitionNo","currentPhaseNo","targetPhaseNo","minGreen","maxGreen", */
/* "simCLosure","simOpening","demandForTargetBlockNo","stayInCurrentBlockNo" */
#define UI_COMM_PACKET_TRANSITION_STR "PTX"
#define UI_COMM_PACKET_TRANSITION                                              \
        ((uint32_t) 'P' | ((uint32_t) 'T' << 8) | ((uint32_t) 'X' << 16))

/* signal plan definition */
/* data: "signalPlanNo","signalprogram","workplan","workplan entry" */
#define UI_COMM_PACKET_SIGNAL_PLAN_STR "SPX"
#define UI_COMM_PACKET_SIGNAL_PLAN                                             \
        ((uint32_t) 'S' | ((uint32_t) 'P' << 8) | ((uint32_t) 'X' << 16))

/* work plan entry */
/* data: "workPlanNo","entryNo","hour,"minute","workMode", <"workModeParam"> */
#define UI_COMM_PACKET_WP_ENTRY_STR "ENX"
#define UI_COMM_PACKET_WP_ENTRY                                                \
        ((uint32_t) 'E' | ((uint32_t) 'N' << 8) | ((uint32_t) 'X' << 16))

/* work schedule entry */
#define UI_COMM_PACKET_WS_ENTRY_STR "WSE"
#define UI_COMM_PACKET_WS_ENTRY                                                \
        ((uint32_t) 'W' | ((uint32_t) 'S' << 8) | ((uint32_t) 'E' << 16))

/* flash periods */
/* data: "flashPeriod","emergencyFlashPeriod" */
#define UI_COMM_PACKET_FLASH_PERIODS_STR "FLP"
#define UI_COMM_PACKET_FLASH_PERIODS                                           \
        ((uint32_t) 'F' | ((uint32_t) 'L' << 8) | ((uint32_t) 'P' << 16))

/* input runtimes */
#define UI_COMM_PACKET_IO_RUNTIME_STR "INR"
#define UI_COMM_PACKET_IO_RUNTIME                                              \
        ((uint32_t) 'I' | ((uint32_t) 'N' << 8) | ((uint32_t) 'R' << 16))

/* ISSD function */
/* open relay */
#define UI_COMM_PACKET_OPEN_RELAY_STR "ORL"
#define UI_COMM_PACKET_OPEN_RELAY                                              \
        ((uint32_t) 'O' | ((uint32_t) 'R' << 8) | ((uint32_t) 'L' << 16))

/* close relay */
#define UI_COMM_PACKET_CLOSE_RELAY_STR "CRL"
#define UI_COMM_PACKET_CLOSE_RELAY                                             \
        ((uint32_t) 'C' | ((uint32_t) 'R' << 8) | ((uint32_t) 'L' << 16))

/* input data manipulation */
#define UI_COMM_PACKET_INP_DATA_MANIP_STR "IDM"
#define UI_COMM_PACKET_INP_DATA_MANIP                                          \
        ((uint32_t) 'I' | ((uint32_t) 'D' << 8) | ((uint32_t) 'M' << 16))

/* input 1 manipulation */
#define UI_COMM_PACKET_INPUT_MANIP1_STR "IM1"
#define UI_COMM_PACKET_INPUT_MANIP1                                            \
        ((uint32_t) 'I' | ((uint32_t) 'M' << 8) | ((uint32_t) '1' << 16))

/* loop dedector data 1 manipulation */
#define UI_COMM_PACKET_LD_MANIP1_STR "LD1"
#define UI_COMM_PACKET_LD_MANIP1                                               \
        ((uint32_t) 'L' | ((uint32_t) 'D' << 8) | ((uint32_t) '1' << 16))

/* loop dedector broken 1 manipulation */
#define UI_COMM_PACKET_LB_MANIP1_STR "LB1"
#define UI_COMM_PACKET_LB_MANIP1                                               \
        ((uint32_t) 'L' | ((uint32_t) 'B' << 8) | ((uint32_t) '1' << 16))

/* input 2 manipulation */
#define UI_COMM_PACKET_INPUT_MANIP2_STR "IM2"
#define UI_COMM_PACKET_INPUT_MANIP2                                            \
        ((uint32_t) 'I' | ((uint32_t) 'M' << 8) | ((uint32_t) '2' << 16))

/* loop dedector data 2 manipulation */
#define UI_COMM_PACKET_LD_MANIP2_STR "LD2"
#define UI_COMM_PACKET_LD_MANIP2                                               \
        ((uint32_t) 'L' | ((uint32_t) 'D' << 8) | ((uint32_t) '2' << 16))

/* loop dedector broken 2 manipulation */
#define UI_COMM_PACKET_LB_MANIP2_STR "LB2"
#define UI_COMM_PACKET_LB_MANIP2                                               \
        ((uint32_t) 'L' | ((uint32_t) 'B' << 8) | ((uint32_t) '2' << 16))

/* signal sequence step */
/* data: "sequenceNo", "stepNo", "stepDuration",signal1",..,signalN"  ; N is the */
/* total number of signal groups */
#define UI_COMM_PACKET_SEQUENCE_STEP_STR "SSX"
#define UI_COMM_PACKET_SEQUENCE_STEP                                           \
        ((uint32_t) 'S' | ((uint32_t) 'S' << 8) | ((uint32_t) 'X' << 16))

/* switch on sequence */
/* data: "sequenceNo" */
#define UI_COMM_PACKET_SWITCH_ON_SEQ_STR "SWN"
#define UI_COMM_PACKET_SWITCH_ON_SEQ                                           \
        ((uint32_t) 'S' | ((uint32_t) 'W' << 8) | ((uint32_t) 'N' << 16))

/* switch off sequence */
/* data: "sequenceNo" */
#define UI_COMM_PACKET_SWITCH_OFF_SEQ_STR "SWF"
#define UI_COMM_PACKET_SWITCH_OFF_SEQ                                          \
        ((uint32_t) 'S' | ((uint32_t) 'W' << 8) | ((uint32_t) 'F' << 16))

/* signals */
/* data: "SGsignal1","SGsignal2",..,"SGsignalN" */
#define UI_COMM_PACKET_SIGNALS_STR "SIG"
#define UI_COMM_PACKET_SIGNALS                                                 \
        ((uint32_t) 'S' | ((uint32_t) 'I' << 8) | ((uint32_t) 'G' << 16))

/* current runtime information */
/*  */
#define UI_COMM_PACKET_CURRENT_RUNTIME_INFO_STR "CRI"
#define UI_COMM_PACKET_CURRENT_RUNTIME_INFO                                    \
        ((uint32_t) 'C' | ((uint32_t) 'R' << 8) | ((uint32_t) 'I' << 16))

/* phase duration change */
/*  */
#define UI_COMM_PACKET_PHASE_DURATION_CHANGE_STR "PDC"
#define UI_COMM_PACKET_PHASE_DURATION_CHANGE                                   \
        ((uint32_t) 'P' | ((uint32_t) 'D' << 8) | ((uint32_t) 'C' << 16))

/* change workmode */
/*  */
#define UI_COMM_PACKET_CHANGE_WORKMODE_STR "CWM"
#define UI_COMM_PACKET_CHANGE_WORKMODE                                         \
        ((uint32_t) 'C' | ((uint32_t) 'W' << 8) | ((uint32_t) 'M' << 16))

/* log record */
/* data: "dd/mm/yy hh:mm:ss","eventStr","bParam","sParam","lPAram" */
#define UI_COMM_PACKET_LOG_NEXT_STR "LGN"
#define UI_COMM_PACKET_LOG_NEXT                                                \
        ((uint32_t) 'L' | ((uint32_t) 'G' << 8) | ((uint32_t) 'N' << 16))

#define UI_COMM_PACKET_LOG_FROM_STR "LGF"
#define UI_COMM_PACKET_LOG_FROM                                                \
        ((uint32_t) 'L' | ((uint32_t) 'G' << 8) | ((uint32_t) 'F' << 16))

#define UI_COMM_PACKET_LOG_DEL_STR "LGD"
#define UI_COMM_PACKET_LOG_DEL                                                 \
        ((uint32_t) 'L' | ((uint32_t) 'G' << 8) | ((uint32_t) 'D' << 16))

#define UI_COMM_PACKET_LOG_LAST_INDEX_STR "LLI"
#define UI_COMM_PACKET_LOG_LAST_INDEX                                          \
        ((uint32_t) 'L' | ((uint32_t) 'L' << 8) | ((uint32_t) 'I' << 16))

/* add user */
/* data: "username", "password", "user type" */
#define UI_COMM_PACKET_ADD_USER_STR "AUR"
#define UI_COMM_PACKET_ADD_USER                                                \
        ((uint32_t) 'A' | ((uint32_t) 'U' << 8) | ((uint32_t) 'R' << 16))

/* remove user */
/* data: "username" */
#define UI_COMM_PACKET_REMOVE_USER_STR "RUR"
#define UI_COMM_PACKET_REMOVE_USER                                             \
        ((uint32_t) 'R' | ((uint32_t) 'U' << 8) | ((uint32_t) 'R' << 16))

/* get usernames */
/* data: "username1", .., "usernameN" */
#define UI_COMM_PACKET_GET_USERNAMES_STR "GUR"
#define UI_COMM_PACKET_GET_USERNAMES                                           \
        ((uint32_t) 'G' | ((uint32_t) 'U' << 8) | ((uint32_t) 'R' << 16))

/* LCD language */
/* data: "language" */
#define UI_COMM_PACKET_LCD_LANGUAGE_STR "LCL"
#define UI_COMM_PACKET_LCD_LANGUAGE                                            \
        ((uint32_t) 'L' | ((uint32_t) 'C' << 8) | ((uint32_t) 'L' << 16))

/* Communication options */
/* data: "checksumEnableFlag" */
#define UI_COMM_PACKET_COMM_CONFIG_STR "CFG"
#define UI_COMM_PACKET_COMM_CONFIG                                             \
        ((uint32_t) 'C' | ((uint32_t) 'F' << 8) | ((uint32_t) 'G' << 16))

/* ////////////////////////////////////////////// */
/* Packet Name - total number packets */

/* number of signals */
/* data: "NumberOfSignals" */
#define UI_COMM_PACKET_SIN_NUMBER_STR "NSI"
#define UI_COMM_PACKET_SIN_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'I' << 16))

/* number of conflicts */
/* data: "NumberOfConflicts" */
#define UI_COMM_PACKET_CONFLICT_NUMBER_STR "NCF"
#define UI_COMM_PACKET_CONFLICT_NUMBER                                         \
        ((uint32_t) 'N' | ((uint32_t) 'C' << 8) | ((uint32_t) 'F' << 16))

/* data: "NumberOfSignalGroups" */
#define UI_COMM_PACKET_SG_NUMBER_STR "NSG"
#define UI_COMM_PACKET_SG_NUMBER                                               \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'G' << 16))

/* number of signal outputs of a signal group */
/* data: "NumberOfSignalOutputs" */
#define UI_COMM_PACKET_SO_NUMBER_STR "NSO"
#define UI_COMM_PACKET_SO_NUMBER                                               \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'O' << 16))

/* number of conflicts for a signal groups */
/* data: "NumberOfConflicts" */
#define UI_COMM_PACKET_CFL_NUMBER_STR "NCL"
#define UI_COMM_PACKET_CFL_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'C' << 8) | ((uint32_t) 'L' << 16))

/* number of phases */
/* data: "NumberOfPhases" */
#define UI_COMM_PACKET_PHA_NUMBER_STR "NPH"
#define UI_COMM_PACKET_PHA_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'P' << 8) | ((uint32_t) 'H' << 16))

/* number of fixed time tables */
/* data: "NumberOfFixedTimeTables" */

/*#define UI_COMM_PACKET_FTT_NUMBER_STR   "NFT"
 #define UI_COMM_PACKET_FTT_NUMBER      ((uint32_t)'N' | ((uint32_t)'F' << 8) |
 *  ((uint32_t)'T' << 16))*/

/* number of program time tables */
/* data: "NumberOfProgramTimeTables" */

/*#define UI_COMM_PACKET_PTT_NUMBER_STR   "NPR"
 #define UI_COMM_PACKET_PTT_NUMBER      ((uint32_t)'N' | ((uint32_t)'P' << 8) |
 *  ((uint32_t)'R' << 16))*/

/* number of operations */
/* data: "NumberOfOperations" */
#define UI_COMM_PACKET_OPT_NUMBER_STR "NOP"
#define UI_COMM_PACKET_OPT_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'O' << 8) | ((uint32_t) 'P' << 16))

/* number of transitions */
/* data: "NumberOfPhaseTransitions" */
#define UI_COMM_PACKET_TRA_NUMBER_STR "NPT"
#define UI_COMM_PACKET_TRA_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'P' << 8) | ((uint32_t) 'T' << 16))

/* number of signal plans */
/* data: "NumberOfSignalPlans" */
#define UI_COMM_PACKET_SP_NUMBER_STR "NSP"
#define UI_COMM_PACKET_SP_NUMBER                                               \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'P' << 16))

/* number of signal program plans (program time table) */
/* data: "NumberOfSignalProgramPlans" */
#define UI_COMM_PACKET_SPP_NUMBER_STR "NSL"
#define UI_COMM_PACKET_SPP_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'L' << 16))

/* number of signal programs */
/* data: "NumberOfSignalPrograms" */
#define UI_COMM_PACKET_SPR_NUMBER_STR "NSR"
#define UI_COMM_PACKET_SPR_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'R' << 16))

/* number of work plans */
/* data: "NumberOfDailyWorkPlans" */
#define UI_COMM_PACKET_WP_NUMBER_STR "NDW"
#define UI_COMM_PACKET_WP_NUMBER                                               \
        ((uint32_t) 'N' | ((uint32_t) 'D' << 8) | ((uint32_t) 'W' << 16))

/* number of entries for a work plan */
/* data: "NumberOfDailyWorkPlanEntries" */
#define UI_COMM_PACKET_WP_ENTRY_NUMBER_STR "NEN"
#define UI_COMM_PACKET_WP_ENTRY_NUMBER                                         \
        ((uint32_t) 'N' | ((uint32_t) 'E' << 8) | ((uint32_t) 'N' << 16))

/* number of work schedule */
/* data: "NumberOfWorkSchedules" */
#define UI_COMM_PACKET_WS_NUMBER_STR "NWS"
#define UI_COMM_PACKET_WS_NUMBER                                               \
        ((uint32_t) 'N' | ((uint32_t) 'W' << 8) | ((uint32_t) 'S' << 16))

/* number of sequences */
/* data: "NumberOfSignalSequences" */
#define UI_COMM_PACKET_SEQ_NUMBER_STR "NSQ"
#define UI_COMM_PACKET_SEQ_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'Q' << 16))

/* number of steps for a sequence */
/* data: "NumberOfSignalSteps" */
#define UI_COMM_PACKET_SEQ_STEP_NUMBER_STR "NSS"
#define UI_COMM_PACKET_SEQ_STEP_NUMBER                                         \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'S' << 16))

/* number of dedectors */
/* data: "number of dedectors" */
#define UI_COMM_PACKET_DET_NUMBER_STR "NDT"
#define UI_COMM_PACKET_DET_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'D' << 8) | ((uint32_t) 'T' << 16))

/* number of inputs */
/* data: "number of inputs" */
#define UI_COMM_PACKET_INPUT_NUMBER_STR "NIN"
#define UI_COMM_PACKET_INPUT_NUMBER                                            \
        ((uint32_t) 'N' | ((uint32_t) 'I' << 8) | ((uint32_t) 'N' << 16))

/* number of statements */
/* data: "number of statements" */
#define UI_COMM_PACKET_STATEMENT_NUMBER_STR "NST"
#define UI_COMM_PACKET_STATEMENT_NUMBER                                        \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'T' << 16))

/* number of phase logic blocks */
/* data: "number of phase logic blocks" */
#define UI_COMM_PACKET_PLB_NUMBER_STR "NPB"
#define UI_COMM_PACKET_PLB_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'P' << 8) | ((uint32_t) 'B' << 16))

/* number of sequence logic blocks */
/* data: "number of sequence logic blocks" */
#define UI_COMM_PACKET_SLB_NUMBER_STR "NSB"
#define UI_COMM_PACKET_SLB_NUMBER                                              \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'B' << 16))

/* number of outputs */
/* data: "number of outputs" */
#define UI_COMM_PACKET_OUTPUT_NUMBER_STR "NOU"
#define UI_COMM_PACKET_OUTPUT_NUMBER                                           \
        ((uint32_t) 'N' | ((uint32_t) 'O' << 8) | ((uint32_t) 'U' << 16))

/* number of sms users */
/* data: "number of sms users" */
#define UI_COMM_PACKET_SMS_USER_NUMBER_STR "NSM"
#define UI_COMM_PACKET_SMS_USER_NUMBER                                         \
        ((uint32_t) 'N' | ((uint32_t) 'S' << 8) | ((uint32_t) 'M' << 16))

/* synchronization packet for signal plan */
/* data: "signalPlanNo", "AZPPhase", "EZPPhase", "GSPPhase", "SYPhase", */
/* "EPhase0", .., "EPPhaseN" */
#define UI_COMM_PACKET_SP_SYNCH_STR "SSP"
#define UI_COMM_PACKET_SP_SYNCH                                                \
        ((uint32_t) 'S' | ((uint32_t) 'S' << 8) | ((uint32_t) 'P' << 16))

/* synchronization packet for sequences */
/* data: "seqNo", "AZPStep", "EZPStep", "GSPStep", "SYStep", "EPStep0", .., */
/* "EPStepN" */
#define UI_COMM_PACKET_SEQ_SYNCH_STR "SSQ"
#define UI_COMM_PACKET_SEQ_SYNCH                                               \
        ((uint32_t) 'S' | ((uint32_t) 'S' << 8) | ((uint32_t) 'Q' << 16))

/* current, voltages, slopes */
#define UI_COMM_PACKET_CVS_STR "CVS"
#define UI_COMM_PACKET_CVS                                                     \
        ((uint32_t) 'C' | ((uint32_t) 'V' << 8) | ((uint32_t) 'S' << 16))

/* logic blocks */
/* phase logic block */
/* data: "logic block number", "logic string" */
#define UI_COMM_PACKET_PLB_STR "PLB"
#define UI_COMM_PACKET_PLB                                                     \
        ((uint32_t) 'P' | ((uint32_t) 'L' << 8) | ((uint32_t) 'B' << 16))

/* sequence logic block */
/* data: "logic block number", "current sequence", "target sequence", "logic */
/* string" */
#define UI_COMM_PACKET_SLB_STR "SLB"
#define UI_COMM_PACKET_SLB                                                     \
        ((uint32_t) 'S' | ((uint32_t) 'L' << 8) | ((uint32_t) 'B' << 16))

/* input definitions */
/* data: "type", "input number", "owner sg", "green duration per demand", "red */
/* duration in broken", "phase in broken" */
#define UI_COMM_PACKET_INPUT_STR "INP"
#define UI_COMM_PACKET_INPUT                                                   \
        ((uint32_t) 'I' | ((uint32_t) 'N' << 8) | ((uint32_t) 'P' << 16))

/* output logic blocks */
/* data: "output number", "owner sg", "logic sentence" */
#define UI_COMM_PACKET_OUTPUT_STR "OUT"
#define UI_COMM_PACKET_OUTPUT                                                  \
        ((uint32_t) 'O' | ((uint32_t) 'U' << 8) | ((uint32_t) 'T' << 16))

/* ssm meauserement packet */
/* request data: "so voltages" */
/* response data: "so voltages", "min current", "now current", "max current" */
#define UI_COMM_PACKET_SSM_STR "SSM"
#define UI_COMM_PACKET_SSM                                                     \
        ((uint32_t) 'S' | ((uint32_t) 'S' << 8) | ((uint32_t) 'M' << 16))

#define UI_COMM_PACKET_SSM_LISTEN_STR "STL"
#define UI_COMM_PACKET_SSM_LISTEN                                              \
        ((uint32_t) 'S' | ((uint32_t) 'T' << 8) | ((uint32_t) 'L' << 16))

#define UI_COMM_PACKET_SSM_TEST_END_STR "STE"
#define UI_COMM_PACKET_SSM_TEST_END                                            \
        ((uint32_t) 'S' | ((uint32_t) 'T' << 8) | ((uint32_t) 'E' << 16))

/* psm meauserement packet */
/* data: */
#define UI_COMM_PACKET_PSM_STR "PSM"
#define UI_COMM_PACKET_PSM                                                     \
        ((uint32_t) 'P' | ((uint32_t) 'S' << 8) | ((uint32_t) 'M' << 16))

/* io meauserement packet */
/* data: */
#define UI_COMM_PACKET_IOM_STR "IOM"
#define UI_COMM_PACKET_IOM                                                     \
        ((uint32_t) 'I' | ((uint32_t) 'O' << 8) | ((uint32_t) 'M' << 16))

/* signal program plan */
#define UI_COMM_PACKET_SP_PLAN_ENTRY_STR "SPP"
#define UI_COMM_PACKET_SP_PLAN_ENTRY                                           \
        ((uint32_t) 'S' | ((uint32_t) 'P' << 8) | ((uint32_t) 'P' << 16))

/* operand variables */
#define UI_COMM_PACKET_OPERANDS_STR "OPV"
#define UI_COMM_PACKET_OPERANDS                                                \
        ((uint32_t) 'O' | ((uint32_t) 'P' << 8) | ((uint32_t) 'V' << 16))

/* operations */
#define UI_COMM_PACKET_OPERATIONS_STR "OPT"
#define UI_COMM_PACKET_OPERATIONS                                              \
        ((uint32_t) 'O' | ((uint32_t) 'P' << 8) | ((uint32_t) 'T' << 16))

/* rules */
#define UI_COMM_PACKET_RULES_STR "RUL"
#define UI_COMM_PACKET_RULES                                                   \
        ((uint32_t) 'R' | ((uint32_t) 'U' << 8) | ((uint32_t) 'L' << 16))

/* statements */
#define UI_COMM_PACKET_STATEMENTS_STR "STM"
#define UI_COMM_PACKET_STATEMENTS                                              \
        ((uint32_t) 'S' | ((uint32_t) 'T' << 8) | ((uint32_t) 'M' << 16))

/* this packet is used as a signal from MCT to device. It means that signal */
/* program contents are completely sent to device after this packet is received */
/* by device, device writes signal program contents (operands, operations, */
/* rules, statements) to storage and clear these contents for the next signal */
/* program that will be sent from MCT */
#define UI_COMM_PACKET_SIGNAL_PROGRAM_STR "SPR"
#define UI_COMM_PACKET_SIGNAL_PROGRAM                                          \
        ((uint32_t) 'S' | ((uint32_t) 'P' << 8) | ((uint32_t) 'R' << 16))

/* line operator */
#define UI_COMM_PACKET_LINE_OPERATOR_STR "LOP"
#define UI_COMM_PACKET_LINE_OPERATOR                                           \
        ((uint32_t) 'L' | ((uint32_t) 'O' << 8) | ((uint32_t) 'P' << 16))

/* pop3 */
#define UI_COMM_PACKET_POP3_STR "POP"
#define UI_COMM_PACKET_POP3                                                    \
        ((uint32_t) 'P' | ((uint32_t) 'O' << 8) | ((uint32_t) 'P' << 16))

/* smtp */
#define UI_COMM_PACKET_SMTP_STR "SMP"
#define UI_COMM_PACKET_SMTP                                                    \
        ((uint32_t) 'S' | ((uint32_t) 'M' << 8) | ((uint32_t) 'P' << 16))

/* sms users */
#define UI_COMM_PACKET_SMS_USER_STR "SMU"
#define UI_COMM_PACKET_SMS_USER                                                \
        ((uint32_t) 'S' | ((uint32_t) 'M' << 8) | ((uint32_t) 'U' << 16))

/* remote connection type */
/* data: "remoteconnectiontype" */
#define UI_COMM_PACKET_REMOTE_CONN_STR "RCT"
#define UI_COMM_PACKET_REMOTE_CONN                                             \
        ((uint32_t) 'R' | ((uint32_t) 'C' << 8) | ((uint32_t) 'T' << 16))

/* --> debug packets */
#define UI_COMM_PACKET_CLR_SO_POWERS_STR "CSP"
#define UI_COMM_PACKET_CLR_SO_POWERS                                           \
        ((uint32_t) 'C' | ((uint32_t) 'S' << 8) | ((uint32_t) 'P' << 16))
/* <-- debug packets */

/* request packet */
/* data: "packetName","parameter" */
#define UI_COMM_PACKET_REQUEST_STR "REQ"
#define UI_COMM_PACKET_REQUEST                                                 \
        ((uint32_t) 'R' | ((uint32_t) 'E' << 8) | ((uint32_t) 'Q' << 16))
#define UI_COMM_PACKET_REQ_NAME_INDEX 5

/* checksum total */
/* data: consumed data checksum total */
#define UI_COMM_PACKET_CHECKSUM_TOTAL_STR "CST"
#define UI_COMM_PACKET_CHECKSUM_TOTAL                                          \
        ((uint32_t) 'C' | ((uint32_t) 'S' << 8) | ((uint32_t) 'T' << 16))

#define UI_COMM_PACKET_MODULE_VERSIONS_STR "VER"
#define UI_COMM_PACKET_MODULE_VERSIONS                                         \
        ((uint32_t) 'V' | ((uint32_t) 'E' << 8) | ((uint32_t) 'R' << 16))

#define UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS_STR "MMV"
#define UI_COMM_PACKET_MAESTRO_MODULE_VERSIONS                                 \
        ((uint32_t) 'M' | ((uint32_t) 'M' << 8) | ((uint32_t) 'V' << 16))

/* request function configuration */
#define UI_COMM_PACKET_FUNCTION_CONF_STR "FCF"
#define UI_COMM_PACKET_FUNCTION_CONF                                           \
        ((uint32_t) 'F' | ((uint32_t) 'C' << 8) | ((uint32_t) 'F' << 16))

/* MCST Upload */
#define UI_COMM_PACKET_MCS_UPLOAD_STR "MUP"
#define UI_COMM_PACKET_MCS_UPLOAD                                             \
        ((uint32_t) 'M' | ((uint32_t) 'U' << 8) | ((uint32_t) 'P' << 16))

/* System Start Time */
#define UI_COMM_PACKET_SYSTEM_START_TIME_STR "SST"
#define UI_COMM_PACKET_SYSTEM_START_TIME                                       \
        ((uint32_t) 'S' | ((uint32_t) 'S' << 8) | ((uint32_t) 'T' << 16))

/* Test */
#define UI_COMM_PACKET_DEBUG_STR "DBG"
#define UI_COMM_PACKET_DEBUG                                                   \
        ((uint32_t) 'D' | ((uint32_t) 'B' << 8) | ((uint32_t) 'G' << 16))

/* Restart */
#define UI_COMM_PACKET_RESET_CPU_STR "RST"
#define UI_COMM_PACKET_RESET_CPU                                               \
        ((uint32_t) 'R' | ((uint32_t) 'S' << 8) | ((uint32_t) 'T' << 16))

/* IAP */
#define UI_COMM_PACKET_IAP_STR "IAP"
#define UI_COMM_PACKET_IAP                                                     \
        ((uint32_t) 'I' | ((uint32_t) 'A' << 8) | ((uint32_t) 'P' << 16))

/* MCS Connection */
#define UI_COMM_PACKET_MCS_CON_INFO_STR "MCI"
#define UI_COMM_PACKET_MCS_CON_INFO                                            \
        ((uint32_t) 'M' | ((uint32_t) 'C' << 8) | ((uint32_t) 'I' << 16))

#define UI_COMM_IAP_DATA_LEN_INDEX 5 /* -IAP: */
#define UI_COMM_IAP_PACKET_HEAD 5
#define UI_COMM_IAP_PACKET_TAIL 1
#define UI_COMM_IAP_PACKET_OVERHEAD                                            \
        (UI_COMM_IAP_PACKET_HEAD + UI_COMM_IAP_PACKET_TAIL)

/* response packets */
#define UI_COMM_PACKET_SUCCESS_STR "SUC"
#define UI_COMM_PACKET_CHECKSUM_ERROR_STR "CHE"
#define UI_COMM_PACKET_FRAME_ERROR_STR "FRE"
#define UI_COMM_PACKET_OPERATION_ERROR_STR "FAI"
#define UI_COMM_PACKET_BUF_SIZE_ERROR_STR "BFE"
#define UI_COMM_PACKET_SECURITY_ERROR_STR "SEC"
#define UI_COMM_PACKET_IN_SECURE_TRANSITION_STR "TRA"
#define UI_COMM_PACKET_FLASH_ERROR_STR "FWE"

#define UI_COMM_RESPONSE_SUCCESS 0
#define UI_COMM_RESPONSE_CHECKSUM_ERROR 1
#define UI_COMM_RESPONSE_FRAME_ERROR 2
#define UI_COMM_RESPONSE_OPERATION_ERROR 3
#define UI_COMM_RESPONSE_BUF_SIZE_ERROR 4
#define UI_COMM_RESPONSE_SECURITY_ERROR 5
#define UI_COMM_RESPONSE_IN_SECURE_TRANSITION 6
#define UI_COMM_RESPONSE_PACKET 7
#define UI_COMM_RESPONSE_FLASH_ERROR 8

/* string data */
#define UI_COMM_STR_MAINWAY "SGMV"
#define UI_COMM_STR_SUBWAY "SGSV"
#define UI_COMM_STR_FLASHER "SGF"
#define UI_COMM_STR_PEDESTRIAN "SGP"
#define UI_COMM_STR_TRAM "SGT"
#define UI_COMM_STR_BICYCLE "SGB"

/* signals */
#define UI_COMM_STR_DARK "D"
#define UI_COMM_STR_RED "R"
#define UI_COMM_STR_YELLOW "Y"
#define UI_COMM_STR_GREEN "G"

/* conflicting signals */
#define UI_COMM_STR_GREENGREENCONFLICTEM "GG"
#define UI_COMM_STR_YELLOWGREENCONFLICTEM "YG"
#define UI_COMM_STR_YELLOWYELLOWCONFLICTEM "YY"
#define UI_COMM_STR_INVALIDSIGNALEM "IS"
#define UI_COMM_STR_INVALIDSIGNALSEQUENCEEM "ISS"
#define UI_COMM_STR_VOLTAGELIMITSEM "VL"
#define UI_COMM_STR_FREQUENCYERROREM "FE"

/* flash signals */
#define UI_COMM_STR_FLASH "F"
#define UI_COMM_STR_UNDEFINED "?"

/* uint8_t values */
#define UI_COMM_STR_TRUE "TRUE"
#define UI_COMM_STR_FALSE "FALSE"

/* week days */
#define UI_COMM_STR_MONDAY "Monday"
#define UI_COMM_STR_TUESDAY "Tuesday"
#define UI_COMM_STR_WEDNESDAY "Wednesday"
#define UI_COMM_STR_THURSDAY "Thursday"
#define UI_COMM_STR_FRIDAY "Friday"
#define UI_COMM_STR_SATURDAY "Saturday"
#define UI_COMM_STR_SUNDAY "Sunday"

/* languages */
#define UI_COMM_STR_LANGUAGE_TURKISH "TR"
#define UI_COMM_STR_LANGUAGE_ENGLISH "EN"

/* device types */
#define UI_COMM_STR_DEVICE_TYPE_MAESTRO "MAESTRO"

#define UI_COMM_STR_NONE "N" /* None */

/* arithmetic operations */
#define UI_COMM_MUL_STR "*"
#define UI_COMM_DIV_STR "/"
#define UI_COMM_ADD_STR "+"
#define UI_COMM_SUB_STR "-"
#define UI_COMM_LTH_STR "<"
#define UI_COMM_GTH_STR ">"
#define UI_COMM_EQU_STR "="
#define UI_COMM_AND_STR "&"
#define UI_COMM_OR_STR "|"

/* ENC packet */
#define SUC_CODE_MAIN_MEMORY_WRITE bit0

/* public data */
typedef enum
{
  Time = 0x02,
  Info,
  NumberofSignal,
  Signal,
  SignalAssignments,
  NumberofSignalGroups,
  SignalGroup,
  SignalOutput,
  CVS, /* 10 */
  NumberofConflicts,
  Conflict,
  FailureAction,
  NumberofSequence,
  NumberofSequenceStep,
  SignalSequenceStep,
  NumberofPhase,
  Phase,
  NumberofInputs,
  NumberofDetectors, /* 20 */
  Input,
  NumberofOutputs,
  Output,
  NumberofSignalPrograms,
  NumberofProgramRules,
  Expression,
  NumberofSignalTasks,
  Statement,
  Rule,
  NumberofTransitions, /* 30 */
  Transition,
  ProgramEnd,
  NumberofFixedTimeTableRows,
  FixedTimeTableRow,
  NumberofProgramTableRows,
  ProgramTableRow,
  NumberofSignalPlans,
  SignalPlan,
  NumberofScheduleRows,
  ScheduleRow /* 40 */
} tEPacketName;

/*  public functions */
extern void UIInit(ISerialPort_t *port);
extern void UIRuntimeTimeoutsChecks(void);
extern void UILogReadIndexSet(uint16_t sId);
extern void UIRxRequest(uint8_t bReqId, char *pstrData, uint16_t sDataSize);
extern void UITxRequest(uint8_t bReqId, char *pstrData, uint16_t sDataSize);
extern uint8_t UIMCSDownloadInProgressGet(void);
extern uint8_t UIMCSUploadInProgressGet(void);

#endif /* ifndef _UI */
