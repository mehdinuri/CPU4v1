/**
 ******************************************************************************
 * File Name          : eeprom.c
 * Description        : Code for ethernet app
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "gpio.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip.h"
#include  "eth.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
tEEthDHCPStates EDHCPState = DHCP_OFF;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Public typedef -----------------------------------------------------------*/

/* Public define ------------------------------------------------------------*/

/* Public variables ---------------------------------------------------------*/

/* Public function prototypes -----------------------------------------------*/

/* Private application code --------------------------------------------------*/

/* Public application code --------------------------------------------------*/
void EthInitDHCP(struct netif *pSNetif)
{
  if (netif_is_up(pSNetif))
  {
    EthSetDHCPState(DHCP_START);
  }
  else
  {
    EthSetDHCPState(DHCP_LINK_DOWN);
  }
}

void EthProcessDHCP(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;

  struct netif *pSNetif = (struct netif *) LwIPGetInterface();

  switch (EDHCPState)
  {
      case DHCP_START:
      {
        ip_addr_set_zero_ip4(&pSNetif->ip_addr);
        ip_addr_set_zero_ip4(&pSNetif->netmask);
        ip_addr_set_zero_ip4(&pSNetif->gw);
        EthSetDHCPState(DHCP_WAIT_ADDRESS);
        dhcp_start(pSNetif);
        break;
      }

      case DHCP_WAIT_ADDRESS:
      {
        if (dhcp_supplied_address(pSNetif))
        {
          EthSetDHCPState(DHCP_ADDRESS_ASSIGNED);
        }
        else
        {
          dhcp = (struct dhcp *) netif_get_client_data(pSNetif,
                                                       LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

          if (dhcp->tries > ETH_MAX_DHCP_TRIES)
          {
            EthSetDHCPState(DHCP_TIMEOUT);
            dhcp_stop(pSNetif);

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
            netif_set_addr(pSNetif, &ipaddr, &netmask, &gw);
          }
        }

        break;
      }

      case DHCP_LINK_DOWN:
      {
        dhcp_stop(pSNetif);
        EthSetDHCPState(DHCP_OFF);
        break;
      }

      case DHCP_TIMEOUT:
      {
        dhcp_stop(pSNetif);
        EthSetDHCPState(DHCP_START);
        break;
      }

      default:
      {
        break;
      }
  } /* switch */
} /* EthProcessDHCP */

void EthSetDHCPState(tEEthDHCPStates eState)
{
  EDHCPState = eState;
}

tEEthDHCPStates EthGetDHCPState(void)
{
  return EDHCPState;
}

uint8_t EthIsIpAssigned(void)
{
  return EDHCPState == DHCP_ADDRESS_ASSIGNED;
}

char *EthGetDHCPIPv4(void)
{
  struct netif *pSNetif = (struct netif *) LwIPGetInterface();

  if (pSNetif != NULL)
  {
    return ipaddr_ntoa(netif_ip_addr4(pSNetif));
  }

  return "";
}

char *EthGetDHCPNetmask(void)
{
  struct netif *pSNetif = (struct netif *) LwIPGetInterface();

  if (pSNetif != NULL)
  {
    return ipaddr_ntoa(netif_ip_netmask4(pSNetif));
  }

  return "";
}

char *EthGetDHCPGateway(void)
{
  struct netif *pSNetif = (struct netif *) LwIPGetInterface();

  if (pSNetif != NULL)
  {
    return ipaddr_ntoa(netif_ip_gw4(pSNetif));
  }

  return "";
}

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK *****END OF FILE****/
