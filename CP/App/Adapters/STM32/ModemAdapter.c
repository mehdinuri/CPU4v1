/*
 * App/Adapters/STM32/ModemAdapter.c
 *
 * IModemPort implementation — UART4 AT command state machine.
 *
 * The Domain only reads modem state/quality/IMEI — it does not drive the
 * AT command exchange directly.  A separate modem background task (or an
 * extension to NetworkTask) will:
 *   1. Send AT commands via HAL_UART_Transmit_IT().
 *   2. Parse replies in the Rx callback.
 *   3. Call ModemAdapter_UpdateState() to push results into the context.
 *
 * All UART interaction is a TODO stub.
 */
#include "ModemAdapter.h"
#include <string.h>

/* --------------------------------------------------------------------------
 * Forward declarations
 * --------------------------------------------------------------------------*/
static ModemState_t ModemCB_GetState(void *ctx);
static uint8_t ModemCB_GetSignalQuality(void *ctx);
static void ModemCB_GetImei(void *ctx, char *outBuf, uint8_t bufLen);

/* --------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------*/

#ifdef STM32H743xx
void ModemAdapter_Init(ModemAdapterCtx_t *ctx, UART_HandleTypeDef *huart)
#else
void ModemAdapter_Init(ModemAdapterCtx_t *ctx, void *huart)
#endif
{
  memset(ctx, 0, sizeof(*ctx));
  ctx->huart = huart;
  ctx->state = MODEM_STATE_OFF;
  ctx->signalQuality = 99U;     /* 99 = unknown per AT+CSQ specification */
  ctx->imei[0] = '\0';

  #ifdef STM32H743xx

  /* TODO: HAL impl — assert MODEM_PWR_KEY GPIO, wait >1 s, then start
   * AT command Sequence to determine if modem is already running.
   *
   * HAL_GPIO_WritePin(MODEM_PWR_KEY_GPIO_Port, MODEM_PWR_KEY_Pin,
   *                   GPIO_PIN_SET);
   * osDelay(1200);
   * HAL_GPIO_WritePin(MODEM_PWR_KEY_GPIO_Port, MODEM_PWR_KEY_Pin,
   *                   GPIO_PIN_RESET);
   */
  #endif
}

IModemPort_t ModemAdapter_CreatePort(ModemAdapterCtx_t *ctx)
{
  IModemPort_t port;

  port.ctx = ctx;
  port.getState = ModemCB_GetState;
  port.getSignalQuality = ModemCB_GetSignalQuality;
  port.getImei = ModemCB_GetImei;

  return port;
}

void ModemAdapter_UpdateState(ModemAdapterCtx_t *ctx,
                              ModemState_t newState,
                              uint8_t signalQuality,
                              const char        *imei)
{
  ctx->state = newState;
  ctx->signalQuality = signalQuality;
  if (imei != NULL)
  {
    /* strncpy with explicit null termination — no reliance on source length. */
    uint8_t i;

    for (i = 0U; i < (MODEM_IMEI_LEN - 1U) && imei[i] != '\0'; i++)
    {
      ctx->imei[i] = imei[i];
    }

    ctx->imei[i] = '\0';
  }
}

/* --------------------------------------------------------------------------
 * Port callbacks
 * --------------------------------------------------------------------------*/

static ModemState_t ModemCB_GetState(void *vctx)
{
  ModemAdapterCtx_t *ctx = (ModemAdapterCtx_t *) vctx;

  return ctx->state;
}

static uint8_t ModemCB_GetSignalQuality(void *vctx)
{
  ModemAdapterCtx_t *ctx = (ModemAdapterCtx_t *) vctx;

  return ctx->signalQuality;
}

static void ModemCB_GetImei(void *vctx, char *outBuf, uint8_t bufLen)
{
  ModemAdapterCtx_t *ctx = (ModemAdapterCtx_t *) vctx;

  if ((outBuf == NULL) || (bufLen == 0U) )
  {
    return;
  }

  uint8_t i;

  for (i = 0U; i < (bufLen - 1U) && ctx->imei[i] != '\0'; i++)
  {
    outBuf[i] = ctx->imei[i];
  }

  outBuf[i] = '\0';
}
