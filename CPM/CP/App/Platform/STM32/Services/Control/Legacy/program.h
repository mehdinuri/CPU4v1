#ifndef _PROGRAM
#define _PROGRAM

#include "data.h"

/* /////////////////////////////////////////////////// */
/*  program states */
#define PROGRAM_STATE_DARK 0 /* all groups dark */
#define PROGRAM_STATE_LOADING 1 /* loading a new program into Maestro */
#define PROGRAM_STATE_UPLOADING 2 /* uploading program from Maestro */

/* //////////////////////////////////////////////////// */
/*  program loading status */
#define PROGRAM_LOADING_NONE 0
#define PROGRAM_LOADING_JUST_STARTED 1
#define PROGRAM_LOADING_MAIN_TO_BACKUP 2
#define PROGRAM_LOADING_MAIN_TO_BACKUP_SUCCESS 3
#define PROGRAM_LOADING_MAIN_TO_BACKUP_ERROR 4
#define PROGRAM_LOADING_BACKUP_TO_MAIN 5
#define PROGRAM_LOADING_BACKUP_TO_MAIN_SUCCESS 6
#define PROGRAM_LOADING_BACKUP_TO_MAIN_ERROR 7
#define PROGRAM_LOADING_IN_PROGRESS 8
#define PROGRAM_LOADING_SUCCESS 9
#define PROGRAM_LOADING_ERROR 10
#define PROGRAM_LOADING_TO_MAIN 11
#define PROGRAM_LOADING_TO_BACKUP 12

/* /////////////////////////////////////////////////// */
/*  program uploading status */
#define PROGRAM_UPLOADING_JUST_STARTED 13
#define PROGRAM_UPLODING_IN_PROGRESS 14
#define PROGRAM_UPLOADING_SUCCESS 15
#define PROGRAM_UPLOADING_ERROR 16

#define PROGRAM_READ_ERR_FLASH 17
#define PROGRAM_READ_ERR_EEPROM 18

#define PROGRAM_LOADING_TOTAL 19
/* /////////////////////////////////////////////////// */
/*  program task runtime */
typedef struct _tSTaskProgramRuntime
{
  uint8_t bProgramState;
  uint8_t bCurrentState;
  uint8_t bProgramLoadingStatus;

  tSTransition STransitionCur; /* last transition applied */
  tSTransition STransitionSel; /* selected transition that will be applied */

  struct
  {
    uint8_t fProgramChanged : 1;
    uint8_t fReserved : 7;
  } __attribute__((packed)) SFlags;
} __attribute__((packed)) tSTaskProgramRuntime, *tpSTaskProgramRuntime;

/* /////////////////////////////////////////////////// */
/*  public functions */
extern void InitProgramTask(void);
extern uint8_t GetCurrentWorkModeFromProgram(void);
extern uint8_t GetCurrentWorkModeParamFromProgram(void);
extern uint8_t GetCurrentSignalPlan(void);
extern uint8_t SeqCurrentGet(void);
extern uint8_t ProgramStateGet(void);
extern void ProgramStateSet(uint8_t bState);
extern uint8_t ProgramCurrentNoGet(void);
extern uint8_t ProgramTargetNoGet(void);
extern void LoadProgramStarts(void);
extern void LoadProgramEnds(void);
extern void SetProgramLoadingStatus(uint8_t bNewStatus);
extern uint8_t ProgramLoadingStatusGet(void);
extern void StateCurrentInit(void);
extern uint8_t StateCurrentGet(void);
extern void StateCurrentSet(uint8_t);
extern uint8_t ProgramCurrentTransitionGet(void);
extern void ProgramSigProgChangeSet(uint8_t bState);

#endif /* ifndef _PROGRAM */
