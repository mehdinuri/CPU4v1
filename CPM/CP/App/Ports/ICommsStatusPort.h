/* App/Ports/ICommsStatusPort.h
 */
#ifndef ICOMMS_STATUS_PORT_H
#define ICOMMS_STATUS_PORT_H

#include <stdint.h>

typedef struct
{
  void *ctx;

  uint8_t (*GetModemType)(void *ctx);
  uint8_t (*GetModemAlive)(void *ctx);
  uint8_t (*GetAsynchConnected)(void *ctx);
  const char *(*GetImei)(void *ctx);
  const char *(*GetUsrMac)(void *ctx);
  const char *(*GetEthMac)(void *ctx);
  uint8_t (*GetSignalQuality)(void *ctx);
  uint8_t (*GetJobCurrent)(void *ctx, char *buf, uint8_t shift);
  const char *(*GetLocalIp)(void *ctx);
  const char *(*GetRemoteIp)(void *ctx);
} ICommsStatusPort_t;

static inline uint8_t CommsStatusGetModemType(ICommsStatusPort_t *p)
{
  return p->GetModemType(p->ctx);
}

static inline uint8_t CommsStatusGetModemAlive(ICommsStatusPort_t *p)
{
  return p->GetModemAlive(p->ctx);
}

static inline uint8_t CommsStatusGetAsynchConnected(ICommsStatusPort_t *p)
{
  return p->GetAsynchConnected(p->ctx);
}

static inline const char *CommsStatusGetImei(ICommsStatusPort_t *p)
{
  return p->GetImei(p->ctx);
}

static inline const char *CommsStatusGetUsrMac(ICommsStatusPort_t *p)
{
  return p->GetUsrMac(p->ctx);
}

static inline const char *CommsStatusGetEthMac(ICommsStatusPort_t *p)
{
  return p->GetEthMac(p->ctx);
}

static inline uint8_t CommsStatusGetSignalQuality(ICommsStatusPort_t *p)
{
  return p->GetSignalQuality(p->ctx);
}

static inline uint8_t CommsStatusGetJobCurrent(ICommsStatusPort_t *p,
                                               char *buf,
                                               uint8_t shift)
{
  return p->GetJobCurrent(p->ctx, buf, shift);
}

static inline const char *CommsStatusGetLocalIp(ICommsStatusPort_t *p)
{
  return p->GetLocalIp(p->ctx);
}

static inline const char *CommsStatusGetRemoteIp(ICommsStatusPort_t *p)
{
  return p->GetRemoteIp(p->ctx);
}

#endif /* ICOMMS_STATUS_PORT_H */
