/* App/Ports/IDisplayPort.h
 *
 * Port interface for a character display (20x4 LCD).
 * Domain code calls the inline helpers; the concrete adapter wires
 * them to lcdDrv.c on STM32 or to a framebuffer mock on host.
 */
#ifndef IDISPLAY_PORT_H
#define IDISPLAY_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  void (*Clear)(void *ctx);
  void (*Write)(void *ctx, uint8_t row, uint8_t col,
                const char *text, uint8_t len);
  void (*PowerOn)(void *ctx);
  void (*PowerOff)(void *ctx);
} IDisplayPort_t;

static inline void DisplayClear(IDisplayPort_t *p)
{
  p->Clear(p->ctx);
}

static inline void DisplayWrite(IDisplayPort_t *p, uint8_t row, uint8_t col,
                                const char *text, uint8_t len)
{
  p->Write(p->ctx, row, col, text, len);
}

static inline void DisplayPowerOn(IDisplayPort_t *p)
{
  p->PowerOn(p->ctx);
}

static inline void DisplayPowerOff(IDisplayPort_t *p)
{
  p->PowerOff(p->ctx);
}

#endif /* IDISPLAY_PORT_H */
