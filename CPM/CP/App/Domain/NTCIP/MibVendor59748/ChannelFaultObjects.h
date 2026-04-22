/* App/Domain/NTCIP/MibVendor59748/ChannelFaultObjects.h
 *
 * Teknotel vendor-private 'channel' group under the CPU4 controller arc.
 * Rooted at 1.3.6.1.4.1.59748.4.2.1.8; mirrors NTCIP 1202 asc.channel(8)
 * for load-switch fault reporting published by the MP.
 */
#ifndef TEKNOTEL_VENDOR_CHANNEL_FAULT_OBJECTS_H
#define TEKNOTEL_VENDOR_CHANNEL_FAULT_OBJECTS_H

#include "Domain/NTCIP/Core/NtcipObjectDirectory.h"
#include "Domain/NTCIP/NtcipContext.h"

void TeknotelChannelFaultObjectsRegister(NtcipObjectDirectory_t *directory,
                                         NtcipContext_t *context);

#endif /* TEKNOTEL_VENDOR_CHANNEL_FAULT_OBJECTS_H */
