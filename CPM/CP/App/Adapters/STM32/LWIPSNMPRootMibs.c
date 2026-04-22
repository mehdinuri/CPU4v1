/* App/Adapters/STM32/LWIPSNMPRootMibs.c
 *
 * Small lwIP root nodes for the canonical 1201/1202/private object trees.
 */
#include "LWIPSNMPRootMibs.h"

#include "LWIPSNMPBridge.h"

static snmp_err_t DelegatingLeafGetInstance(const u32_t *rootOid,
                                            u8_t rootOidLen,
                                            struct snmp_node_instance *instance)
{
  struct snmp_obj_id exactOid;

  if ((rootOid == NULL) || (instance == NULL))
  {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

  snmp_oid_assign(&exactOid, rootOid, rootOidLen);
  snmp_oid_append(&exactOid,
                  instance->instance_oid.id,
                  instance->instance_oid.len);

  if (LWIPSNMPBridgeGetManagedState(exactOid.id, exactOid.len)
      != LWIP_SNMP_MANAGED_STATE_EXACT)
  {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

  return SNMP_ERR_NOERROR;
}

static snmp_err_t DelegatingLeafGetNextInstance(const u32_t *rootOid,
                                                u8_t rootOidLen,
                                                struct snmp_node_instance *
                                                instance)
{
  struct snmp_obj_id startOid;
  struct snmp_obj_id nextOid;

  if ((rootOid == NULL) || (instance == NULL))
  {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

  snmp_oid_assign(&startOid, rootOid, rootOidLen);
  snmp_oid_append(&startOid,
                  instance->instance_oid.id,
                  instance->instance_oid.len);

  if (LWIPSNMPBridgeFindNextManagedOid(startOid.id,
                                       startOid.len,
                                       rootOid,
                                       rootOidLen,
                                       &nextOid) == 0U)
  {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

  if (nextOid.len < rootOidLen)
  {
    return SNMP_ERR_NOSUCHINSTANCE;
  }

  snmp_oid_assign(&instance->instance_oid,
                  nextOid.id + rootOidLen,
                  (u8_t) (nextOid.len - rootOidLen));

  return SNMP_ERR_NOERROR;
}

#define LWIP_SNMP_CREATE_DELEGATING_LEAF(oid)                                 \
        {                                                                           \
          { SNMP_NODE_SCALAR, (oid) },                                              \
          DelegatingLeafGetInstance,                                                \
          DelegatingLeafGetNextInstance                                             \
        }

static const struct snmp_leaf_node kNtcip1201Subtree1 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(1U);
static const struct snmp_leaf_node kNtcip1201Subtree2 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(2U);
static const struct snmp_leaf_node kNtcip1201Subtree3 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(3U);
static const struct snmp_leaf_node kNtcip1201Subtree4 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(4U);
static const struct snmp_leaf_node kNtcip1201Subtree5 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(5U);
static const struct snmp_leaf_node kNtcip1201Subtree7 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(7U);

static const struct snmp_node *const kNtcip1201Nodes[] =
{
  &kNtcip1201Subtree1.node,
  &kNtcip1201Subtree2.node,
  &kNtcip1201Subtree3.node,
  &kNtcip1201Subtree4.node,
  &kNtcip1201Subtree5.node,
  &kNtcip1201Subtree7.node
};
static const struct snmp_tree_node kNtcip1201Root =
  SNMP_CREATE_TREE_NODE(0U, kNtcip1201Nodes);
static const u32_t kNtcip1201BaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 6U
};
const struct snmp_mib ntcip1201mib =
  SNMP_MIB_CREATE(kNtcip1201BaseOid, &kNtcip1201Root.node);

static const struct snmp_leaf_node kNtcip1202Subtree1 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(1U);
static const struct snmp_leaf_node kNtcip1202Subtree2 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(2U);
static const struct snmp_leaf_node kNtcip1202Subtree3 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(3U);
static const struct snmp_leaf_node kNtcip1202Subtree4 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(4U);
static const struct snmp_leaf_node kNtcip1202Subtree5 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(5U);
static const struct snmp_leaf_node kNtcip1202Subtree6 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(6U);
static const struct snmp_leaf_node kNtcip1202Subtree7 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(7U);
static const struct snmp_leaf_node kNtcip1202Subtree8 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(8U);
static const struct snmp_leaf_node kNtcip1202Subtree9 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(9U);
static const struct snmp_leaf_node kNtcip1202Subtree12 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(12U);
static const struct snmp_leaf_node kNtcip1202Subtree13 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(13U);

static const struct snmp_node *const kNtcip1202Nodes[] =
{
  &kNtcip1202Subtree1.node,
  &kNtcip1202Subtree2.node,
  &kNtcip1202Subtree3.node,
  &kNtcip1202Subtree4.node,
  &kNtcip1202Subtree5.node,
  &kNtcip1202Subtree6.node,
  &kNtcip1202Subtree7.node,
  &kNtcip1202Subtree8.node,
  &kNtcip1202Subtree9.node,
  &kNtcip1202Subtree12.node,
  &kNtcip1202Subtree13.node
};
static const struct snmp_tree_node kNtcip1202Root =
  SNMP_CREATE_TREE_NODE(0U, kNtcip1202Nodes);
static const u32_t kNtcip1202BaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 2U, 1U
};
const struct snmp_mib ntcip1202mib =
  SNMP_MIB_CREATE(kNtcip1202BaseOid, &kNtcip1202Root.node);

static const struct snmp_leaf_node kNtcip1103ApplicationSubtree4 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(4U);
static const struct snmp_leaf_node kNtcip1103ApplicationSubtree6 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(6U);
static const struct snmp_leaf_node kNtcip1103ApplicationSubtree7 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(7U);
static const struct snmp_leaf_node kNtcip1103ApplicationSubtree8 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(8U);

static const struct snmp_node *const kNtcip1103ApplicationNodes[] =
{
  &kNtcip1103ApplicationSubtree4.node,
  &kNtcip1103ApplicationSubtree6.node,
  &kNtcip1103ApplicationSubtree7.node,
  &kNtcip1103ApplicationSubtree8.node
};
static const struct snmp_tree_node kNtcip1103ApplicationRoot =
  SNMP_CREATE_TREE_NODE(0U, kNtcip1103ApplicationNodes);
static const u32_t kNtcip1103ApplicationBaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 1U, 7U
};
const struct snmp_mib ntcip1103applicationmib =
  SNMP_MIB_CREATE(kNtcip1103ApplicationBaseOid, &kNtcip1103ApplicationRoot.node);

static const struct snmp_leaf_node kNtcip1103TrapSubtree1 =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(1U);

static const struct snmp_node *const kNtcip1103TrapNodes[] =
{
  &kNtcip1103TrapSubtree1.node
};
static const struct snmp_tree_node kNtcip1103TrapRoot =
  SNMP_CREATE_TREE_NODE(0U, kNtcip1103TrapNodes);
static const u32_t kNtcip1103TrapBaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 1206U, 4U, 1U, 4U
};
const struct snmp_mib ntcip1103trapmib =
  SNMP_MIB_CREATE(kNtcip1103TrapBaseOid, &kNtcip1103TrapRoot.node);

