/* App/Adapters/STM32/ModemAdapter.c
 *
 * Boot-time network type loader.
 *
 * Metadata restore runs before the scheduler starts.  Read the EEPROM
 * through the device adapter directly instead of routing through MSM;
 * MSMTask is not running at this point.
 */
#include "ModemAdapter.h"

#include <string.h>

#include "MCS.h"
#include "MSM.h"

uint8_t ModemAdapterLoadModuleType(IEepromStoragePort_t *eepromPort)
{
  tSMCSConInfo info;

  if (eepromPort == NULL)
  {
    return (uint8_t) MCS_NETWORK_TYPE_NONE;
  }

  memset(&info, 0, sizeof(info));

  if (EepromStorageRead(eepromPort,
                        EEPROM_STORAGE_ADDR_MCS_CON_INFO,
                        &info,
                        sizeof(info)) == FALSE)
  {
    return (uint8_t) MCS_NETWORK_TYPE_NONE;
  }

  if (info.bInitialized != MCS_CON_INFO_INITIALIZED)
  {
    return (uint8_t) MCS_NETWORK_TYPE_NONE;
  }

  if (info.bNetworkType >= (uint8_t) MCS_NETWORK_TYPE_MAX)
  {
    return (uint8_t) MCS_NETWORK_TYPE_NONE;
  }

  return info.bNetworkType;
}
