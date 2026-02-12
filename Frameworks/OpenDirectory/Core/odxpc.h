typedef void (^odxpc_handler_t)(CFPropertyListRef, uint64_t, bool);

void odxpc_send_message_with_reply(uint64_t reqtype, const uint8_t *session, const uint8_t *node, CFDataRef data, dispatch_queue_t replyq, odxpc_handler_t handler);
