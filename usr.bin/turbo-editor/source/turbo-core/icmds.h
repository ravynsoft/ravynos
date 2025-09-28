#ifndef TURBO_ICMDS_H
#define TURBO_ICMDS_H

#define Uses_TEvent
#include <tvision/tv.h>

namespace turbo {

// Any numbered commands may conflict with commands used by Turbo Vision or
// by the application linking against Turbo. To work around this, use commands
// identified by a unique address and specified via the 'infoPtr' field of a
// regular 'cmValid' broadcast event, which will usually not have any side
// effects on views which aren't handling the internal command on purpose.

using InternalCommandId = void *;

struct InternalCommands
{
    static const InternalCommandId cmGetEditor;
};

inline void *internalMessage(TView *receiver, InternalCommandId command)
{
    return message(receiver, evBroadcast, cmValid, command);
}

inline bool isInternalMessage(const MessageEvent &message, InternalCommandId command) noexcept
{
    return message.command == cmValid && message.infoPtr == command;
}

} // namespace turbo

#endif // TURBO_ICMDS_H
