/* App/Adapters/STM32/EthernetModemAdapter.h
 *
 * IModemPort concrete adapter for Ethernet (LwIP / DHCP).
 * No serial port is used; GetBaudRate() returns 0.
 * OnInit() calls MX_LWIP_Init() to start the LwIP stack.
 */
#ifndef ETHERNET_MODEM_ADAPTER_H
#define ETHERNET_MODEM_ADAPTER_H

#include "Ports/IModemPort.h"

/* Context is empty — DHCP state is managed by LwIP internally. */
typedef struct
{
  ISerialPort_t *serialPort;
  uint8_t bPadding; /* reserved */
} EthernetModemAdapterCtx_t;

void EthernetModemAdapterInit(EthernetModemAdapterCtx_t *ctx);
IModemPort_t EthernetModemAdapterCreatePort(EthernetModemAdapterCtx_t *ctx);

#endif /* ETHERNET_MODEM_ADAPTER_H */
