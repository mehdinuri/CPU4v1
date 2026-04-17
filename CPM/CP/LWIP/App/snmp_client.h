/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */

#ifndef __SNMP_CLIENT_H__
#define __SNMP_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32h7xx_hal.h"
#include "snmp_core.h"

typedef enum SNMP_CLIENT_STATE
{
  SNMP_CLIENT_NONE = 0,
  SNMP_CLIENT_STARTED
} tESNMPClientState;

extern uint8_t SNMPClientStart(ip_addr_t *pSTargetIp);
extern void SNMPClientStop(void);
extern tESNMPClientState SNMPClientGetState(void);
extern uint8_t SNMPClientIsStarted(void);
extern void SNMPClientSendPowerOffTrap(void);
extern void SNMPSendColdStartTrap(void);
extern uint8_t SNMPSendPowerDownTrap(void);
extern void SNMPSendSSMStatusTrap(uint16_t sSSMStatus);
extern void SNMPSendDoorOpenTrap(void);

#ifdef __cplusplus
}
#endif

#endif /* __SNMP_CLIENT_H__ */
