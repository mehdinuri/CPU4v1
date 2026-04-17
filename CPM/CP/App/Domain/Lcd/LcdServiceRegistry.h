/* App/Domain/Lcd/LcdServiceRegistry.h
 *
 * Consolidates all system ports required by the LCD subsystem.
 * This prevents telescoping parameter lists in page initialization.
 */
#ifndef LCD_SERVICE_REGISTRY_H
#define LCD_SERVICE_REGISTRY_H

#include "Ports/ISystemPort.h"
#include "Ports/ICommsStatusPort.h"
#include "Ports/IUserPort.h"
#include "Ports/ILogRepositoryPort.h"
#include "Ports/IIntersectionStatusPort.h"
#include "Ports/IRealtimeClockPort.h"
#include "Ports/IGpsPort.h"

typedef struct
{
  ISystemPort_t             *system;
  ICommsStatusPort_t        *comms;
  IUserPort_t               *user;
  ILogRepositoryPort_t      *logs;
  IIntersectionStatusPort_t *intersection;
  IRealtimeClockPort_t      *rtc;
  IGpsPort_t                *gps;
} LcdServiceRegistry_t;

#endif /* LCD_SERVICE_REGISTRY_H */
