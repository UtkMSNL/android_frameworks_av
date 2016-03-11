
#include <utils/Log.h>
#include <ui/PixelFormat.h>

#include "RpcBufferConsumerListener.h"
#include "RpcCameraClientServer.h"

namespace android {

void RpcBufferConsumerListener::onFrameAvailable(const BufferItem& /*item*/)
{
    // get an available buffer from the camera preview
    BufferQueue::BufferItem item;
    status_t err = mConsumer->acquireBuffer(&item, 0);
    if (err == BufferQueue::NO_BUFFER_AVAILABLE) {
        // shouldn't happen
        ALOGW("RpcBufferConsumerListener::onFrameAvailable: frame was not available");
        return;
    } else if (err != OK) {
        // now what? fake end-of-stream?
        ALOGW("RpcBufferConsumerListener::onFrameAvailable: acquireBuffer returned err=%d", err);
        return;
    }
    err = item.mFence->waitForever("RpcBufferConsumerListener::onFrameAvailable");
    if (err != OK) {
        ALOGW("failed to wait for buffer fence: %d", err);
        // keep going
    }
    if (item.mGraphicBuffer != NULL) {
        ALOGV("RpcBufferConsumerListener::onFrameAvailable: setting mBufferSlot %d", item.mBuf);
        mBufferSlot[item.mBuf] = item.mGraphicBuffer;
    }
    sp<GraphicBuffer> src = mBufferSlot[item.mBuf];
    
    int width = src->width;
    int height = src->height;
    int stride = src->stride;
    int format = src->format;
    int usage = src->usage;
    
    uint8_t const * src_bits = NULL;
    err = src->lock(GRALLOC_USAGE_SW_READ_OFTEN, (void**)&src_bits);
    ALOGE_IF(err, "error locking src buffer %s", strerror(-err));
    // FIXME: set the bytes per pixel through the pixel format
    const size_t bpp = 1;
    const size_t bpr = stride * bpp;
    const size_t size = bpr * height + ((bpr + 1) / 2) * ((height + 1) / 2) * 2;
    refreshPreviewWindow(src_bits, size, width, height, stride, format, usage);
    if (src_bits)
        src->unlock();

    ALOGE("a buffer item is available with data size: %d", size);
    // release the prevew buffer
    mConsumer->releaseBuffer(item.mBuf, item.mFrameNumber, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, item.mFence);
    
    /*/ dequeue a buffer from the actual preview target
    int width = src->width;
    int height = src->height;
    int stride = src->stride;
    int format = src->format;
    int usage = src->usage;
    int buf = -1;
    sp<Fence> fence;
    status_t result = mProducer->dequeueBuffer(&buf, &fence, false, width, height, format, usage);
    if (result < 0) {
        ALOGE("rpc camera service dequeue buffer failed in listener");
        mConsumer->releaseBuffer(item.mBuf, item.mFrameNumber, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, item.mFence);
        return;
    }
    sp<GraphicBuffer>& gbuf(mpBufferSlot[buf]);
    if (result & IGraphicBufferProducer::RELEASE_ALL_BUFFERS) {
        for (int i = 0; i < BufferQueue::NUM_BUFFER_SLOTS; i++) {
            mpBufferSlot[i] = 0;
        }
    }
    if ((result & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
        result = mProducer->requestBuffer(buf, &gbuf);
        if (result != NO_ERROR) {
            ALOGE("rpc camera service dequeueBuffer: IGraphicBufferProducer::requestBuffer failed: %d", result);
            mProducer->cancelBuffer(buf, fence);
            mConsumer->releaseBuffer(item.mBuf, item.mFrameNumber, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, item.mFence);
            return;
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
    // get the pixels and copy it to the destination producer
    uint8_t const * src_bits = NULL;
    err = src->lock(GRALLOC_USAGE_SW_READ_OFTEN, (void**)&src_bits);
    ALOGE_IF(err, "error locking src buffer %s", strerror(-err));

    uint8_t* dst_bits = NULL;
    err = dst->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dst_bits);
    ALOGE_IF(err, "error locking dst buffer %s", strerror(-err));
    if (src_bits && dst_bits) {
        const size_t bpp = 1;
        const size_t bpr = stride * bpp;
        const size_t size = bpr * height + ((bpr + 1) / 2) * ((height + 1) / 2) * 2;
        memcpy(dst_bits, src_bits, size);
        ALOGE("rpc camera service copied data: width[%d], height[%d], stride[%d], format[%d], usage[%d]", width, height, stride, format, usage);
    }
    if (src_bits)
        src->unlock();

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
    */
}

void RpcBufferConsumerListener::onFrameReplaced(const BufferItem& item)
{
    ALOGE("a buffer item is replaced");
}

void RpcBufferConsumerListener::onBuffersReleased()
{
}

void RpcBufferConsumerListener::onSidebandStreamChanged()
{
}

}  // namespace android

