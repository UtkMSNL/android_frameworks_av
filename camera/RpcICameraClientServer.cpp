#include <camera/RpcICameraClientServer.h>
#include <utils/String8.h>
#include <camera/Camera.h>

namespace android {

// ICameraClient_notifyCallback
RpcResponse* ICameraClient_notifyCallback(RpcRequest* request) 
{
    // FIXME: the ext2 will be a file descriptor when msgType == CAMERA_MSG_PREVIEW_FRAME && ext1 == CAMERA_FRAME_DATA_FD
    RPC_SERVER_REQUEST_COMMON()
    int32_t msgType;
    request->getArg((char*) &msgType, sizeof(msgType));
    int ext1;
    request->getArg((char*) &ext1, sizeof(ext1));
    int ext2;
    request->getArg((char*) &ext2, sizeof(ext2));
    sp<ICameraClient> cameraClient = *((sp<ICameraClient>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    cameraClient->notifyCallback(msgType, ext1, ext2);
    
    RpcResponse* response = new RpcResponse(false);
    return response;
}

RpcResponse* ICameraClient_dataCallback(RpcRequest* request) 
{
    RPC_SERVER_REQUEST_COMMON()
    int32_t msgType;
    request->getArg((char*) &msgType, sizeof(msgType));
    size_t size;
    request->getArg((char*) &size, sizeof(size));
    // FIXME: reuse the allocated memory buffer in the future
    sp<MemoryHeapBase> heap = new MemoryHeapBase(size);
    sp<MemoryBase> buffer = new MemoryBase(heap, 0, size);
    request->getArg((char*) heap->base(), size);
    int32_t metasize;
    request->getArg((char*) &metasize, sizeof(metasize));
    camera_frame_metadata_t *metadata = NULL;
    if (metasize != -1) {
        metadata = new camera_frame_metadata_t;
        metadata->number_of_faces = metasize;
        char *buf = new char[metasize];
        request->getArg(buf, metasize);
        metadata->faces = (camera_face_t *) buf;
    }
    
    sp<ICameraClient> cameraClient = *((sp<ICameraClient>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    cameraClient->dataCallback(msgType, buffer, metadata);
    
    if (metadata) {
        delete metadata->faces;
        delete metadata;
    }
    
    RpcResponse* response = new RpcResponse(false);
    return response;
}

RpcResponse* ICameraClient_dataCallbackTimestamp(RpcRequest* request) 
{
    RPC_SERVER_REQUEST_COMMON()
    nsecs_t timestamp;
    request->getArg((char*) &timestamp, sizeof(timestamp));
    int32_t msgType;
    request->putArg((char*) &msgType, sizeof(msgType));
    size_t size;
    request->getArg((char*) &size, sizeof(size));
    // FIXME: reuse the allocated memory buffer in the future
    sp<MemoryHeapBase> heap = new MemoryHeapBase(size);
    sp<MemoryBase> buffer = new MemoryBase(heap, 0, size);
    request->getArg((char*) heap->base(), size);
    
    sp<ICameraClient> cameraClient = *((sp<ICameraClient>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    cameraClient->dataCallbackTimestamp(timestamp, msgType, buffer);
    
    RpcResponse* response = new RpcResponse(false);
    return response;
}

int registerICameraClientForRpc(sp<ICameraClient> cameraClient)
{
    int cameraClientId = CameraRpcUtilInst.nextServiceObjId++;
    sp<ICameraClient>* cclientref = new sp<ICameraClient>(cameraClient);
    CameraRpcUtilInst.idToObjMap[cameraClientId] = cclientref;
    CameraRpcUtilInst.rpcclient->registerFunc(cameraClientId, CCP_METH_NOTIFY_CALLBACK, &ICameraClient_notifyCallback);
    CameraRpcUtilInst.rpcclient->registerFunc(cameraClientId, CCP_METH_DATA_CALLBACK, &ICameraClient_dataCallback);
    CameraRpcUtilInst.rpcclient->registerFunc(cameraClientId, CCP_METH_DATA_CALLBACK_TIMESTAMP, &ICameraClient_dataCallbackTimestamp);
    
    return cameraClientId;
}    

void unregisterICameraClient(int serviceId)
{
    CameraRpcUtilInst.idToObjMap.erase(serviceId);
}

} // namespace android
