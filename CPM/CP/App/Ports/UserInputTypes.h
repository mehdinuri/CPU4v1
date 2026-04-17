/* App/Ports/UserInputTypes.h
 *
 * Shared HAL-free keypad types and key-code definitions.
 */
#ifndef USER_INPUT_TYPES_H
#define USER_INPUT_TYPES_H

#include <stdint.h>

typedef uint32_t KeypadSnapshot_t;

#define KEYPAD_KEY_COUNT      16U
#define KEYPAD_SNAPSHOT_NONE  ((KeypadSnapshot_t) 0U)

#define KEY_0            0U
#define KEY_1            1U
#define KEY_2            2U
#define KEY_3            3U
#define KEY_4            4U
#define KEY_5            5U
#define KEY_6            6U
#define KEY_7            7U
#define KEY_8            8U
#define KEY_9            9U
#define KEY_LEFT         10U
#define KEY_RIGHT        11U
#define KEY_CLEAR        12U
#define KEY_UP           13U
#define KEY_DELETE_DOWN  14U
#define KEY_ENTER        15U
#define KEY_NONE         0xFFU

static inline KeypadSnapshot_t KeypadSnapshotBit(uint8_t key)
{
  if (key >= KEYPAD_KEY_COUNT)
  {
    return KEYPAD_SNAPSHOT_NONE;
  }

  return (KeypadSnapshot_t) 1UL << key;
}

static inline uint8_t KeypadSnapshotHasKey(KeypadSnapshot_t snapshot,
                                           uint8_t key)
{
  return ((snapshot & KeypadSnapshotBit(key)) != KEYPAD_SNAPSHOT_NONE)
         ? 1U
         : 0U;
}

#endif /* USER_INPUT_TYPES_H */
