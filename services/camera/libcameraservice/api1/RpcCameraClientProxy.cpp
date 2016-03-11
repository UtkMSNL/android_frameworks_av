
#include "RpcCameraClientProxy.h"
#include <gui/BufferQueue.h>
#include <camera/RpcICameraClientServer.h>

// method id for Camera Service
#define CC_METH_REFRESH_PREVIEW_WINDOW 1

namespace android {
    
// ----------------------------------------------------------------------------
//      RPC Camera client in the server
// ----------------------------------------------------------------------------
sp<IGraphicBufferProducer> RpcCameraClientProxy::mBufferProducer;
sp<GraphicBuffer> RpcCameraClientProxy::mBufferSlot[BufferQueue::NUM_BUFFER_SLOTS];

RpcCameraClientProxy::RpcCameraClientProxy(int remoteServiceId)
    : mRemoteServiceId(remoteServiceId) {}
    
RpcCameraClientProxy::~RpcCameraClientProxy() {}
    
void RpcCameraClientProxy::disconnect()
{
    RPC_CLIENT_FUNC00(mRemoteServiceId, CC_METH_DISCONNECT)
    unregisterICameraClient(mRemoteServiceId);
}

status_t RpcCameraClientProxy::connect(const sp<ICameraClient>& client)
{
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CC_METH_CONNECT)
    int cameraClientId = registerICameraClientForRpc(client);
    request->putArg((char*) &cameraClientId, sizeof(cameraClientId));
    
    RPC_CLIENT_RESPONSE_RET(status_t)
}

status_t RpcCameraClientProxy::lock()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_LOCK, status_t)
}

status_t RpcCameraClientProxy::unlock()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_UNLOCK, status_t)
}

status_t RpcCameraClientProxy::setPreviewTarget(const sp<IGraphicBufferProducer>& bufferProducer) {
    RpcCameraClientProxy::mBufferProducer = sp<IGraphicBufferProducer>(bufferProducer);
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CC_METH_SET_PREVIEW_TARGET)
    RPC_CLIENT_RESPONSE_RET(status_t)
}

void RpcCameraClientProxy::setPreviewCallbackFlag(int flag)
{
    RPC_CLIENT_FUNC10(mRemoteServiceId, CC_METH_SET_PREVIEW_CALLBACK_FLAG, flag)
}

status_t RpcCameraClientProxy::setPreviewCallbackTarget(const sp<IGraphicBufferProducer>& callbackProducer)
{
    // FIXME: support this call
    return -1;
}

status_t RpcCameraClientProxy::startPreview()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_START_PREVIEW, status_t)
}

void RpcCameraClientProxy::stopPreview()
{
    RPC_CLIENT_FUNC00(mRemoteServiceId, CC_METH_STOP_PREVIEW)
}

bool RpcCameraClientProxy::previewEnabled()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_PREVIEW_ENABLED, bool)
}

status_t RpcCameraClientProxy::storeMetaDataInBuffers(bool enabled)
{
    RPC_CLIENT_FUNC11(mRemoteServiceId, CC_METH_STORE_METADATA_IN_BUFFERS, enabled, status_t)
}

status_t RpcCameraClientProxy::startRecording()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_START_RECORDING, status_t)
}

void RpcCameraClientProxy::stopRecording()
{
    RPC_CLIENT_FUNC00(mRemoteServiceId, CC_METH_STOP_RECORDING)
}

bool RpcCameraClientProxy::recordingEnabled()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_RECORDING_ENABLED, bool)
}

void RpcCameraClientProxy::releaseRecordingFrame(const sp<IMemory>& mem){
    // FIXME: support this call
    return;
}

status_t RpcCameraClientProxy::autoFocus()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_AUTO_FOCUS, status_t)
}

status_t RpcCameraClientProxy::cancelAutoFocus()
{
    RPC_CLIENT_FUNC01(mRemoteServiceId, CC_METH_CANCEL_AUTO_FOCUS, status_t)
}

status_t RpcCameraClientProxy::takePicture(int msgType)
{
    RPC_CLIENT_FUNC11(mRemoteServiceId, CC_METH_TAKE_PICTURE, msgType, status_t)
}

status_t RpcCameraClientProxy::setParameters(const String8& params)
{
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CC_METH_SET_PARAMETERS)
    size_t len = params.length();
    request->putArg((char*) &len, sizeof(len));
    request->putArg((char*) params.string(), len);
    RPC_CLIENT_RESPONSE_RET(status_t)
}

