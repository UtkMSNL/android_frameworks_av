
#include "RpcCameraClientServer.h"
#include <camera/RpcICameraClientProxy.h>
#include <camera/ICamera.h>
#include <pthread.h>
#include <queue>

namespace android {
    
// ----------------------------------------------------------------------------
//      RPC Camera client in the server
// ----------------------------------------------------------------------------

// CameraClient_disconnect
RPC_GENERATE_SERVER_METHOD00(CC_, ICamera, disconnect)

// CameraClient_connect
RpcResponse* CameraClient_connect(RpcRequest* request)
{
    RPC_SERVER_REQUEST_COMMON()
    int remoteServiceId;
    request->getArg((char*) &remoteServiceId, sizeof(remoteServiceId));
    sp<ICameraClient> client(new RpcICameraClientProxy(remoteServiceId));
    
    sp<ICamera> camera = *((sp<ICamera>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = camera->connect(client);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// CameraClient_lock
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, lock, status_t)

// CameraClient_unlock
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, unlock, status_t)

// CameraClient_setPreviewTarget
RpcResponse* CameraClient_setPreviewTarget(RpcRequest* request)
{
    RPC_SERVER_REQUEST_COMMON()
    // use a fake pointer address so that the camera client can know this is a remote one
    IGraphicBufferProducer* fakeProducer = (IGraphicBufferProducer*) -1;
    sp<IGraphicBufferProducer> bufferProducer(fakeProducer);
    
    sp<ICamera> camera = *((sp<ICamera>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = camera->setPreviewTarget(bufferProducer);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// CameraClient_setPreviewCallbackFlag
RPC_GENERATE_SERVER_METHOD10(CC_, ICamera, setPreviewCallbackFlag, int)

// CameraClient_startPreview
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, startPreview, status_t)

// CameraClient_stopPreview
RPC_GENERATE_SERVER_METHOD00(CC_, ICamera, stopPreview)

// CameraClient_previewEnabled
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, previewEnabled, bool)

// CameraClient_storeMetaDataInBuffers
RPC_GENERATE_SERVER_METHOD11(CC_, ICamera, storeMetaDataInBuffers, bool, status_t)

// CameraClient_startRecording
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, startRecording, status_t)

// CameraClient_stopRecording
RPC_GENERATE_SERVER_METHOD00(CC_, ICamera, stopRecording)

// CameraClient_recordingEnabled
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, recordingEnabled, bool)

// CameraClient_autoFocus
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, autoFocus, status_t)

// CameraClient_cancelAutoFocus
RPC_GENERATE_SERVER_METHOD01(CC_, ICamera, cancelAutoFocus, status_t)

// CameraClient_takePicture
RPC_GENERATE_SERVER_METHOD11(CC_, ICamera, takePicture, int, status_t)

