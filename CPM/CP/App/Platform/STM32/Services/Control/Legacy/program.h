#ifndef _PROGRAM
#define _PROGRAM

#include "data.h"

/* /////////////////////////////////////////////////// */
/*  program states */
#define PROGRAM_STATE_DARK 0 /* all groups dark */
#define PROGRAM_STATE_LOADING 1 /* loading a new program into Maestro */
/* /////////////////////////////////////////////////// */
/*  program task runtime */
typedef struct _tSTaskProgramRuntime
{
  uint8_t bProgramState;
  uint8_t bCurrentState;

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
extern void StateCurrentInit(void);
extern uint8_t StateCurrentGet(void);
extern void StateCurrentSet(uint8_t);
extern uint8_t ProgramCurrentTransitionGet(void);
extern void ProgramSigProgChangeSet(uint8_t bState);

#endif /* ifndef _PROGRAM */
