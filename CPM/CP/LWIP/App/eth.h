/**
 ******************************************************************************
 * @file           : ethernet.h
 * @brief          : Header for ethernet.c file.
 *                   This file contains the common defines for ethernet app.
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETHERNET_H__
#define __ETHERNET_H__

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "cmsis_os.h"
#include "MCS.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
/* Private define ------------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public define ------------------------------------------------------------*/
#define ETH_MAX_DHCP_TRIES  4
#define ETH_TIMEOUT_DNS_RESOLVE 30000

/* Public macros ------------------------------------------------------------*/

/* Public types -------------------------------------------------------------*/
typedef enum DHCP_STATES
{
  DHCP_OFF = 0,
  DHCP_START,
  DHCP_WAIT_ADDRESS,
  DHCP_ADDRESS_ASSIGNED,
  DHCP_TIMEOUT,
  DHCP_LINK_DOWN
} tEEthDHCPStates;

/* Public function prototypes -----------------------------------------------*/
extern void EthInitDHCP(struct netif *pSNetif);
extern void EthProcessDHCP(void);
extern void EthSetDHCPState(tEEthDHCPStates eState);
extern tEEthDHCPStates EthGetDHCPState(void);
extern uint8_t EthIsIpAssigned(void);
extern char *EthGetDHCPIPv4(void);
extern char *EthGetDHCPNetmask(void);
extern char *EthGetDHCPGateway(void);

#endif /* __ETHERNET_H__ */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
