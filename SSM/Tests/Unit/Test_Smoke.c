#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_ssm_infrastructure(void)
{
  TEST_PASS_MESSAGE("SSM host test infrastructure is operational.");
}

int main(void)
{
  UNITY_BEGIN();
  RUN_TEST(test_ssm_infrastructure);

  return UNITY_END();
}
