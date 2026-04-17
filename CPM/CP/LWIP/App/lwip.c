/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : lwip.c
  * Description        : This file provides initialization code for LWIP
  *                      middleWare.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"
#include <string.h>

/* USER CODE BEGIN 0 */
#include "MCS.h"
#include "eth.h"
#include "lwip/pbuf.h"
#include "netif/ppp/pppos.h"
#include "ppp.h"
#include "snmp_client.h"
#include "tcp_client.h"
#include "usart.h"
#include <string.h>
#include "MCSAsynch.h"
#include "udp_probe.h"

/* USER CODE END 0 */
/* Private function prototypes -----------------------------------------------*/
static void ethernet_link_status_updated(struct netif *netif);
/* ETH Variables initialization ----------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
/* USER CODE BEGIN OS_THREAD_ATTR_CMSIS_RTOS_V2 */
#define INTERFACE_THREAD_STACK_SIZE ( 1024 )
osThreadAttr_t attributes;
/* USER CODE END OS_THREAD_ATTR_CMSIS_RTOS_V2 */

/* USER CODE BEGIN 2 */
struct netif *LwIPGetInterface(void)
{
  return &gnetif;
}

uint8_t LwIPIsNetifUp(void)
{
  return netif_is_up(&gnetif) && netif_is_link_up(&gnetif);
}

void LwIPSetNetifDown(void)
{
  netif_set_down(&gnetif);
  netif_set_link_down(&gnetif);
}

void LwIPConfigInterface(void)
{
  ip4_addr_t ipaddr;
  ip4_addr_t netmask;
  ip4_addr_t gw;

  netif_set_down(&gnetif);

  HAL_ETH_Stop_IT(&heth);
  HAL_ETH_DeInit(&heth);

  EthFreeRxPool();
  EthFreeTxPbuf();

  heth.Init.MACAddr = &MCSGetEthernetMACAddress()->bAddress0;
  memcpy(&gnetif.hwaddr[0], &MCSGetEthernetMACAddress()->bAddress0,
         NETIF_MAX_HWADDR_LEN);

  HAL_ETH_Init(&heth);
  HAL_ETH_Start_IT(&heth);

  if (MCSIsEthernetStaticIP())
  {
    IP4_ADDR(&ipaddr,
             MCSGetLocalIPv4()->bAddress0,
             MCSGetLocalIPv4()->bAddress1,
             MCSGetLocalIPv4()->bAddress2,
             MCSGetLocalIPv4()->bAddress3);
    IP4_ADDR(&netmask,
             MCSGetSubnetMask()->bAddress0,
             MCSGetSubnetMask()->bAddress1,
             MCSGetSubnetMask()->bAddress2,
             MCSGetSubnetMask()->bAddress3);
    IP4_ADDR(&gw,
             MCSGetGateway()->bAddress0,
             MCSGetGateway()->bAddress1,
             MCSGetGateway()->bAddress2,
             MCSGetGateway()->bAddress3);
    netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
  }
  else
  {
    EthSetDHCPState(DHCP_LINK_DOWN);

    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
  }

  IP4_ADDR(&ipaddr,
           MCSGetPrimaryDNSServer()->bAddress0,
           MCSGetPrimaryDNSServer()->bAddress1,
           MCSGetPrimaryDNSServer()->bAddress2,
           MCSGetPrimaryDNSServer()->bAddress3);
  dns_setserver(0, &ipaddr);
  IP4_ADDR(&ipaddr,
           MCSGetSecondaryDNSServer()->bAddress0,
           MCSGetSecondaryDNSServer()->bAddress1,
           MCSGetSecondaryDNSServer()->bAddress2,
           MCSGetSecondaryDNSServer()->bAddress3);
  dns_setserver(1, &ipaddr);

  netif_set_up(&gnetif);

  if (!MCSIsEthernetStaticIP())
  {
    EthInitDHCP(&gnetif);
  }
} /* LwIPConfigInterface */

