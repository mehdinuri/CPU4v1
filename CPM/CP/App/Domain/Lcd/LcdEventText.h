/* App/Domain/Lcd/LcdEventText.h
 *
 * Operator-facing LCD text lookup for log events and live safety reasons.
 */
#ifndef LCD_EVENT_TEXT_H
#define LCD_EVENT_TEXT_H

#include <stdint.h>

const char *LcdEventText_GetEventLong(uint8_t code, uint8_t lang);
const char *LcdEventText_GetSafetyReasonShort(uint8_t reasonCode, uint8_t lang);

#endif /* LCD_EVENT_TEXT_H */
