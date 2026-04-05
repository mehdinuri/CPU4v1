#pragma once

/*
 * App/Adapters/STM32/ModemAdapter.h
 *
 * IModemPort concrete implementation for STM32H743.
 * Drives the GPRS modem via UART4 using an AT command state machine.
 * The state, signal quality and IMEI are updated by an AT command parser
 * running in a background task (TODO stub); the Domain reads them through
 * the port interface.
 */
#include "Ports/IModemPort.h"

#define MODEM_IMEI_LEN  16U   /* 15 digits + null terminator */

#ifdef STM32H743xx
#include "stm32h7xx_hal.h"
#endif

typedef struct
{
  ModemState_t state;                  /* Current modem lifecycle state   */
  uint8_t signalQuality;               /* 0-31 / 99 (AT+CSQ response)    */
  char imei[MODEM_IMEI_LEN];           /* IMEI string, null-terminated    */

  #ifdef STM32H743xx
  UART_HandleTypeDef *huart;           /* HAL UART handle (UART4)         */
  #else
  void *huart;
  #endif
} ModemAdapterCtx_t;

/**
 * Initialise the adapter context and store the UART handle.
 * Sets state to MODEM_STATE_OFF; signalQuality to 99 (unknown).
 */
#ifdef STM32H743xx
void ModemAdapter_Init(ModemAdapterCtx_t *ctx, UART_HandleTypeDef *huart);

#else
void ModemAdapter_Init(ModemAdapterCtx_t *ctx, void *huart);

#endif

/** Build an IModemPort_t wired to ctx. */
IModemPort_t ModemAdapter_CreatePort(ModemAdapterCtx_t *ctx);

/**
 * Update modem state from the AT command parser result.
 * Called by the modem background task — not from Domain code.
 */
void ModemAdapter_UpdateState(ModemAdapterCtx_t *ctx,
                              ModemState_t newState,
                              uint8_t signalQuality,
                              const char        *imei);
