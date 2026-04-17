/*
 * App/Platform/STM32/Core/FaultHandler.c
 *
 * Snapshots ARM Cortex-M7 fault-status registers for post-mortem
 * debugging.  All state is kept in static volatile variables so a
 * debugger can inspect them by name after stopping the core.
 *
 * Moved from Core/Src/stm32h7xx_it.c USER CODE blocks.
 */
#include "FaultHandler.h"
#include "stm32h7xx_hal.h"

/* Configurable Fault Status Register (UFSR | BFSR | MMFSR) */
static volatile uint32_t s_cfsr = 0U;
/* Hard Fault Status Register */
static volatile uint32_t s_hfsr = 0U;
/* Bus Fault Address Register (valid when BFARVALID in CFSR is set) */
static volatile uint32_t s_bfar = 0U;
/* MemManage Fault Address Register (valid when MMARVALID in CFSR is set) */
static volatile uint32_t s_mmfar = 0U;
/* Main-stack pointer at fault entry */
static volatile uint32_t s_sp = 0U;
/* Program counter of the faulting instruction (frame offset 6) */
static volatile uint32_t s_pc = 0U;

void FaultHandler_Capture(void)
{
  s_cfsr = SCB->CFSR;
  s_hfsr = SCB->HFSR;
  s_bfar = SCB->BFAR;
  s_mmfar = SCB->MMFAR;
  s_sp = __get_MSP();
  s_pc = ((uint32_t *) s_sp)[6];    /* PC is saved-exception-frame offset 6 */
}
