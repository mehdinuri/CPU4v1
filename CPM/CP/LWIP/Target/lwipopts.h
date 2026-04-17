/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : Target/lwipopts.h
  * Description        : This file overrides LWIP stack default configuration
  *                      done in opt.h file.
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

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __LWIPOPTS__H__
#define __LWIPOPTS__H__

#include "main.h"

/*-----------------------------------------------------------------------------*/
/* Current version of LwIP supported by CubeMx: 2.2.1 -*/
/*-----------------------------------------------------------------------------*/

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */
#include "defs.h"
/* USER CODE END 0 */

#ifdef __cplusplus
 extern "C" {
#endif

/* STM32CubeMX Specific Parameters (not defined in opt.h) ---------------------*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
/*----- WITH_RTOS enabled (Since FREERTOS is set) -----*/
#define WITH_RTOS 1
/*----- WITH_MBEDTLS enabled (Since MBEDTLS and FREERTOS are set) -----*/
#define WITH_MBEDTLS 1
/*----- CHECKSUM_BY_HARDWARE disabled -----*/
#define CHECKSUM_BY_HARDWARE 0
/*-----------------------------------------------------------------------------*/

/* LwIP Stack Parameters (modified compared to initialization value in opt.h) -*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
/*----- Value in opt.h for LWIP_DHCP: 0 -----*/
#define LWIP_DHCP 1
/*----- Default value in ETH configuration GUI in CubeMx: 1524 -----*/
#define ETH_RX_BUFFER_SIZE 1536
/*----- Value in opt.h for LWIP_DNS: 0 -----*/
#define LWIP_DNS 1
/*----- Default Value for MEMP_NUM_UDP_PCB: 4 ---*/
#define MEMP_NUM_UDP_PCB 8
/*----- Default Value for MEMP_NUM_TCP_PCB: 5 ---*/
#define MEMP_NUM_TCP_PCB 8
/*----- Value in opt.h for MEM_ALIGNMENT: 1 -----*/
#define MEM_ALIGNMENT 4
/*----- Default Value for MEM_SIZE: 1600 ---*/
#define MEM_SIZE 32768
/*----- Default Value for H7 devices: 0x30004000 -----*/
#define LWIP_RAM_HEAP_POINTER 0x30008000
/*----- Default Value for MEMP_NUM_SYS_TIMEOUT: 8 ---*/
#define MEMP_NUM_SYS_TIMEOUT 16
/*----- Value supported for H7 devices: 1 -----*/
#define LWIP_SUPPORT_CUSTOM_PBUF 1
/*----- Default Value for PBUF_POOL_SIZE: 16 ---*/
#define PBUF_POOL_SIZE 32
/*----- Value in opt.h for LWIP_ETHERNET: LWIP_ARP || PPPOE_SUPPORT -*/
#define LWIP_ETHERNET 1
/*----- Value in opt.h for LWIP_DNS_SECURE: (LWIP_DNS_SECURE_RAND_XID | LWIP_DNS_SECURE_NO_MULTIPLE_OUTSTANDING | LWIP_DNS_SECURE_RAND_SRC_PORT) -*/
#define LWIP_DNS_SECURE 7
/*----- Value in opt.h for TCP_SND_QUEUELEN: (4*TCP_SND_BUF + (TCP_MSS - 1))/TCP_MSS -----*/
#define TCP_SND_QUEUELEN 9
/*----- Value in opt.h for TCP_SNDLOWAT: LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/2), (2 * TCP_MSS) + 1), (TCP_SND_BUF) - 1) -*/
#define TCP_SNDLOWAT 1071
/*----- Value in opt.h for TCP_SNDQUEUELOWAT: LWIP_MAX(TCP_SND_QUEUELEN)/2, 5) -*/
#define TCP_SNDQUEUELOWAT 5
/*----- Value in opt.h for TCP_WND_UPDATE_THRESHOLD: LWIP_MIN(TCP_WND/4, TCP_MSS*4) -----*/
#define TCP_WND_UPDATE_THRESHOLD 536
/*----- Default Value for LWIP_NETIF_STATUS_CALLBACK: 0 ---*/
#define LWIP_NETIF_STATUS_CALLBACK 1
/*----- Value in opt.h for LWIP_NETIF_LINK_CALLBACK: 0 -----*/
#define LWIP_NETIF_LINK_CALLBACK 1
/*----- Default Value for LWIP_HAVE_LOOPIF: 1 ---*/
#define LWIP_HAVE_LOOPIF 0
/*----- Default Value for LWIP_NETIF_LOOPBACK: 0 ---*/
#define LWIP_NETIF_LOOPBACK 1
/*----- Default Value for TCPIP_THREAD_NAME: "tcpip_thread" ---*/
#define TCPIP_THREAD_NAME "TCPIPTask"
/*----- Value in opt.h for TCPIP_THREAD_STACKSIZE: 0 -----*/
#define TCPIP_THREAD_STACKSIZE 4096
/*----- Value in opt.h for TCPIP_THREAD_PRIO: 1 -----*/
#define TCPIP_THREAD_PRIO 40
/*----- Value in opt.h for TCPIP_MBOX_SIZE: 0 -----*/
#define TCPIP_MBOX_SIZE 6
/*----- Default Value for SLIPIF_THREAD_NAME: "slipif_loop" ---*/
#define SLIPIF_THREAD_NAME "SLIPIFLoopTask"
/*----- Value in opt.h for SLIPIF_THREAD_STACKSIZE: 0 -----*/
#define SLIPIF_THREAD_STACKSIZE 1024
/*----- Value in opt.h for SLIPIF_THREAD_PRIO: 1 -----*/
#define SLIPIF_THREAD_PRIO 24
/*----- Default Value for DEFAULT_THREAD_NAME: "lwIP" ---*/
#define DEFAULT_THREAD_NAME "LwIPTask"
/*----- Value in opt.h for DEFAULT_THREAD_STACKSIZE: 0 -----*/
#define DEFAULT_THREAD_STACKSIZE 1024
/*----- Value in opt.h for DEFAULT_THREAD_PRIO: 1 -----*/
#define DEFAULT_THREAD_PRIO 24
/*----- Value in opt.h for DEFAULT_UDP_RECVMBOX_SIZE: 0 -----*/
#define DEFAULT_UDP_RECVMBOX_SIZE 6
/*----- Value in opt.h for DEFAULT_TCP_RECVMBOX_SIZE: 0 -----*/
#define DEFAULT_TCP_RECVMBOX_SIZE 6
/*----- Value in opt.h for DEFAULT_ACCEPTMBOX_SIZE: 0 -----*/
#define DEFAULT_ACCEPTMBOX_SIZE 6
/*----- Value in opt.h for RECV_BUFSIZE_DEFAULT: INT_MAX -----*/
#define RECV_BUFSIZE_DEFAULT 2000000000
/*----- Value in opt.h for LWIP_USE_EXTERNAL_MBEDTLS: 0 -----*/
#define LWIP_USE_EXTERNAL_MBEDTLS 1
/*----- Default Value for PPP_SUPPORT: 0 ---*/
#define PPP_SUPPORT 1
/*----- Default Value for PPP_INPROC_IRQ_SAFE: 0 ---*/
#define PPP_INPROC_IRQ_SAFE 1
/*----- Default Value for PPP_NOTIFY_PHASE: 0 ---*/
#define PPP_NOTIFY_PHASE 1
/*----- Default Value for PAP_SUPPORT: 0 ---*/
#define PAP_SUPPORT 1
/*----- Default Value for LWIP_SNMP: 0 ---*/
#define LWIP_SNMP 1
/*----- Default Value for SNMP_MAX_COMMUNITY_STR_LEN: 7 ---*/
#define SNMP_MAX_COMMUNITY_STR_LEN 32
/*----- Default Value for SNMP_LWIP_MIB2_SYSDESC: "lwIP" ---*/
#define SNMP_LWIP_MIB2_SYSDESC "Maestro CPU4 Intersection Controlling System"
/*----- Default Value for SNMP_LWIP_MIB2_SYSNAME: "FQDN-unk" ---*/
#define SNMP_LWIP_MIB2_SYSNAME "Maestro CPU4"
/*----- Default Value for SNMP_LWIP_MIB2_SYSCONTACT: "" ---*/
#define SNMP_LWIP_MIB2_SYSCONTACT "info@teknotel.com.tr"
/*----- Default Value for SNMP_LWIP_MIB2_SYSLOCATION: "" ---*/
#define SNMP_LWIP_MIB2_SYSLOCATION "Istanbul Türkiye"
/*----- Default Value for LWIP_SNMP_V3: 0 ---*/
#define LWIP_SNMP_V3 1
/*----- Default Value for LWIP_SNMP_CONFIGURE_VERSIONS: 0 ---*/
#define LWIP_SNMP_CONFIGURE_VERSIONS 1
/*----- Value in opt.h for MIB2_STATS: 0 or SNMP_LWIP_MIB2 -----*/
#define MIB2_STATS 1
/*----- Value in opt.h for CHECKSUM_GEN_ICMP6: 1 -----*/
#define CHECKSUM_GEN_ICMP6 0
/*----- Value in opt.h for CHECKSUM_CHECK_ICMP6: 1 -----*/
#define CHECKSUM_CHECK_ICMP6 0
/*-----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

#if defined DEBUG && defined LWIP_TRACE
#define LWIP_DEBUG 1
#define MEMP_OVERFLOW_CHECK 2
#define MEMP_SANITY_CHECK 1

#define LWIP_STATS_DISPLAY 1
#define SYS_STATS 1
#define MEM_STATS 1
#define MEMP_STATS 1

#define NETIF_DEBUG LWIP_DBG_OFF
#define IP_DEBUG LWIP_DBG_OFF
#define TCP_DEBUG LWIP_DBG_OFF
#define TCP_INPUT_DEBUG LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
#define TCPIP_DEBUG LWIP_DBG_OFF
#define UDP_DEBUG LWIP_DBG_OFF
#define MEM_DEBUG LWIP_DBG_ON
#define MEMP_DEBUG LWIP_DBG_ON
#define PBUF_DEBUG LWIP_DBG_ON
#define PPP_DEBUG LWIP_DBG_OFF
#define PPPOS_DEBUG LWIP_DBG_OFF
#endif

/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /*__LWIPOPTS__H__ */
