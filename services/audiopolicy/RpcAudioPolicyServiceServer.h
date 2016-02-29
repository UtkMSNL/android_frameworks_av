#ifndef ANDROID_RPCAUDIOPOLICYSERVICE_SERVER_H
#define ANDROID_RPCAUDIOPOLICYSERVICE_SERVER_H

#include "AudioPolicyService.h"
#include <media/RpcAudioCommon.h>
#include <rpc/share_rpc.h>

namespace android {

// ---------------------------------------------------------------------------
void registerRpcAudioPolicyServiceServer();

}

#endif
