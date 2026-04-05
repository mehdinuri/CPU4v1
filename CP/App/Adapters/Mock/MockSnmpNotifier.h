#pragma once

/*
 * App/Adapters/Mock/MockSnmpNotifier.h
 *
 * In-memory ISnmpNotifierPort implementation for unit tests.
 * Records every trap emission so tests can assert on type and payload.
 */
#include "Ports/ISNMPNotifierPort.h"
#include <string.h>

#define MOCK_SNMP_TRAP_LOG_MAX  64U

typedef struct
{
  SnmpTrapType_t type;
  uint32_t payload;
} MockTrapEntry_t;

typedef struct
{
  MockTrapEntry_t log[MOCK_SNMP_TRAP_LOG_MAX];
  uint32_t count;
} MockSnmpNotifierCtx_t;

/* Initialise ctx: zero logged traps. */
static inline void MockSnmpNotifier_Init(MockSnmpNotifierCtx_t *ctx)
{
  memset(ctx, 0, sizeof(*ctx));
}

/* Return true if at least one trap of the given type was recorded. */
static inline bool MockSnmpNotifier_HasTrap(const MockSnmpNotifierCtx_t *ctx,
                                            SnmpTrapType_t type)
{
  for (uint32_t i = 0U; i < ctx->count; i++)
  {
    if (ctx->log[i].type == type)
    {
      return true;
    }
  }

  return false;
}

/* Count traps of a specific type. */
static inline uint32_t MockSnmpNotifier_CountTrap(
  const MockSnmpNotifierCtx_t *ctx,
  SnmpTrapType_t type)
{
  uint32_t n = 0U;

  for (uint32_t i = 0U; i < ctx->count; i++)
  {
    if (ctx->log[i].type == type)
    {
      n++;
    }
  }

  return n;
}

/* Build an ISnmpNotifierPort_t wired to ctx. */
ISnmpNotifierPort_t MockSnmpNotifier_Create(MockSnmpNotifierCtx_t *ctx);
