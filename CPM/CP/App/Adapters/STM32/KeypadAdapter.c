/* App/Adapters/STM32/KeypadAdapter.c
 *
 * IUserInputPort concrete implementation for the legacy 4x4 LCD keypad.
 *
 * ScanSnapshot() drives one row high at a time and reads the four
 * column inputs. Each active logical key is reported in the returned
 * bitmask snapshot. Only one key per row is considered, matching the
 * legacy lcd.c scan semantics.
 */
#include "KeypadAdapter.h"
#include "main.h"
#include "stm32h7xx_hal.h"

static const uint8_t s_keyMap[KEYPAD_ROWS][KEYPAD_COLS] =
{
  { KEY_ENTER, KEY_0, KEY_LEFT, KEY_CLEAR },
  { KEY_UP, KEY_9, KEY_6, KEY_3 },
  { KEY_RIGHT, KEY_8, KEY_5, KEY_2 },
  { KEY_DELETE_DOWN, KEY_7, KEY_4, KEY_1 }
};

/* ------------------------------------------------------------------
 * Private GPIO helpers (formerly in gpio.c)
 * ------------------------------------------------------------------ */
static void KeypadGPIOInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /* Init rows as push-pull outputs */
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pin = KEYPAD_ROW1_Pin | KEYPAD_ROW2_Pin | KEYPAD_ROW3_Pin;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = KEYPAD_ROW4_Pin;
  HAL_GPIO_Init(KEYPAD_ROW4_GPIO_Port, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = KEYPAD_ROW5_Pin;
  HAL_GPIO_Init(KEYPAD_ROW5_GPIO_Port, &GPIO_InitStructure);

  /* Init columns as pull-down inputs */
  GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructure.Pin =
    KEYPAD_COL1_Pin | KEYPAD_COL2_Pin | KEYPAD_COL3_Pin | KEYPAD_COL4_Pin;
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
}

static void KeypadSetAllRowsLow(void)
{
  HAL_GPIO_WritePin(KEYPAD_ROW5_GPIO_Port, KEYPAD_ROW5_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(KEYPAD_ROW4_GPIO_Port, KEYPAD_ROW4_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(KEYPAD_ROW3_GPIO_Port, KEYPAD_ROW3_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(KEYPAD_ROW2_GPIO_Port, KEYPAD_ROW2_Pin,
                    GPIO_PIN_RESET);
  HAL_GPIO_WritePin(KEYPAD_ROW1_GPIO_Port, KEYPAD_ROW1_Pin,
                    GPIO_PIN_RESET);
}

static void KeypadSetActiveRow(uint8_t row)
{
  KeypadSetAllRowsLow();

  switch (row)
  {
      case 0U:
      {
        HAL_GPIO_WritePin(KEYPAD_ROW1_GPIO_Port, KEYPAD_ROW1_Pin, GPIO_PIN_SET);
        break;
      }

      case 1U:
      {
        HAL_GPIO_WritePin(KEYPAD_ROW2_GPIO_Port, KEYPAD_ROW2_Pin, GPIO_PIN_SET);
        break;
      }

      case 2U:
      {
        HAL_GPIO_WritePin(KEYPAD_ROW3_GPIO_Port, KEYPAD_ROW3_Pin, GPIO_PIN_SET);
        break;
      }

      case 3U:
      {
        HAL_GPIO_WritePin(KEYPAD_ROW4_GPIO_Port, KEYPAD_ROW4_Pin, GPIO_PIN_SET);
        break;
      }

      default:
      {
        break;
      }
  }
}

static GPIO_PinState KeypadReadColumn(uint8_t colNo)
{
  switch (colNo)
  {
      case 1U:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL1_GPIO_Port, KEYPAD_COL1_Pin);
      }

      case 2U:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL2_GPIO_Port, KEYPAD_COL2_Pin);
      }

      case 3U:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL3_GPIO_Port, KEYPAD_COL3_Pin);
      }

      case 4U:
      {
        return HAL_GPIO_ReadPin(KEYPAD_COL4_GPIO_Port, KEYPAD_COL4_Pin);
      }

      default:
      {
        return GPIO_PIN_RESET;
      }
  }
}

/* ------------------------------------------------------------------
 * Port callback
 * ------------------------------------------------------------------ */
static KeypadSnapshot_t AdapterScanSnapshot(void *ctx)
{
  uint8_t row;
  KeypadSnapshot_t snapshot = KEYPAD_SNAPSHOT_NONE;

  for (row = 0U; row < KEYPAD_ROWS; row++)
  {
    KeypadSetActiveRow(row);

    if (KeypadReadColumn(1U) == GPIO_PIN_SET)
    {
      snapshot |= KeypadSnapshotBit(s_keyMap[row][0]);
    }
    else if (KeypadReadColumn(2U) == GPIO_PIN_SET)
    {
      snapshot |= KeypadSnapshotBit(s_keyMap[row][1]);
    }
    else if (KeypadReadColumn(3U) == GPIO_PIN_SET)
    {
      snapshot |= KeypadSnapshotBit(s_keyMap[row][2]);
    }
    else if (KeypadReadColumn(4U) == GPIO_PIN_SET)
    {
      snapshot |= KeypadSnapshotBit(s_keyMap[row][3]);
    }
  }

  KeypadSetAllRowsLow();
  ((KeypadAdapterCtx_t *) ctx)->lastSnapshot = snapshot;

  return snapshot;
}

/* ------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------ */
void KeypadAdapterInit(KeypadAdapterCtx_t *ctx)
{
  ctx->lastSnapshot = KEYPAD_SNAPSHOT_NONE;
  KeypadGPIOInit();
  KeypadSetAllRowsLow();
}

IUserInputPort_t KeypadAdapterCreatePort(KeypadAdapterCtx_t *ctx)
{
  IUserInputPort_t port;

  port.ctx = ctx;
  port.ScanSnapshot = AdapterScanSnapshot;

  return port;
}
