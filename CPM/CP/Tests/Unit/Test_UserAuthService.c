#include "unity.h"

#include <string.h>

#include "Domain/Services/UserAuthService.h"

typedef struct
{
  UserAuthStoreRecord_t record;
  uint16_t legacyAdminPin;
  uint8_t loadOk;
  uint8_t saveOk;
  uint8_t legacyOk;
} TestUserAuthStoreCtx_t;

static uint8_t TestLoad(void *ctx, UserAuthStoreRecord_t *record)
{
  TestUserAuthStoreCtx_t *store = (TestUserAuthStoreCtx_t *) ctx;

  if ((store->loadOk == 0U) || (record == NULL))
  {
    return 0U;
  }

  *record = store->record;
  return 1U;
}

static uint8_t TestSave(void *ctx, const UserAuthStoreRecord_t *record)
{
  TestUserAuthStoreCtx_t *store = (TestUserAuthStoreCtx_t *) ctx;

  if ((store->saveOk == 0U) || (record == NULL))
  {
    return 0U;
  }

  store->record = *record;
  return 1U;
}

static uint8_t TestLoadLegacyAdminPin(void *ctx, uint16_t *adminPin)
{
  TestUserAuthStoreCtx_t *store = (TestUserAuthStoreCtx_t *) ctx;

  if ((store->legacyOk == 0U) || (adminPin == NULL))
  {
    return 0U;
  }

  *adminPin = store->legacyAdminPin;
  return 1U;
}

void setUp(void)
{
}

void tearDown(void)
{
}

void test_UserAuthServiceMigratesLegacyAdminPinAndAuthenticatesRoles(void)
{
  TestUserAuthStoreCtx_t storeCtx;
  IUserAuthStorePort_t storePort;
  UserAuthService_t service;

  (void) memset(&storeCtx, 0, sizeof(storeCtx));
  storeCtx.loadOk = 0U;
  storeCtx.saveOk = 1U;
  storeCtx.legacyOk = 1U;
  storeCtx.legacyAdminPin = 2468U;

  storePort.ctx = &storeCtx;
  storePort.Load = TestLoad;
  storePort.Save = TestSave;
  storePort.LoadLegacyAdminPin = TestLoadLegacyAdminPin;

  UserAuthServiceInit(&service);
  UserAuthServiceBind(&service, &storePort);

  TEST_ASSERT_EQUAL(USER_ROLE_ADMIN,
                    UserAuthServiceLogin(&service,
                                        USER_AUTH_ADMIN_USERNAME,
                                        2468U));
  TEST_ASSERT_EQUAL(USER_ROLE_ADMIN,
                    UserAuthServiceGetActiveRole(&service));

  UserAuthServiceLogout(&service);
  TEST_ASSERT_EQUAL(USER_ROLE_GUEST,
                    UserAuthServiceLogin(&service,
                                        USER_AUTH_GUEST_USERNAME,
                                        USER_AUTH_DEFAULT_GUEST_PIN));
}

void test_UserAuthServiceChangeAdminPinPersistsAndRejectsWeakPins(void)
{
  TestUserAuthStoreCtx_t storeCtx;
  IUserAuthStorePort_t storePort;
  UserAuthService_t service;

  (void) memset(&storeCtx, 0, sizeof(storeCtx));
  storeCtx.loadOk = 0U;
  storeCtx.saveOk = 1U;
  storeCtx.legacyOk = 1U;
  storeCtx.legacyAdminPin = 4321U;

  storePort.ctx = &storeCtx;
  storePort.Load = TestLoad;
  storePort.Save = TestSave;
  storePort.LoadLegacyAdminPin = TestLoadLegacyAdminPin;

  UserAuthServiceInit(&service);
  UserAuthServiceBind(&service, &storePort);

  TEST_ASSERT_EQUAL(USER_AUTH_CHANGE_INVALID_CURRENT,
                    UserAuthServiceChangeAdminPin(&service, 1111U, 2468U));
  TEST_ASSERT_EQUAL(USER_AUTH_CHANGE_INVALID_NEW,
                    UserAuthServiceChangeAdminPin(&service, 4321U, 0U));
  TEST_ASSERT_EQUAL(USER_AUTH_CHANGE_OK,
                    UserAuthServiceChangeAdminPin(&service, 4321U, 2468U));
  TEST_ASSERT_EQUAL(USER_ROLE_ADMIN,
                    UserAuthServiceLogin(&service,
                                        USER_AUTH_ADMIN_USERNAME,
                                        2468U));
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_UserAuthServiceMigratesLegacyAdminPinAndAuthenticatesRoles);
  RUN_TEST(test_UserAuthServiceChangeAdminPinPersistsAndRejectsWeakPins);
  return UNITY_END();
}
