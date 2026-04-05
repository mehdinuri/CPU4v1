#pragma once

/*
 * App/Adapters/STM32/KeypadAdapter.h
 *
 * IUserInputPort concrete implementation for STM32H743.
 * Raw GPIO keypad input is translated to UserCommand_t values and pushed
 * into a fixed-size circular FIFO.  The UITask calls
 * KeypadAdapter_PushCommand() from a GPIO EXTI callback; the Domain polls
 * hasCommand() / dequeue() through the port interface each 100 ms tick.
 */
#include "Ports/IUserInputPort.h"

#define KEYPAD_QUEUE_DEPTH  16U   /* Maximum pending commands before drop */

typedef struct
{
  UserCommand_t queue[KEYPAD_QUEUE_DEPTH];   /* Circular FIFO                */
  uint8_t head;                              /* Index of next item to read    */
  uint8_t tail;                              /* Index to write next item      */
  uint8_t count;                             /* Items currently in the queue  */
} KeypadAdapterCtx_t;

/** Initialise the adapter context (empty queue). */
void KeypadAdapter_Init(KeypadAdapterCtx_t *ctx);

/** Build an IUserInputPort_t wired to ctx. */
IUserInputPort_t KeypadAdapter_CreatePort(KeypadAdapterCtx_t *ctx);

/**
 * Push a command into the circular queue.
 * Called from UITask GPIO EXTI handler — must be ISR-safe (no blocking).
 * Drops the command silently if the queue is full.
 */
void KeypadAdapter_PushCommand(KeypadAdapterCtx_t *ctx, UserCommand_t cmd);
