/**
 ******************************************************************************
 * @file    Domain/CanRxBackpressure.h
 * @brief   Consecutive-drop counter used to promote silent CAN Rx loss to
 *          a sticky fault flag.
 *
 *          The CAN Rx ISR must try to allocate a pool slot and enqueue the
 *          frame. Under overload either can fail. A handful of losses is
 *          normal jitter; sustained loss indicates real backpressure and
 *          should reach the maintenance layer.
 *
 *          This module is pure: the caller owns concurrency. The only
 *          legitimate caller (can_msg_parser.c) drives it from a single
 *          ISR context, so no atomics or locks are required inside.
 ******************************************************************************
 */

#ifndef DOMAIN_CAN_RX_BACKPRESSURE_H
#define DOMAIN_CAN_RX_BACKPRESSURE_H

#include <stdint.h>

typedef struct
{
  uint32_t consecutiveDrops;
} CanRxBackpressure_t;

/**
 * @brief Zero the counter.
 */
void CanRxBackpressure_Reset(CanRxBackpressure_t *state);

/**
 * @brief A frame successfully reached the Rx queue — end the drop streak.
 */
void CanRxBackpressure_RecordSuccess(CanRxBackpressure_t *state);

/**
 * @brief A frame was dropped (alloc or enqueue failure).
 *
 * @param state      Counter state.
 * @param threshold  Consecutive-drop count at which we report overrun.
 * @return 1 if the post-increment count is >= threshold, 0 otherwise.
 *         The return is meant to be fed into an idempotent sticky flag;
 *         repeated 1-returns while still in overrun are expected.
 */
uint8_t CanRxBackpressure_RecordDrop(CanRxBackpressure_t *state,
                                     uint32_t threshold);

#endif /* DOMAIN_CAN_RX_BACKPRESSURE_H */

/************************ (C) COPYRIGHT TEKNOTEL ELEKTRONIK ****END OF FILE****/
