#ifndef ANDROID_RPC_CAMERA_CLIENT_SERVER_H
#define ANDROID_RPC_CAMERA_CLIENT_SERVER_H

#include "CameraClient.h"
#include <camera/RpcCameraCommon.h>
#include <rpc/share_rpc.h>
#include <gui/BufferQueue.h>

namespace android {

class RpcCameraClientProxy : public CameraClient
{
public:
    RpcCameraClientProxy(int remoteServiceId);
    virtual ~RpcCameraClientProxy();
    virtual void            disconnect();
    virtual status_t        connect(const sp<ICameraClient>& client);
    virtual status_t        lock();
    virtual status_t        unlock();
    virtual status_t        setPreviewTarget(const sp<IGraphicBufferProducer>& bufferProducer);
    virtual void            setPreviewCallbackFlag(int flag);
    virtual status_t        setPreviewCallbackTarget(
            const sp<IGraphicBufferProducer>& callbackProducer);
    virtual status_t        startPreview();
    virtual void            stopPreview();
    virtual bool            previewEnabled();
    virtual status_t        storeMetaDataInBuffers(bool enabled);
    virtual status_t        startRecording();
    virtual void            stopRecording();
    virtual bool            recordingEnabled();
    virtual void            releaseRecordingFrame(const sp<IMemory>& mem);
    virtual status_t        autoFocus();
    virtual status_t        cancelAutoFocus();
    virtual status_t        takePicture(int msgType);
    virtual status_t        setParameters(const String8& params);
    virtual String8         getParameters() const;
    virtual status_t        sendCommand(int32_t cmd, int32_t arg1, int32_t arg2);
    
    static sp<IGraphicBufferProducer> mBufferProducer;
    static sp<GraphicBuffer> mBufferSlot[BufferQueue::NUM_BUFFER_SLOTS];
    int mRemoteServiceId;
};

// ---------------------------------------------------------------------------    
void registerRpcCameraClientProxy();

}

#endif
