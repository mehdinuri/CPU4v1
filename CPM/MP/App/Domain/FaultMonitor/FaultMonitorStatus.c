/* App/Domain/FaultMonitor/FaultMonitorStatus.c */

#include "FaultMonitor/FaultMonitorStatus.h"

#include <stddef.h>
#include <string.h>

void FaultMonitorStatusClear(FaultMonitorStatus_t *status)
{
  if (status == NULL)
  {
    return;
  }

  (void) memset(status, 0, sizeof(*status));
}
