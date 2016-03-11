#ifndef RPC_ICAMERA_CLIENT_PROXY_H_
#define RPC_ICAMERA_CLIENT_PROXY_H_

#include <camera/ICameraClient.h>
#include <rpc/share_rpc.h>
#include <binder/IPCThreadState.h>
#include <camera/RpcCameraCommon.h>

namespace android {

class RpcICameraClientProxy : public BnCameraClient
{
public:
    RpcICameraClientProxy(int remoteServiceId);
    virtual ~RpcICameraClientProxy();
    virtual void notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2);
    virtual void dataCallback(int32_t msgType, const sp<IMemory>& data,
                                         camera_frame_metadata_t *metadata);
    virtual void dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& data);
    
    int mRemoteServiceId;
};

} // namespace android

#endif
