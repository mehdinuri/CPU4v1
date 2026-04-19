/* App/Ports/IModemConfigPort.h
 *
 * Port for persisted modem-type configuration.
 */
#ifndef IMODEM_CONFIG_PORT_H
#define IMODEM_CONFIG_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetModemType)(void *ctx);
  void (*SetModemType)(void *ctx, uint8_t modemType);
  uint8_t (*SaveConfig)(void *ctx);
  uint8_t (*IsValidModemType)(void *ctx, uint8_t modemType);
} IModemConfigPort_t;

static inline uint8_t IModemConfigPort_GetModemType(IModemConfigPort_t *p)
{
  return p->GetModemType(p->ctx);
}

static inline void IModemConfigPort_SetModemType(IModemConfigPort_t *p,
                                                 uint8_t modemType)
{
  p->SetModemType(p->ctx, modemType);
}

static inline uint8_t IModemConfigPort_SaveConfig(IModemConfigPort_t *p)
{
  return p->SaveConfig(p->ctx);
}

static inline uint8_t IModemConfigPort_IsValidModemType(
  IModemConfigPort_t *p,
  uint8_t modemType)
{
  return p->IsValidModemType(p->ctx, modemType);
}

#endif /* IMODEM_CONFIG_PORT_H */
