/* App/Platform/STM32/Tasks/Tasks.h
 *
 * Task lifecycle entry-point. Called by MainApplication_Init() once
 * ports + domain services are wired. Creates every FreeRTOS task in
 * the new architecture and leaves them suspended until the scheduler
 * starts.
 */
#ifndef TASKS_H
#define TASKS_H

void Tasks_Start(void);

#endif /* TASKS_H */
