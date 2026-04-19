#ifndef __MMI_H__
#define __MMI_H__

#include "MLM.h"

void OpenMMI(void);
void CloseMMI(void);
void StreamPSMMeasurements(void);
void StreamDateTime(void);
void StreamGPRSImei(void);
void StreamUSRMAC(void);
void StreamEthernetMAC(void);
void StreamGsmOperator(void);
void StreamGPRSState(void);
void StreamOperationRuntime(void);
void StreamModuleRuntime(void);
void StreamGateStateChanged(uint8_t fState, tpSLogRecord pSLog);
void StreamErrorRuntime(void);
void StreamSignals(void);
void StreamInputs(void);
void StreamSOTest(void);

#endif /* __MMI_H__ */