/* USER CODE END 2 */

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void)
{
  /* Initialize the LwIP stack with RTOS */
  tcpip_init( NULL, NULL );

  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
	
  if (MCSIsEthernetStaticIP())
  {
    IP4_ADDR(&ipaddr,
             MCSGetLocalIPv4()->bAddress0,
             MCSGetLocalIPv4()->bAddress1,
             MCSGetLocalIPv4()->bAddress2,
             MCSGetLocalIPv4()->bAddress3);
    IP4_ADDR(&netmask,
             MCSGetSubnetMask()->bAddress0,
             MCSGetSubnetMask()->bAddress1,
             MCSGetSubnetMask()->bAddress2,
             MCSGetSubnetMask()->bAddress3);
    IP4_ADDR(&gw,
             MCSGetGateway()->bAddress0,
             MCSGetGateway()->bAddress1,
             MCSGetGateway()->bAddress2,
             MCSGetGateway()->bAddress3);
  }

  /* add the network interface (IPv4/IPv6) with RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  /* We must always bring the network interface up connection or not... */
  netif_set_up(&gnetif);

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);

  /* Create the Ethernet link handler thread */
/* USER CODE BEGIN H7_OS_THREAD_NEW_CMSIS_RTOS_V2 */
  memset(&attributes, 0x0, sizeof(osThreadAttr_t));
  attributes.name = "EthLink";
  attributes.stack_size = INTERFACE_THREAD_STACK_SIZE;
  attributes.priority = osPriorityBelowNormal;
  osThreadNew(ethernet_link_thread, &gnetif, &attributes);

  IP4_ADDR(&ipaddr,
           MCSGetPrimaryDNSServer()->bAddress0,
           MCSGetPrimaryDNSServer()->bAddress1,
           MCSGetPrimaryDNSServer()->bAddress2,
           MCSGetPrimaryDNSServer()->bAddress3);
  dns_setserver(0, &ipaddr);
  IP4_ADDR(&ipaddr,
           MCSGetSecondaryDNSServer()->bAddress0,
           MCSGetSecondaryDNSServer()->bAddress1,
           MCSGetSecondaryDNSServer()->bAddress2,
           MCSGetSecondaryDNSServer()->bAddress3);
  dns_setserver(1, &ipaddr);

  if (!MCSIsEthernetStaticIP())
  {
    EthInitDHCP(&gnetif);
  }

  return;
	/* USER CODE END H7_OS_THREAD_NEW_CMSIS_RTOS_V2 */

  /* Start DHCP negotiation for a network interface (IPv4) */
  //dhcp_start(&gnetif);

	/* USER CODE BEGIN 3 */

	/* USER CODE END 3 */
}

#ifdef USE_OBSOLETE_USER_CODE_SECTION_4
/* Kept to help code migration. (See new 4_1, 4_2... sections) */
/* Avoid to use this user section which will become obsolete. */
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
#endif

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
static void ethernet_link_status_updated(struct netif *netif)
{
  if (netif_is_up(netif))
  {
/* USER CODE BEGIN 5 */
    if (!MCSIsEthernetStaticIP())
    {
      EthSetDHCPState(DHCP_START);
    }

/* USER CODE END 5 */
  }
  else /* netif is down */
  {
/* USER CODE BEGIN 6 */
    if (!MCSIsEthernetStaticIP())
    {
      EthSetDHCPState(DHCP_LINK_DOWN);
    }

    if (MCSAsynchConnectedGet())
    {
      MCSAsynchStop(MCS_ASYNCH_DISC_TYPE_ETH_LINK_DOWN);
    }

    if (SNMPClientIsStarted())
    {
      SNMPClientStop();
    }

    if (UDPProbeIsStarted())
    {
      UDPProbeStop();
    }

/* USER CODE END 6 */
  }
}

#if defined ( __CC_ARM )  /* MDK ARM Compiler */
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
  sio_fd_t sd;

/* USER CODE BEGIN 7 */
  sd = 0; // dummy code
/* USER CODE END 7 */

  return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
/* USER CODE BEGIN 8 */
/* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 9 */
  recved_bytes = 0; // dummy code
/* USER CODE END 9 */
  return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 10 */
  recved_bytes = 0; // dummy code
/* USER CODE END 10 */
  return recved_bytes;
}
#endif /* MDK ARM Compiler */

