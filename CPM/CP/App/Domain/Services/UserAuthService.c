/* App/Domain/Services/UserAuthService.c */
#include "UserAuthService.h"

#include <string.h>

#define USER_AUTH_STORE_MAGIC 0x41555448UL
#define USER_AUTH_STORE_VERSION 1U

static uint8_t PinIsValid(uint16_t pin)
{
  return (uint8_t) (pin <= 9999U);
}

static uint32_t HashPin(UserRole_t role, uint16_t pin)
{
  uint32_t hash = 0x811C9DC5UL;
  uint32_t value = (((uint32_t) role & 0xFFUL) << 24)
                   | ((uint32_t) pin & 0xFFFFUL)
                   | 0x0059748BUL;
  uint8_t index;

  for (index = 0U; index < 4U; index++)
  {
    hash ^= (value >> (index * 8U)) & 0xFFUL;
    hash *= 16777619UL;
  }

  hash ^= 0xA55AF00DUL;
  hash ^= hash >> 16;
  hash *= 0x7FEB352DUL;
  hash ^= hash >> 15;
  hash *= 0x846CA68BUL;
  hash ^= hash >> 16;

  return hash;
}

static void RecordSetDefaults(UserAuthStoreRecord_t *record)
{
  if (record == NULL)
  {
    return;
  }

  (void) memset(record, 0, sizeof(*record));
  record->magic = USER_AUTH_STORE_MAGIC;
  record->version = USER_AUTH_STORE_VERSION;
  record->guestPinHash = HashPin(USER_ROLE_GUEST, USER_AUTH_DEFAULT_GUEST_PIN);
  record->adminPinHash = HashPin(USER_ROLE_ADMIN, USER_AUTH_DEFAULT_ADMIN_PIN);
}

static uint8_t RecordIsValid(const UserAuthStoreRecord_t *record)
{
  if (record == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((record->magic == USER_AUTH_STORE_MAGIC)
                    && (record->version == USER_AUTH_STORE_VERSION)
                    && (record->guestPinHash != 0U)
                    && (record->adminPinHash != 0U));
}

static uint8_t EnsureLoaded(UserAuthService_t *service)
{
  UserAuthStoreRecord_t record;

  if (service == NULL)
  {
    return 0U;
  }

  if (service->loaded != 0U)
  {
    return 1U;
  }

  if (service->storePort == NULL)
  {
    return 0U;
  }

  (void) memset(&record, 0, sizeof(record));
  if ((UserAuthStoreLoad(service->storePort, &record) != 0U)
      && (RecordIsValid(&record) != 0U))
  {
    service->record = record;
    service->loaded = 1U;
    return 1U;
  }

  RecordSetDefaults(&service->record);
  (void) UserAuthStoreSave(service->storePort, &service->record);
  service->loaded = 1U;
  return 1U;
}

void UserAuthServiceInit(UserAuthService_t *service)
{
  if (service != NULL)
  {
    (void) memset(service, 0, sizeof(*service));
    service->activeRole = USER_ROLE_NONE;
  }
}

void UserAuthServiceBind(UserAuthService_t *service,
                         IUserAuthStorePort_t *storePort)
{
  if (service != NULL)
  {
    service->storePort = storePort;
    service->loaded = 0U;
    service->activeRole = USER_ROLE_NONE;
  }
}

UserRole_t UserAuthServiceLogin(UserAuthService_t *service,
                                uint16_t username,
                                uint16_t password)
{
  if ((EnsureLoaded(service) == 0U) || (PinIsValid(password) == 0U))
  {
    return USER_ROLE_NONE;
  }

  service->activeRole = USER_ROLE_NONE;
  if ((username == USER_AUTH_GUEST_USERNAME)
      && (service->record.guestPinHash == HashPin(USER_ROLE_GUEST, password)))
  {
    service->activeRole = USER_ROLE_GUEST;
  }
  else if ((username == USER_AUTH_ADMIN_USERNAME)
           && (service->record.adminPinHash
               == HashPin(USER_ROLE_ADMIN, password)))
  {
    service->activeRole = USER_ROLE_ADMIN;
  }

  return service->activeRole;
}

UserRole_t UserAuthServiceGetActiveRole(const UserAuthService_t *service)
{
  if (service == NULL)
  {
    return USER_ROLE_NONE;
  }

  return service->activeRole;
}

void UserAuthServiceLogout(UserAuthService_t *service)
{
  if (service != NULL)
  {
    service->activeRole = USER_ROLE_NONE;
  }
}

uint16_t UserAuthServiceGetAdminUsername(const UserAuthService_t *service)
{
  (void) service;
  return USER_AUTH_ADMIN_USERNAME;
}

uint8_t UserAuthServiceIsAdminValid(const UserAuthService_t *service)
{
  return EnsureLoaded((UserAuthService_t *) service);
}

UserAuthChangeStatus_t UserAuthServiceChangeAdminPin(
  UserAuthService_t *service,
  uint16_t currentPassword,
  uint16_t newPassword)
{
  UserAuthStoreRecord_t nextRecord;

  if (EnsureLoaded(service) == 0U)
  {
    return USER_AUTH_CHANGE_INTERNAL_ERROR;
  }

  if ((PinIsValid(currentPassword) == 0U)
      || (service->record.adminPinHash
          != HashPin(USER_ROLE_ADMIN, currentPassword)))
  {
    return USER_AUTH_CHANGE_INVALID_CURRENT;
  }

  if ((PinIsValid(newPassword) == 0U)
      || (newPassword == USER_AUTH_DEFAULT_GUEST_PIN)
      || (newPassword == USER_AUTH_ADMIN_USERNAME)
      || (newPassword == currentPassword))
  {
    return USER_AUTH_CHANGE_INVALID_NEW;
  }

  nextRecord = service->record;
  nextRecord.adminPinHash = HashPin(USER_ROLE_ADMIN, newPassword);
  if (UserAuthStoreSave(service->storePort, &nextRecord) == 0U)
  {
    return USER_AUTH_CHANGE_STORE_FAILED;
  }

  service->record = nextRecord;
  return USER_AUTH_CHANGE_OK;
}
