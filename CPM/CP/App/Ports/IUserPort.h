/* App/Ports/IUserPort.h
 */
#ifndef IUSER_PORT_H
#define IUSER_PORT_H

#include <stdint.h>

typedef enum
{
  USER_ROLE_NONE = 0,
  USER_ROLE_GUEST = 1,
  USER_ROLE_ADMIN = 2
} UserRole_t;

typedef struct
{
  void *ctx;

  UserRole_t (*Login)(void *ctx, uint16_t username, uint16_t password);
  UserRole_t (*GetActiveRole)(void *ctx);
  void (*Logout)(void *ctx);
  uint8_t (*ChangeAdminPassword)(void *ctx,
                                 uint16_t currentPassword,
                                 uint16_t newPassword);
} IUserPort_t;

static inline UserRole_t UserLogin(IUserPort_t *p,
                                   uint16_t username,
                                   uint16_t password)
{
  return p->Login(p->ctx, username, password);
}

static inline UserRole_t UserGetActiveRole(IUserPort_t *p)
{
  return p->GetActiveRole(p->ctx);
}

static inline uint8_t UserCanAccessConfiguration(IUserPort_t *p)
{
  return (uint8_t) (UserGetActiveRole(p) == USER_ROLE_ADMIN);
}

static inline void UserLogout(IUserPort_t *p)
{
  p->Logout(p->ctx);
}

static inline uint8_t UserChangeAdminPassword(IUserPort_t *p,
                                              uint16_t currentPassword,
                                              uint16_t newPassword)
{
  return p->ChangeAdminPassword(p->ctx, currentPassword, newPassword);
}

static inline uint8_t UserValidateLogin(IUserPort_t *p,
                                        uint16_t username,
                                        uint16_t password)
{
  return (uint8_t) (UserLogin(p, username, password) != USER_ROLE_NONE);
}

#endif /* IUSER_PORT_H */
