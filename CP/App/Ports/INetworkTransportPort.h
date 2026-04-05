#pragma once

/*
 * App/Ports/INetworkTransportPort.h
 *
 * TCP/UDP send and receive abstraction. Used by the Domain to communicate
 * with the MCS (Management Control System) server. The concrete adapter
 * wraps LWIP sockets; the mock captures sent packets and injects responses.
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum
{
  TRANSPORT_OK            = 0,
  TRANSPORT_ERR_NOT_CONNECTED,
  TRANSPORT_ERR_TIMEOUT,
  TRANSPORT_ERR_SEND_FAILED,
  TRANSPORT_ERR_NO_DATA,
} TransportResult_t;

typedef struct INetworkTransportPort
{
  void *ctx;

  /* True if a TCP connection to the remote server is currently active. */
  bool (*isConnected)(void *ctx);

  /* Send [len] bytes. Non-blocking; returns TRANSPORT_ERR_NOT_CONNECTED
   * if the link is down (caller should retry). */
  TransportResult_t (*send)(void *ctx, const uint8_t *data, size_t len);

  /* Copy up to [maxLen] bytes of pending received data into [outBuf].
   * Returns TRANSPORT_ERR_NO_DATA if the receive queue is empty. */
  TransportResult_t (*recv)(void *ctx, uint8_t *outBuf, size_t maxLen,
                            size_t *outLen);
} INetworkTransportPort_t;

static inline bool Transport_IsConnected(INetworkTransportPort_t *p)
{
  return p->isConnected(p->ctx);
}

static inline TransportResult_t Transport_Send(INetworkTransportPort_t *p,
                                               const uint8_t *data,
                                               size_t len)
{
  return p->send(p->ctx, data, len);
}

static inline TransportResult_t Transport_Recv(INetworkTransportPort_t *p,
                                               uint8_t *buf,
                                               size_t maxLen,
                                               size_t *outLen)
{
  return p->recv(p->ctx, buf, maxLen, outLen);
}
