/* App/Ports/IAdminInfoPort.h
 *
 * Port for persisted admin user metadata.
 */
#ifndef IADMIN_INFO_PORT_H
#define IADMIN_INFO_PORT_H

#include <stdint.h>

typedef struct
{
  uint16_t username;
  uint8_t validity;
} AdminInfo_t;

typedef struct
{
  void *ctx;

  uint8_t (*Read)(void *ctx);
  void (*Get)(void *ctx, AdminInfo_t *info);
} IAdminInfoPort_t;

static inline uint8_t AdminInfoPort_Read(IAdminInfoPort_t *p)
{
  return p->Read(p->ctx);
}

static inline void AdminInfoPort_Get(IAdminInfoPort_t *p, AdminInfo_t *info)
{
  p->Get(p->ctx, info);
}

#endif /* IADMIN_INFO_PORT_H */
