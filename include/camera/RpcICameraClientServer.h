#ifndef RPC_ICAMERA_CLIENT_SERVER_H_
#define RPC_ICAMERA_CLIENT_SERVER_H_

#include <camera/ICameraClient.h>
#include <rpc/share_rpc.h>
#include <binder/IPCThreadState.h>
#include <camera/RpcCameraCommon.h>
#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>

namespace android {

int registerICameraClientForRpc(sp<ICameraClient> cameraClient);

void unregisterICameraClient(int serviceId);

} // namespace android

#endif
