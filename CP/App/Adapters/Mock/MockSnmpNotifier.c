/*
 * App/Adapters/Mock/MockSnmpNotifier.c
 */
#include "MockSnmpNotifier.h"

static void mock_send_trap(void *ctx, SnmpTrapType_t type, uint32_t payload)
{
  MockSnmpNotifierCtx_t *m = (MockSnmpNotifierCtx_t *) ctx;

  if (m->count < MOCK_SNMP_TRAP_LOG_MAX)
  {
    m->log[m->count].type = type;
    m->log[m->count].payload = payload;
    m->count++;
  }
}

ISnmpNotifierPort_t MockSnmpNotifier_Create(MockSnmpNotifierCtx_t *ctx)
{
  ISnmpNotifierPort_t port;

  port.ctx = ctx;
  port.sendTrap = mock_send_trap;

  return port;
}
