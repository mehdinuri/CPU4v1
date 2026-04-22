/* App/Ports/IUserAuthStorePort.h
 *
 * Storage boundary for local guest/admin authentication state.
 */
#ifndef IUSER_AUTH_STORE_PORT_H
#define IUSER_AUTH_STORE_PORT_H

#include <stdint.h>

typedef struct
{
  uint32_t magic;
  uint16_t version;
  uint16_t reserved0;
  uint32_t guestPinHash;
  uint32_t adminPinHash;
} UserAuthStoreRecord_t;

typedef struct
{
  void *ctx;

  uint8_t (*Load)(void *ctx, UserAuthStoreRecord_t *record);
  uint8_t (*Save)(void *ctx, const UserAuthStoreRecord_t *record);
} IUserAuthStorePort_t;

static inline uint8_t UserAuthStoreLoad(IUserAuthStorePort_t *p,
                                        UserAuthStoreRecord_t *record)
{
  return p->Load(p->ctx, record);
}

static inline uint8_t UserAuthStoreSave(IUserAuthStorePort_t *p,
                                        const UserAuthStoreRecord_t *record)
{
  return p->Save(p->ctx, record);
}

#endif /* IUSER_AUTH_STORE_PORT_H */
