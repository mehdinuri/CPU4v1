/* App/Ports/IUserPort.h
 */
#ifndef IUSER_PORT_H
#define IUSER_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*ValidateLogin)(void *ctx, uint16_t username, uint16_t password);
} IUserPort_t;

static inline uint8_t UserValidateLogin(IUserPort_t *p,
                                        uint16_t username,
                                        uint16_t password)
{
  return p->ValidateLogin(p->ctx, username, password);
}

#endif /* IUSER_PORT_H */