String8 RpcCameraClientProxy::getParameters() const
{
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, CC_METH_GET_PARAMETERS)
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    size_t len;
    response->getRet((char*) &len, sizeof(len));
    char paramsBuf[len];
    response->getRet(paramsBuf, len);
    String8 result;
    result.setTo(paramsBuf, len);
    
    delete response;
    
    return result;
}
status_t RpcCameraClientProxy::sendCommand(int32_t cmd, int32_t arg1, int32_t arg2)
{
    RPC_CLIENT_FUNC31(mRemoteServiceId, CC_METH_SEND_COMMAND, cmd, arg1, arg2, status_t)
}

// client receive preview data to refresh preview window
RpcResponse* CameraClient_refreshPreviewWindow(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    size_t size;
    request->getArg((char*) &size, sizeof(size));
    uint8_t data[size];
    request->getArg((char*) data, size);
    int width;
    request->getArg((char*) &width, sizeof(width));
    int height;
    request->getArg((char*) &height, sizeof(height));
    int stride;
    request->getArg((char*) &stride, sizeof(stride));
    int format;
    request->getArg((char*) &format, sizeof(format));
    int usage;
    request->getArg((char*) &usage, sizeof(usage));
    RpcResponse* response = new RpcResponse(false);
    if (RpcCameraClientProxy::mBufferProducer == NULL) {
        return response;
    }
    sp<IGraphicBufferProducer> mProducer(RpcCameraClientProxy::mBufferProducer);
    int buf = -1;
    sp<Fence> fence;
    status_t err = mProducer->dequeueBuffer(&buf, &fence, false, width, height, format, usage);
    if (err < 0) {
        ALOGE("rpc camera service dequeue buffer failed in listener");
        return response;
    }
    sp<GraphicBuffer>& gbuf(RpcCameraClientProxy::mBufferSlot[buf]);
    if (err & IGraphicBufferProducer::RELEASE_ALL_BUFFERS) {
        for (int i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
            RpcCameraClientProxy::mBufferSlot[i] = 0;
        }
    }
    if ((err & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
        err = mProducer->requestBuffer(buf, &gbuf);
        if (err != NO_ERROR) {
            ALOGE("rpc camera service dequeueBuffer: IGraphicBufferProducer::requestBuffer failed: %d", err);
            mProducer->cancelBuffer(buf, fence);
            return response;
        }
    }
    if (fence->isValid()) {
        err = fence->waitForever("RpcBufferConsumerListener::onFrameAvailable::dequeue");
        if (err != OK) {
            ALOGW("failed to wait for buffer fence in dequeue: %d", err);
            // keep going
        }
    }
    sp<GraphicBuffer> dst(gbuf);
    uint8_t* dst_bits = NULL;
    err = dst->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dst_bits);
    ALOGE_IF(err, "error locking dst buffer %s", strerror(-err));
    if (dst_bits) {
        // FIXME: set the bytes per pixel through query the pixel format
        const size_t bpp = 1;
        const size_t bpr = stride * bpp;
        memcpy(dst_bits, data, size);
        ALOGE("rpc camera service copied data: width[%d], height[%d], stride[%d], format[%d], usage[%d]", width, height, stride, format, usage);
    }
    if (dst_bits)
        dst->unlock();
    // queue the producer buffer
    int64_t timestamp;
    timestamp = systemTime(SYSTEM_TIME_MONOTONIC);
    // Make sure the crop rectangle is entirely inside the buffer.
    Rect crop(width, height);

    IGraphicBufferProducer::QueueBufferOutput output;
    // NATIVE_WINDOW_SCALING_MODE_FREEZE this doesn't work, we need to make it scaling
    IGraphicBufferProducer::QueueBufferInput input(timestamp, true, crop,
            NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW, 0 ^ 0, false,
            Fence::NO_FENCE, 0);
    err = mProducer->queueBuffer(buf, input, &output);
    if (err != OK)  {
        ALOGE("rpc camera service queueBuffer: error queuing buffer, %d", err);
    }
    return response;
}

// ---------------------------------------------------------------------------
__attribute__ ((visibility ("default"))) void registerRpcCameraClientProxy() {
    CameraRpcUtilInst.rpcclient->registerFunc(CameraRpcUtilInst.CAMERA_PREVIEW_REFRESH_ID, CC_METH_REFRESH_PREVIEW_WINDOW, &CameraClient_refreshPreviewWindow);
}

}