// CameraClient_setParameters
RpcResponse* CameraClient_setParameters(RpcRequest* request)
{
    RPC_SERVER_REQUEST_COMMON()
    size_t len;
    request->getArg((char*) &len, sizeof(len));
    char paramsBuf[len];
    request->getArg(paramsBuf, len);
    String8 params;
    params.setTo(paramsBuf, len);
    
    sp<ICamera> camera = *((sp<ICamera>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = camera->setParameters(params);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// CameraClient_getParameters
RpcResponse* CameraClient_getParameters(RpcRequest* request)
{
    RPC_SERVER_REQUEST_COMMON()
    
    sp<ICamera> camera = *((sp<ICamera>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    String8 params = camera->getParameters();
    
    RpcResponse* response = new RpcResponse(true);
    size_t len = params.length();
    response->putRet((char*) &len, sizeof(len));
    response->putRet((char*) params.string(), len);
    
    return response;
}

// CameraClient_sendCommand
RPC_GENERATE_SERVER_METHOD31(CC_, ICamera, sendCommand, int32_t, int32_t, int32_t, status_t)

/*static bool isFirst = true;
static pthread_mutex_t lock;
static pthread_cond_t cond;
static std::queue<RpcRequest*> frameReqs;

static void* syncthLoop(void* args)
{
    while (true) {
        pthread_mutex_lock(&lock);
        // get an available buffer from the camera preview
        if (frameReqs.empty()) {
            pthread_cond_wait(&cond, &lock);
        }
        RpcRequest* request = frameReqs.front();
        frameReqs.pop();
        pthread_mutex_unlock(&lock);
        CLIENT_METH_PROFILING_START(CameraRpcUtilInst.CAMERA_PREVIEW_REFRESH_ID, CC_METH_REFRESH_PREVIEW_WINDOW)
        
        RpcResponse* response = CameraRpcUtilInst.rpcserver->doRpc(request);
        CLIENT_METH_PROFILING_END(response->seqNo)
        delete response;
    }
    
    return NULL;
}*/        

// server send preview data to refresh preview window for the camera client
void refreshPreviewWindow(const uint8_t* data, size_t size, int width, int height, int stride, int format, int usage)
{
    /*if (isFirst) {
        isFirst = false;
    
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
        pthread_t frameSyncThread;
        pthread_create(&frameSyncThread, NULL, syncthLoop, NULL);
    }
    if (frameReqs.size() >= 5) {
        return;
    }*/
    for (unsigned int i = 0; i < CameraRpcUtilInst.rpcserver->cliSocketFds.size(); i++) {
        CLIENT_METH_PROFILING_START(CameraRpcUtilInst.CAMERA_PREVIEW_REFRESH_ID, CC_METH_REFRESH_PREVIEW_WINDOW)
        RpcRequest* request = new RpcRequest(CameraRpcUtilInst.CAMERA_PREVIEW_REFRESH_ID,
            CC_METH_REFRESH_PREVIEW_WINDOW, CameraRpcUtilInst.rpcserver->cliSocketFds[i], true);
        uid_t uidval = IPCThreadState::self()->getCallingUid();
        pid_t pidval = IPCThreadState::self()->getCallingPid();
        request->putArg((char*) &uidval, sizeof(uidval));
        request->putArg((char*) &pidval, sizeof(pidval));
        
        request->putArg((char*) &size, sizeof(size));
        request->putArg((char*) data, size);
        request->putArg((char*) &width, sizeof(width));
        request->putArg((char*) &height, sizeof(height));
        request->putArg((char*) &stride, sizeof(stride));
        request->putArg((char*) &format, sizeof(format));
        request->putArg((char*) &usage, sizeof(usage));
        
        /*pthread_mutex_lock(&lock);
        frameReqs.push(request);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);*/
        RpcResponse* response = CameraRpcUtilInst.rpcserver->doRpc(request);
        CLIENT_METH_PROFILING_END(response->seqNo)
        delete response;
    }
}

// ---------------------------------------------------------------------------
int registerCameraClientForRpc(sp<ICamera> camera) {
    int serviceId = CameraRpcUtilInst.nextServiceObjId++;
    sp<ICamera>* cameraref = new sp<ICamera>(camera);
    CameraRpcUtilInst.idToObjMap[serviceId] = cameraref;
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_DISCONNECT, &Rpc_CC_disconnect);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_CONNECT, &CameraClient_connect);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_LOCK, &Rpc_CC_lock);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_UNLOCK, &Rpc_CC_unlock);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_SET_PREVIEW_TARGET, &CameraClient_setPreviewTarget);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_SET_PREVIEW_CALLBACK_FLAG, &Rpc_CC_setPreviewCallbackFlag);
    //CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_SET_PREVIEW_CALLBACK_TARGET, &CameraClient_setPreviewCallbackTarget);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_START_PREVIEW, &Rpc_CC_startPreview);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_STOP_PREVIEW, &Rpc_CC_stopPreview);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_PREVIEW_ENABLED, &Rpc_CC_previewEnabled);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_STORE_METADATA_IN_BUFFERS, &Rpc_CC_storeMetaDataInBuffers);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_START_RECORDING, &Rpc_CC_startRecording);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_STOP_RECORDING, &Rpc_CC_stopRecording);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_RECORDING_ENABLED, &Rpc_CC_recordingEnabled);
   // CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_RELEASE_RECORDING_FRAME, &CameraClient_releaseRecordingFrame);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_AUTO_FOCUS, &Rpc_CC_autoFocus);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_CANCEL_AUTO_FOCUS, &Rpc_CC_cancelAutoFocus);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_TAKE_PICTURE, &Rpc_CC_takePicture);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_SET_PARAMETERS, &CameraClient_setParameters);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_GET_PARAMETERS, &CameraClient_getParameters);
    CameraRpcUtilInst.rpcserver->registerFunc(serviceId, CC_METH_SEND_COMMAND, &Rpc_CC_sendCommand);
    
    return serviceId;
}

}
