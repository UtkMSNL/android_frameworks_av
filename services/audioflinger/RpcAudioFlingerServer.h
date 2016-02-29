#ifndef ANDROID_RPC_AUDIO_FLINGER_SERVER_H
#define ANDROID_RPC_AUDIO_FLINGER_SERVER_H

#include "AudioFlinger.h"
#include <media/RpcAudioCommon.h>
#include <rpc/sbuffer_sync.h>

namespace android {

// ---------------------------------------------------------------------------
void registerRpcAudioFlingerServer();

}

#endif
