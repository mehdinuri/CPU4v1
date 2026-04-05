/*
 * Platform/STM32/Tasks/NetworkTask.c
 *
 * FreeRTOS task that drives the LWIP network stack, obtains a DHCP lease,
 * and maintains a TCP connection to the MCS (Management Control System)
 * server.  Reconnects automatically on link loss.
 *
 * Priority : osPriorityNormal
 * Trigger  : event-driven (netif link state change + reconnect timer)
 * Argument : unused
 *
 * LWIP is initialised by CubeMX-generated MX_LWIP_Init() before the RTOS
 * scheduler starts.  This task calls ethernetif_input() to feed received
 * Ethernet frames into the LWIP input queue and handles DHCP state.
 */
#include "Tasks.h"

#ifdef STM32H743xx
#include "lwip/netif.h"
#include "lwip/dhcp.h"
#include "lwip/tcp.h"
#include "ethernetif.h"

extern struct netif gnetif;   /* CubeMX-generated global netif instance */
#endif

/* Reconnect interval in milliseconds. */
#define NETWORK_RECONNECT_INTERVAL_MS  5000U
#define NETWORK_DHCP_POLL_INTERVAL_MS   500U
#define NETWORK_ETH_POLL_INTERVAL_MS    100U

void NetworkTask(void *argument)
{
  (void) argument;

  #ifdef STM32H743xx
  /* Bring up the network interface. */
  netif_set_up(&gnetif);
  netif_set_default(&gnetif);

  /* Start DHCP client. */
  dhcp_start(&gnetif);
  #endif

  for (;;)
  {
    #ifdef STM32H743xx

    /* TODO: HAL impl — poll Ethernet Rx and feed LWIP.
     *
     * ethernetif_input(&gnetif);
     *
     * Monitor DHCP state:
     * if (dhcp_supplied_address(&gnetif)) {
     *     // DHCP lease obtained — attempt MCS TCP connect if not already up.
     * }
     *
     * TCP reconnect logic:
     * if (!mcs_tcp_connected) {
     *     mcs_connect(&gnetif, MCS_SERVER_IP, MCS_SERVER_PORT);
     * }
     */
    osDelay(NETWORK_ETH_POLL_INTERVAL_MS);
    #else
    osDelay(NETWORK_ETH_POLL_INTERVAL_MS);
    #endif
  }
}
