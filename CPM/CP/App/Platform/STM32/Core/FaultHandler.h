/*
 * App/Platform/STM32/Core/FaultHandler.h
 *
 * Hard-fault diagnostic capture.
 * Call FaultHandler_Capture() from inside HardFault_Handler() immediately
 * on entry; the handler then hangs in its generated while(1) as before.
 * All register snapshots are retained in static volatile storage so a
 * JTAG/SWD debugger can inspect them after stopping the core.
 */
#pragma once

void FaultHandler_Capture(void);
