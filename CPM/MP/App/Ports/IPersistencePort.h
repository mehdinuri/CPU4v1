/* App/Ports/IPersistencePort.h
 *
 * Port interface for byte-addressable persistent storage (EEPROM /
 * NAND). Domain modules use this for configuration caching so that a
 * cold boot can validate against the last known good config received
 * from CP before the live stream comes back up.
 */
#ifndef I_PERSISTENCE_PORT_H
#define I_PERSISTENCE_PORT_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*Read)(void *ctx,
                  uint32_t address,
                  uint8_t *buffer,
                  uint32_t length);
  uint8_t (*Write)(void *ctx,
                   uint32_t address,
                   const uint8_t *buffer,
                   uint32_t length);
  uint8_t (*Size)(void *ctx, uint32_t *size);
} IPersistencePort_t;

static inline uint8_t PersistenceRead(IPersistencePort_t *port,
                                      uint32_t address,
                                      uint8_t *buffer,
                                      uint32_t length)
{
  if ((port == NULL) || (port->Read == NULL))
  {
    return 0U;
  }

  return port->Read(port->ctx, address, buffer, length);
}

static inline uint8_t PersistenceWrite(IPersistencePort_t *port,
                                       uint32_t address,
                                       const uint8_t *buffer,
                                       uint32_t length)
{
  if ((port == NULL) || (port->Write == NULL))
  {
    return 0U;
  }

  return port->Write(port->ctx, address, buffer, length);
}

static inline uint8_t PersistenceSize(const IPersistencePort_t *port,
                                      uint32_t *size)
{
  if ((port == NULL) || (port->Size == NULL))
  {
    return 0U;
  }

  return port->Size(port->ctx, size);
}

#endif /* I_PERSISTENCE_PORT_H */
