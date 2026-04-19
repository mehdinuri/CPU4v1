/* App/Domain/Services/UserAuthService.h
 *
 * Role-based local authentication with fixed guest/admin identities and
 * persisted hashed PINs.
 */
#ifndef USER_AUTH_SERVICE_H
#define USER_AUTH_SERVICE_H

#include <stdint.h>

#include "Ports/IUserAuthStorePort.h"
#include "Ports/IUserPort.h"

#define USER_AUTH_GUEST_USERNAME 0U
#define USER_AUTH_ADMIN_USERNAME 1111U
#define USER_AUTH_DEFAULT_GUEST_PIN 0U
#define USER_AUTH_DEFAULT_ADMIN_PIN 1111U

typedef enum
{
  USER_AUTH_CHANGE_OK = 0,
  USER_AUTH_CHANGE_INVALID_CURRENT = 1,
  USER_AUTH_CHANGE_INVALID_NEW = 2,
  USER_AUTH_CHANGE_STORE_FAILED = 3,
  USER_AUTH_CHANGE_INTERNAL_ERROR = 4
} UserAuthChangeStatus_t;

typedef struct
{
  IUserAuthStorePort_t *storePort;
  UserAuthStoreRecord_t record;
  UserRole_t activeRole;
  uint8_t loaded;
} UserAuthService_t;

void UserAuthServiceInit(UserAuthService_t *service);
void UserAuthServiceBind(UserAuthService_t *service,
                         IUserAuthStorePort_t *storePort);
UserRole_t UserAuthServiceLogin(UserAuthService_t *service,
                                uint16_t username,
                                uint16_t password);
UserRole_t UserAuthServiceGetActiveRole(const UserAuthService_t *service);
void UserAuthServiceLogout(UserAuthService_t *service);
uint16_t UserAuthServiceGetAdminUsername(const UserAuthService_t *service);
uint8_t UserAuthServiceIsAdminValid(const UserAuthService_t *service);
UserAuthChangeStatus_t UserAuthServiceChangeAdminPin(
  UserAuthService_t *service,
  uint16_t currentPassword,
  uint16_t newPassword);

#endif /* USER_AUTH_SERVICE_H */
