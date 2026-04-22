/* App/Adapters/STM32/LWIPSNMPRootMibs.h
 *
 * Small handwritten lwIP MIB roots that delegate 1201/1202/private OID
 * walking into the canonical object-directory bridge.
 */
#ifndef LWIP_SNMP_ROOT_MIBS_H
#define LWIP_SNMP_ROOT_MIBS_H

#include "lwip/apps/snmp_core.h"

extern const struct snmp_mib ntcip1201mib;
extern const struct snmp_mib ntcip1202mib;
extern const struct snmp_mib ntcip1103applicationmib;
extern const struct snmp_mib ntcip1103trapmib;
extern const struct snmp_mib teknotelmib;

#endif /* LWIP_SNMP_ROOT_MIBS_H */
