/* App/Domain/NTCIP/MibVendor59748/UnitObjects.c
 *
 * Teknotel vendor 'unit' group. Mirrors the NTCIP 1202 asc.unit(3) slot at
 * 1.3.6.1.4.1.59748.4.2.1.3 and currently exposes:
 *
 *   unitFailureFlashPeriodDs      .4.2.1.3.1  -- R/W, deciseconds
 *   unitSnmpV3ActiveUsername      .4.2.1.3.2  -- R/W, active username
 *   unitSnmpV3NewAuthPassphrase   .4.2.1.3.3  -- staged, write/apply flow
 *   unitSnmpV3NewPrivPassphrase   .4.2.1.3.4  -- staged, write/apply flow
 *   unitSnmpV3Apply               .4.2.1.3.5  -- commit staged credentials
 */
#include "UnitObjects.h"

#include <string.h>

#include "Domain/NTCIP/Core/NtcipDbTransactionService.h"

enum
{
  UNIT_OBJECT_TAG_FAILURE_FLASH_PERIOD_DS = 1,
  UNIT_OBJECT_TAG_SNMPV3_ACTIVE_USERNAME = 2,
  UNIT_OBJECT_TAG_SNMPV3_NEW_AUTH_PASSPHRASE = 3,
  UNIT_OBJECT_TAG_SNMPV3_NEW_PRIV_PASSPHRASE = 4,
  UNIT_OBJECT_TAG_SNMPV3_APPLY = 5
};

static const uint32_t kUnitFailureFlashPeriodDsOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 1U
};
static const uint32_t kUnitSnmpV3ActiveUsernameOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 2U
};
static const uint32_t kUnitSnmpV3NewAuthPassphraseOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 3U
};
static const uint32_t kUnitSnmpV3NewPrivPassphraseOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 4U
};
static const uint32_t kUnitSnmpV3ApplyOid[] = {
  1U, 3U, 6U, 1U, 4U, 1U, 59748U, 4U, 2U, 1U, 3U, 5U
};

static uint8_t ValidFailureFlashPeriodDs(uint32_t failureFlashPeriodDs)
{
  return (uint8_t) ((failureFlashPeriodDs
                     == INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_500MS_DS)
                    || (failureFlashPeriodDs
                        == INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_1000MS_DS)
                    || (failureFlashPeriodDs
                        == INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_2000MS_DS)
                    || (failureFlashPeriodDs
                        == INTERSECTION_UNIT_FAILURE_FLASH_PERIOD_4000MS_DS));
}

