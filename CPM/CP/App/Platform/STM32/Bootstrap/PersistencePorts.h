/* App/Platform/STM32/Core/PersistencePorts.h
 *
 * Global persistence ports created by MainApplication_Init().
 * Application and task code uses these semantic persistence handles and
 * never calls flash or EEPROM routines directly.
 */
#ifndef PERSISTENCE_PORTS_H
#define PERSISTENCE_PORTS_H

#include "Ports/IConfigRepositoryPort.h"
#include "Ports/IPersistencePort.h"
#include "Ports/ILogRepositoryPort.h"

extern IPersistencePort_t g_persistencePort;
extern IConfigRepositoryPort_t g_configRepositoryPort;
extern ILogRepositoryPort_t g_logRepositoryPort;

#endif /* PERSISTENCE_PORTS_H */
