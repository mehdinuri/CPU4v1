/* App/Domain/Services/MmiProtocol.c */
#include "MmiProtocol.h"

uint8_t MmiProtocolV2CanIdToMessageClass(uint16_t canId,
                                         MmiProtocolMessageClass_t *messageClass)
{
  if (messageClass == NULL)
  {
    return 0U;
  }

  switch (canId)
  {
    case MMI_PROTOCOL_V2_CAN_ID_ACK:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_ACK;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_EVENT_SEG:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_EVENT;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_PUBLISH_SEG:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_PUBLISH;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_SUBSCRIBE_SEG:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_RESPONSE_SEG:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_RESPONSE;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_COMMAND_SEG:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_HELLO_RSP:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_RESPONSE;
      return 1U;
    }

    case MMI_PROTOCOL_V2_CAN_ID_HELLO_REQ:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_REQUEST;
      return 1U;
    }

    default:
    {
      *messageClass = MMI_PROTOCOL_V2_MESSAGE_CLASS_NONE;
      return 0U;
    }
  }
}

uint8_t MmiProtocolV2MessageClassToCanId(MmiProtocolMessageClass_t messageClass,
                                         uint16_t *canId)
{
  if (canId == NULL)
  {
    return 0U;
  }

  switch (messageClass)
  {
    case MMI_PROTOCOL_V2_MESSAGE_CLASS_ACK:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_ACK;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_EVENT:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_EVENT_SEG;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_PUBLISH:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_PUBLISH_SEG;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_SUBSCRIBE:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_SUBSCRIBE_SEG;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_RESPONSE:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_RESPONSE_SEG;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_COMMAND:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_COMMAND_SEG;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_RESPONSE:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_HELLO_RSP;
      return 1U;
    }

    case MMI_PROTOCOL_V2_MESSAGE_CLASS_HELLO_REQUEST:
    {
      *canId = MMI_PROTOCOL_V2_CAN_ID_HELLO_REQ;
      return 1U;
    }

    default:
    {
      return 0U;
    }
  }
}

uint8_t MmiProtocolV2SegmentIsFirst(const MmiProtocolSegmentV2_t *segment)
{
  if (segment == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((segment->flags & MMI_PROTOCOL_V2_SEGMENT_FLAG_FIRST) != 0U);
}

uint8_t MmiProtocolV2SegmentIsLast(const MmiProtocolSegmentV2_t *segment)
{
  if (segment == NULL)
  {
    return 0U;
  }

  return (uint8_t) ((segment->flags & MMI_PROTOCOL_V2_SEGMENT_FLAG_LAST) != 0U);
}

uint16_t MmiProtocolV2ObjectPrefixLength(void)
{
  return (uint16_t) sizeof(MmiProtocolObjectPrefixV2_t);
}
