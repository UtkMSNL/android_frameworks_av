#include <camera/RpcICameraClientProxy.h>

namespace android {

RpcICameraClientProxy::RpcICameraClientProxy(int remoteServiceId)
    : mRemoteServiceId(remoteServiceId) {}
    
RpcICameraClientProxy::~RpcICameraClientProxy() {}

void RpcICameraClientProxy::notifyCallback(int32_t msgType, int32_t ext1, int32_t ext2)
{
    // FIXME: the ext2 will be a file descriptor when msgType == CAMERA_MSG_PREVIEW_FRAME && ext1 == CAMERA_FRAME_DATA_FD
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CCP_METH_NOTIFY_CALLBACK)
    request->putArg((char*) &msgType, sizeof(msgType));
    request->putArg((char*) &ext1, sizeof(ext1));
    request->putArg((char*) &ext2, sizeof(ext2));
    
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    delete response;
}

void RpcICameraClientProxy::dataCallback(int32_t msgType, const sp<IMemory>& data,
                                         camera_frame_metadata_t *metadata)
{
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CCP_METH_DATA_CALLBACK)
    request->putArg((char*) &msgType, sizeof(msgType));
    void* dataptr = data->pointer();
    size_t size = data->size();
    request->putArg((char*) &size, sizeof(size));
    request->putArg((char*) dataptr, size);
    if (metadata) {
        request->putArg((char*) &metadata->number_of_faces, sizeof(metadata->number_of_faces));
        request->putArg((char*) metadata->faces, sizeof(camera_face_t) * metadata->number_of_faces);
    } else {
        int32_t num = -1;
        request->putArg((char*) &num, sizeof(num));
    }
    
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    delete response;
}

void RpcICameraClientProxy::dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& data)
{
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CCP_METH_DATA_CALLBACK_TIMESTAMP)
    request->putArg((char*) &timestamp, sizeof(timestamp));
    request->putArg((char*) &msgType, sizeof(msgType));
    void* dataptr = data->pointer();
    size_t size = data->size();
    request->putArg((char*) &size, sizeof(size));
    request->putArg((char*) dataptr, size);
    
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    delete response;
}
    

} // namespace android
