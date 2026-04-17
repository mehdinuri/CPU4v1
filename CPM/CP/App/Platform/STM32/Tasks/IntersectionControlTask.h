/* App/Platform/STM32/Tasks/IntersectionControlTask.h
 *
 * 10 ms platform task that advances the new intersection engine and
 * dispatches its output image through the MMU/output-driver seam.
 */
#ifndef INTERSECTION_CONTROL_TASK_H
#define INTERSECTION_CONTROL_TASK_H

void IntersectionControlTaskFunc(void *argument);

#endif /* INTERSECTION_CONTROL_TASK_H */
