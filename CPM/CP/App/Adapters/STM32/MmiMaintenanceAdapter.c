/* App/Adapters/STM32/MmiMaintenanceAdapter.c */
#include "MmiMaintenanceAdapter.h"

#include <string.h>

#include "MLM.h"
#include "data.h"
#include "lcd.h"
#include "program.h"

enum
{
  MMI_MAINTENANCE_REQUEST_ALL_RED = 1U,
  MMI_MAINTENANCE_REQUEST_DARK = 2U,
  MMI_MAINTENANCE_REQUEST_FLASH = 3U,
  MMI_MAINTENANCE_REQUEST_PLAN_RETURN = 4U
};

static uint8_t RequestModeControl(void *ctx, uint8_t requestedControl)
{
  (void) ctx;

  switch (requestedControl)
  {
      case MMI_MAINTENANCE_REQUEST_ALL_RED:
      {
        UserStateReqSet(STATES_CLOSED);
        return TRUE;
      }

      case MMI_MAINTENANCE_REQUEST_DARK:
      {
        UserStateReqSet(STATES_NO_CONTROL);
        return TRUE;
      }

      case MMI_MAINTENANCE_REQUEST_FLASH:
      {
        UserStateReqSet(STATES_FLASH);
        return TRUE;
      }

      case MMI_MAINTENANCE_REQUEST_PLAN_RETURN:
      {
        UserStateReqFree();
        return TRUE;
      }

      default:
      {
        return FALSE;
      }
  }
}

static uint8_t RequestRelayState(void *ctx, uint8_t requestedState)
{
  (void) ctx;
  SetLCDPowerRelay(requestedState);
  SetLCDPowerRelayRequest(TRUE);
  return TRUE;
}

static uint8_t FactoryReset(void *ctx)
{
  (void) ctx;
  ReturnFactorySettings();
  return TRUE;
}

static uint8_t EnterIapMode(void *ctx)
{
  (void) ctx;
  return FALSE;
}

static uint8_t StartOutputTest(void *ctx)
{
  MmiMaintenanceAdapterCtx_t *adapter = (MmiMaintenanceAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return FALSE;
  }

  StartSSMTest(SSM_TEST_FROM_MMI);
  LogRequest(LOG_REQ_APPEND_ASYNCH,
             NULL,
             EVENT_USER_REQ_SSM_TEST_STARTS,
             0,
             0,
             0,
             0);
  adapter->selectedOutputNumber = 0U;
  adapter->outputTestActive = TRUE;
  return TRUE;
}

static uint8_t StopOutputTest(void *ctx)
{
  MmiMaintenanceAdapterCtx_t *adapter = (MmiMaintenanceAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return FALSE;
  }

  StopSSMTest();
  LoadProgramEnds();
  RestartProgram();
  LogRequest(LOG_REQ_APPEND_ASYNCH,
             NULL,
             EVENT_USER_REQ_SSM_TEST_ENDS,
             0,
             0,
             0,
             0);
  adapter->outputTestActive = FALSE;
  return TRUE;
}

static uint8_t SelectOutputTest(void *ctx, uint8_t outputNumber)
{
  MmiMaintenanceAdapterCtx_t *adapter = (MmiMaintenanceAdapterCtx_t *) ctx;

  if (adapter == NULL)
  {
    return FALSE;
  }

  adapter->selectedOutputNumber = outputNumber;
  SetOnSONo(outputNumber);
  return TRUE;
}

static uint8_t ReadOutputTestStatus(void *ctx,
                                    MmiMaintenanceOutputTestStatus_t *status)
{
  MmiMaintenanceAdapterCtx_t *adapter = (MmiMaintenanceAdapterCtx_t *) ctx;
  uint8_t currentGroup;

  if ((adapter == NULL) || (status == NULL))
  {
    return FALSE;
  }

  (void) memset(status, 0, sizeof(*status));
  status->outputNumber = (uint8_t) (adapter->selectedOutputNumber + 1U);
  currentGroup = (uint8_t) (adapter->selectedOutputNumber
                            / SIGNAL_OUTPUTS_PER_CURRENT_GROUP);
  status->powerNet = GetSOPowerRecordNet(adapter->selectedOutputNumber);
  status->power = GetSOPower(adapter->selectedOutputNumber);
  status->state = 0U;
  status->net = 0U;
  status->currentNow = GetCurrentMeasurement(currentGroup, CURRENT_NOW);
  status->currentMin = GetCurrentMeasurement(currentGroup, CURRENT_MIN);
  status->currentMax = GetCurrentMeasurement(currentGroup, CURRENT_MAX);

  return (uint8_t) (adapter->outputTestActive != 0U);
}

void MmiMaintenanceAdapterInit(MmiMaintenanceAdapterCtx_t *ctx)
{
  if (ctx != NULL)
  {
    (void) memset(ctx, 0, sizeof(*ctx));
  }
}

IMmiMaintenancePort_t MmiMaintenanceAdapterCreatePort(
  MmiMaintenanceAdapterCtx_t *ctx)
{
  IMmiMaintenancePort_t port;

  port.ctx = ctx;
  port.RequestModeControl = RequestModeControl;
  port.RequestRelayState = RequestRelayState;
  port.FactoryReset = FactoryReset;
  port.EnterIapMode = EnterIapMode;
  port.StartOutputTest = StartOutputTest;
  port.StopOutputTest = StopOutputTest;
  port.SelectOutputTest = SelectOutputTest;
  port.ReadOutputTestStatus = ReadOutputTestStatus;

  return port;
}
