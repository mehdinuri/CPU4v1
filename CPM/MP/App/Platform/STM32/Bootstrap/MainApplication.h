/* App/Platform/STM32/Bootstrap/MainApplication.h
 *
 * Entry point for the new hexagonal MP architecture. Called from
 * main.c after CubeMX HAL init (MX_* functions) and BEFORE the RTOS
 * scheduler starts. Instantiates every adapter, populates the global
 * ports, wires them into domain services, and spawns the MP tasks.
 *
 * When the legacy FreeRTOS task tree in app_freertos.c is retired,
 * MainApplication_Init() becomes the single owner of the MP task set.
 */
#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H

void MainApplication_Init(void);

#endif /* MAIN_APPLICATION_H */
