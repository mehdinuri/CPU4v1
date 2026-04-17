#ifndef __SIGNALOUTPUTCATCH_H__
#define __SIGNALOUTPUTCATCH_H__

#include  "data.h"

/*
 *  extern methods
 */
extern uint8_t SOCatchIsCardActive(uint8_t bModuleNumber);
extern void SOCatchModuleActiveNowSet(uint8_t bModuleNumber, uint8_t fState);

#endif
