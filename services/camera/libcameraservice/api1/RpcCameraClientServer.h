#ifndef ANDROID_RPC_CAMERA_CLIENT_SERVER_H
#define ANDROID_RPC_CAMERA_CLIENT_SERVER_H

#include "CameraClient.h"
#include <camera/RpcCameraCommon.h>
#include <rpc/share_rpc.h>

namespace android {

// ---------------------------------------------------------------------------
void refreshPreviewWindow(const uint8_t* data, size_t size, int width, int height, int stride, int format, int usage);

int registerCameraClientForRpc(sp<ICamera> camera);

}

#endif