static uint8_t RequestUsesCommunitySecurity(
  const NtcipRequestContext_t *requestContext)
{
  if (requestContext == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((requestContext->authModel
                     == (uint8_t) NTCIP_AUTH_MODEL_SNMP_V1)
                    || (requestContext->authModel
                        == (uint8_t) NTCIP_AUTH_MODEL_SNMP_V2C));
}

static uint8_t StrictReleasePolicyEnabled(const NtcipContext_t *context)
{
  if ((context == NULL) || (context->snmpSecurityPort == NULL))
  {
    return 0U;
  }

  return SnmpSecurityPortGetStrictReleasePolicy(context->snmpSecurityPort);
}

static uint8_t CopyOctetStringToText(char *dst,
                                     uint16_t dstSize,
                                     const NtcipValue_t *value)
{
  if ((dst == NULL) || (value == NULL)
      || (value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
      || (value->data.octetString.length >= dstSize))
  {
    return 0U;
  }

  (void) memset(dst, 0, dstSize);
  if (value->data.octetString.length > 0U)
  {
    (void) memcpy(&dst[0],
                  &value->data.octetString.bytes[0],
                  value->data.octetString.length);
  }

  return 1U;
}

static uint8_t SnmpSecurityChangeStaged(const NtcipContext_t *context)
{
  if (context == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((context->stagedSnmpV3Username[0] != '\0')
                    || (context->stagedSnmpV3AuthPassphrase[0] != '\0')
                    || (context->stagedSnmpV3PrivPassphrase[0] != '\0'));
}

static void ClearSnmpSecurityStaging(NtcipContext_t *context)
{
  if (context == NULL)
  {
    return;
  }

  (void) memset(&context->stagedSnmpV3Username[0],
                0,
                sizeof(context->stagedSnmpV3Username));
  (void) memset(&context->stagedSnmpV3AuthPassphrase[0],
                0,
                sizeof(context->stagedSnmpV3AuthPassphrase));
  (void) memset(&context->stagedSnmpV3PrivPassphrase[0],
                0,
                sizeof(context->stagedSnmpV3PrivPassphrase));
}

static uint8_t GetSnmpV3Username(const NtcipContext_t *context,
                                 char *dst,
                                 uint16_t dstSize)
{
  if ((context == NULL) || (dst == NULL) || (dstSize == 0U))
  {
    return 0U;
  }

  (void) memset(dst, 0, dstSize);
  if (context->stagedSnmpV3Username[0] != '\0')
  {
    (void) strncpy(dst, &context->stagedSnmpV3Username[0], dstSize - 1U);
    dst[dstSize - 1U] = '\0';
    return 1U;
  }

  if (context->snmpSecurityPort == NULL)
  {
    return 0U;
  }

  return SnmpSecurityPortGetSnmpV3Username(context->snmpSecurityPort,
                                           dst,
                                           dstSize);
}

static NtcipError_t ValidateDatabaseWrite(const NtcipContext_t *context,
                                          const NtcipRequestContext_t *
                                          requestContext)
{
  if ((context == NULL) || (context->dbTransactionService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  return NtcipDbTransactionServiceValidateDatabaseWrite(
    context->dbTransactionService,
    requestContext);
}

static NtcipError_t ValidateSnmpSecurityWrite(const NtcipContext_t *context,
                                              const NtcipRequestContext_t *
                                              requestContext)
{
  if ((context == NULL) || (context->snmpSecurityPort == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if ((StrictReleasePolicyEnabled(context) != 0U)
      && (RequestUsesCommunitySecurity(requestContext) != 0U))
  {
    return NTCIP_ERROR_NO_ACCESS;
  }

  return ValidateDatabaseWrite(context, requestContext);
}

static NtcipError_t GetUnitObject(void *groupContext,
                                  const NtcipObjectDescriptor_t *descriptor,
                                  const uint32_t *indexes,
                                  uint8_t indexCount,
                                  const NtcipRequestContext_t *requestContext,
                                  NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  IntersectionUnitConfig_t unitConfig;

  (void) indexes;
  (void) indexCount;
  (void) requestContext;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL)
      || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (ConfigurationServiceGetActiveUnitConfig(context->configurationService,
                                              &unitConfig) == 0U)
  {
    return NTCIP_ERROR_GEN_ERROR;
  }

  switch (descriptor->tag)
  {
      case UNIT_OBJECT_TAG_FAILURE_FLASH_PERIOD_DS:
      {
        NtcipValueSetUnsigned32(value, unitConfig.failureFlashPeriodDs);

        return NTCIP_ERROR_OK;
      }

      case UNIT_OBJECT_TAG_SNMPV3_ACTIVE_USERNAME:
      {
        char username[NTCIP_SNMPV3_USERNAME_MAX_LENGTH + 1U];

        if (GetSnmpV3Username(context, &username[0], sizeof(username)) == 0U)
        {
          return NTCIP_ERROR_GEN_ERROR;
        }

        return NtcipValueSetCString(value, &username[0]);
      }

      case UNIT_OBJECT_TAG_SNMPV3_NEW_AUTH_PASSPHRASE:
      case UNIT_OBJECT_TAG_SNMPV3_NEW_PRIV_PASSPHRASE:
      case UNIT_OBJECT_TAG_SNMPV3_APPLY:
      {
        return NTCIP_ERROR_NO_ACCESS;
      }

      default:
      {
        return NTCIP_ERROR_NOT_FOUND;
      }
  }
}

static NtcipError_t SetTestUnitObject(void *groupContext,
                                      const NtcipObjectDescriptor_t *descriptor,
                                      const uint32_t *indexes,
                                      uint8_t indexCount,
                                      const NtcipRequestContext_t *
                                      requestContext,
                                      const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;

  (void) indexes;
  (void) indexCount;

  if ((context == NULL) || (descriptor == NULL) || (value == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  if (descriptor->tag != UNIT_OBJECT_TAG_FAILURE_FLASH_PERIOD_DS)
  {
    error = ValidateSnmpSecurityWrite(context, requestContext);
    if (error != NTCIP_ERROR_OK)
    {
      return error;
    }

    switch (descriptor->tag)
    {
      case UNIT_OBJECT_TAG_SNMPV3_ACTIVE_USERNAME:
        if ((value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
            || (value->data.octetString.length == 0U)
            || (value->data.octetString.length
                > NTCIP_SNMPV3_USERNAME_MAX_LENGTH))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return NTCIP_ERROR_OK;

      case UNIT_OBJECT_TAG_SNMPV3_NEW_AUTH_PASSPHRASE:
      case UNIT_OBJECT_TAG_SNMPV3_NEW_PRIV_PASSPHRASE:
        if ((value->type != NTCIP_VALUE_TYPE_OCTET_STRING)
            || (value->data.octetString.length < 8U)
            || (value->data.octetString.length
                > NTCIP_SNMPV3_PASSPHRASE_MAX_LENGTH))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return NTCIP_ERROR_OK;

      case UNIT_OBJECT_TAG_SNMPV3_APPLY:
        if ((value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
            || (value->data.unsigned32 != 1U)
            || (SnmpSecurityChangeStaged(context) == 0U))
        {
          return NTCIP_ERROR_BAD_VALUE;
        }

        return NTCIP_ERROR_OK;

      default:
        return NTCIP_ERROR_READ_ONLY;
    }
  }

  if (value->type != NTCIP_VALUE_TYPE_UNSIGNED32)
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  error = ValidateDatabaseWrite(context, requestContext);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  return (ValidFailureFlashPeriodDs(value->data.unsigned32) != 0U)
         ? NTCIP_ERROR_OK
         : NTCIP_ERROR_BAD_VALUE;
}

static NtcipError_t SetValueUnitObject(void *groupContext,
                                       const NtcipObjectDescriptor_t *descriptor,
                                       const uint32_t *indexes,
                                       uint8_t indexCount,
                                       const NtcipRequestContext_t *
                                       requestContext,
                                       const NtcipValue_t *value)
{
  NtcipContext_t *context = (NtcipContext_t *) groupContext;
  NtcipError_t error;

  error = SetTestUnitObject(groupContext,
                            descriptor,
                            indexes,
                            indexCount,
                            requestContext,
                            value);
  if (error != NTCIP_ERROR_OK)
  {
    return error;
  }

  if ((context == NULL) || (context->configurationService == NULL))
  {
    return NTCIP_ERROR_BAD_VALUE;
  }

  switch (descriptor->tag)
  {
    case UNIT_OBJECT_TAG_FAILURE_FLASH_PERIOD_DS:
      return (ConfigurationServiceSetUnitFailureFlashPeriodDs(
                context->configurationService,
                (uint8_t) value->data.unsigned32) != 0U)
             ? NTCIP_ERROR_OK
             : NTCIP_ERROR_GEN_ERROR;

    case UNIT_OBJECT_TAG_SNMPV3_ACTIVE_USERNAME:
      if (CopyOctetStringToText(&context->stagedSnmpV3Username[0],
                                sizeof(context->stagedSnmpV3Username),
                                value) == 0U)
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      return NTCIP_ERROR_OK;

    case UNIT_OBJECT_TAG_SNMPV3_NEW_AUTH_PASSPHRASE:
      if (CopyOctetStringToText(&context->stagedSnmpV3AuthPassphrase[0],
                                sizeof(context->stagedSnmpV3AuthPassphrase),
                                value) == 0U)
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      return NTCIP_ERROR_OK;

    case UNIT_OBJECT_TAG_SNMPV3_NEW_PRIV_PASSPHRASE:
      if (CopyOctetStringToText(&context->stagedSnmpV3PrivPassphrase[0],
                                sizeof(context->stagedSnmpV3PrivPassphrase),
                                value) == 0U)
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      return NTCIP_ERROR_OK;

    case UNIT_OBJECT_TAG_SNMPV3_APPLY:
    {
      const char *username =
        (context->stagedSnmpV3Username[0] != '\0')
        ? &context->stagedSnmpV3Username[0]
        : NULL;
      const char *authPassphrase =
        (context->stagedSnmpV3AuthPassphrase[0] != '\0')
        ? &context->stagedSnmpV3AuthPassphrase[0]
        : (context->stagedSnmpV3PrivPassphrase[0] != '\0')
          ? &context->stagedSnmpV3PrivPassphrase[0]
          : NULL;
      const char *privPassphrase =
        (context->stagedSnmpV3PrivPassphrase[0] != '\0')
        ? &context->stagedSnmpV3PrivPassphrase[0]
        : (context->stagedSnmpV3AuthPassphrase[0] != '\0')
          ? &context->stagedSnmpV3AuthPassphrase[0]
          : NULL;
      uint8_t ok;

      if ((authPassphrase != NULL) || (privPassphrase != NULL))
      {
        ok = SnmpSecurityPortSetSnmpV3Credentials(context->snmpSecurityPort,
                                                  username,
                                                  authPassphrase,
                                                  privPassphrase);
      }
      else if (username != NULL)
      {
        ok = SnmpSecurityPortSetSnmpV3Username(context->snmpSecurityPort,
                                               username);
      }
      else
      {
        return NTCIP_ERROR_BAD_VALUE;
      }

      if (ok == 0U)
      {
        return NTCIP_ERROR_COMMIT_FAILED;
      }

      ClearSnmpSecurityStaging(context);
      return NTCIP_ERROR_OK;
    }

    default:
      return NTCIP_ERROR_READ_ONLY;
  }
}

static const NtcipObjectDescriptor_t kUnitObjects[] = {
  { kUnitFailureFlashPeriodDsOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_FAILURE_FLASH_PERIOD_DS, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject },
  { kUnitSnmpV3ActiveUsernameOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    UNIT_OBJECT_TAG_SNMPV3_ACTIVE_USERNAME, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject },
  { kUnitSnmpV3NewAuthPassphraseOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    UNIT_OBJECT_TAG_SNMPV3_NEW_AUTH_PASSPHRASE, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject },
  { kUnitSnmpV3NewPrivPassphraseOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_OCTET_STRING,
    UNIT_OBJECT_TAG_SNMPV3_NEW_PRIV_PASSPHRASE, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject },
  { kUnitSnmpV3ApplyOid, 12U, NTCIP_OBJECT_KIND_SCALAR, 0U,
    NTCIP_ACCESS_READ_WRITE, NTCIP_VALUE_TYPE_UNSIGNED32,
    UNIT_OBJECT_TAG_SNMPV3_APPLY, GetUnitObject,
    SetTestUnitObject, SetValueUnitObject }
};

void TeknotelUnitObjectsRegister(NtcipObjectDirectory_t *directory,
                                 NtcipContext_t *context)
{
  (void) NtcipObjectDirectoryRegisterGroup(
    directory,
    "59748.unit",
    kUnitObjects,
    (uint16_t) (sizeof(kUnitObjects) / sizeof(kUnitObjects[0])),
    context);
}
