#ifndef ANDROID_RPC_AUDIO_FLINGER_SERVER_H
#define ANDROID_RPC_AUDIO_FLINGER_SERVER_H

#include "AudioFlinger.h"
#include "RpcAudioCommon.h"
#include <rpc/share_rpc.h>

namespace android {

// ---------------------------------------------------------------------------
void registerRpcAudioFlingerServer();

}

#endif