/* TEKNOTEL-CPU4-MIB: .59748.4.2.1 (cpu4). Subtree numbers mirror NTCIP
 * 1202 groupings -- unit(3), channel(8) -- with vendor extensions at
 * 20 (cpMpLink) and 21 (driverModule) placed outside the NTCIP range. */
static const struct snmp_leaf_node kTeknotelUnitSubtree =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(3U);
static const struct snmp_leaf_node kTeknotelChannelSubtree =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(8U);
static const struct snmp_leaf_node kTeknotelCpMpLinkSubtree =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(20U);
static const struct snmp_leaf_node kTeknotelDriverModuleSubtree =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(21U);
static const struct snmp_leaf_node kTeknotelEventSourceSubtree =
  LWIP_SNMP_CREATE_DELEGATING_LEAF(22U);

static const struct snmp_node *const kTeknotelNodes[] =
{
  &kTeknotelUnitSubtree.node,
  &kTeknotelChannelSubtree.node,
  &kTeknotelCpMpLinkSubtree.node,
  &kTeknotelDriverModuleSubtree.node,
  &kTeknotelEventSourceSubtree.node
};
static const struct snmp_tree_node kTeknotelRoot =
  SNMP_CREATE_TREE_NODE(0U, kTeknotelNodes);
static const u32_t kTeknotelBaseOid[] =
{
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U
};
const struct snmp_mib teknotelmib =
  SNMP_MIB_CREATE(kTeknotelBaseOid, &kTeknotelRoot.node);
