
#include <utils/Log.h>
#include <ui/PixelFormat.h>
#include <cstdlib>

#include "RpcBufferConsumerListener.h"
#include "RpcCameraClientServer.h"

#include <pthread.h>

namespace android {

static pthread_mutex_t lock;
static pthread_cond_t cond;
static sp<IGraphicBufferConsumer> consumer;
static sp<GraphicBuffer> mBufferSlot[BufferQueue::NUM_BUFFER_SLOTS];

static bool isFirst = true;

#define ALIGN_UP(x, a)    (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_TO_8KB(x)   ((((x) + (1 << 13) - 1) >> 13) << 13)
#define ALIGN_TO_128B(x)  ((((x) + (1 <<  7) - 1) >>  7) <<  7)
#define ALIGN_TO_32B(x)   ((((x) + (1 <<  5) - 1) >>  5) <<  5)
static void* prvthLoop(void* args)
{
    while (true) {
    //struct timeval startLoop;
    //gettimeofday(&startLoop, NULL);
    
        // get an available buffer from the camera preview
        BufferQueue::BufferItem* item = new BufferQueue::BufferItem();
        status_t err = consumer->acquireBuffer(item, 0);
        if (err != OK) {
            pthread_mutex_lock(&lock);
            pthread_cond_wait(&cond, &lock);
            pthread_mutex_unlock(&lock);
            continue;
        }
        
        err = item->mFence->waitForever("RpcBufferConsumerListener::onFrameAvailable");
        //ALOGE("a buffer item is available after wait %p, %d, %d", item->mFence.get(), item->mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
        if (err != OK) {
            ALOGW("failed to wait for buffer fence: %d", err);
            // keep going
        }
        if (item->mGraphicBuffer != NULL) {
            ALOGV("RpcBufferConsumerListener::onFrameAvailable: setting mBufferSlot %d", item->mBuf);
            mBufferSlot[item->mBuf] = item->mGraphicBuffer;
        }
        int buf = item->mBuf;
        int frameNumber = item->mFrameNumber;
        sp<GraphicBuffer> src = mBufferSlot[item->mBuf];
        sp<Fence> fence = item->mFence;
        delete item;
        
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
        //const size_t bpr = width * bpp;
        //const size_t size = bpr * height + ((bpr + 1) / 2) * ((height + 1) / 2) * 2;
        // for samsung TODO: find out its size for this pixel format
        size_t size = ALIGN_UP(ALIGN_UP(width, 16) * ALIGN_UP(height, 16), 2048);
        size = size + ALIGN_UP(ALIGN_UP(width, 16) * ALIGN_UP(height >> 1, 8), 2048);
        //format = 0x11c;
        refreshPreviewWindow(src_bits, size, width, height, stride, format, usage);
        if (src_bits)
            src->unlock();

        //ALOGI("a buffer item is available with data size[%d], width[%d], height[%d], format[%#x]", size, width, height, format);
        // release the prevew buffer
        err = consumer->releaseBuffer(buf, frameNumber, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, fence);
        ALOGE_IF(err, "error release buffer %s, err, %d", strerror(-err), err);
        
    //struct timeval finishLoop;
    //gettimeofday(&finishLoop, NULL);
       // u8 timerate = (finishLoop.tv_sec - startLoop.tv_sec) * 1000000 + (finishLoop.tv_usec - startLoop.tv_usec);
       // ALOGE("[fps evaluation], per frame processing time: %lld", timerate);
    }
    return NULL;
}

static struct timeval perFrameLastTime;
void RpcBufferConsumerListener::onFrameAvailable(const BufferItem& /*item*/)
{
    if (isFirst) {
        isFirst = false;
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
        pthread_t rpcThread;
        pthread_create(&rpcThread, NULL, prvthLoop, NULL);
    }
    pthread_mutex_lock(&lock);
    consumer = mConsumer;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
        /*struct timeval finish;
        gettimeofday(&finish, NULL);
        u8 timerate = (finish.tv_sec - perFrameLastTime.tv_sec) * 1000000 + (finish.tv_usec - perFrameLastTime.tv_usec);
        ALOGE("[fps evaluation], time since last frame: %lld", timerate);
        gettimeofday(&perFrameLastTime, NULL);*/
    
    // this part is moved into a thread because the samsumg phone have the problem
    // the problem is that: after calling graphicBuffer->lock(), the data content in the BufferItem is changed
    // this makes the destruction of the item throw a runtime error
    /*// get an available buffer from the camera preview
    BufferQueue::BufferItem* item = new BufferQueue::BufferItem();
    ALOGE("a buffer item is available with %p", item);
    status_t err = mConsumer->acquireBuffer(item, 0);
    ALOGE("a buffer item is available with %p", item);
    if (err == BufferQueue::NO_BUFFER_AVAILABLE) {
        // shouldn't happen
        ALOGW("RpcBufferConsumerListener::onFrameAvailable: frame was not available");
        return;
    } else if (err != OK) {
        // now what? fake end-of-stream?
        ALOGW("RpcBufferConsumerListener::onFrameAvailable: acquireBuffer returned err=%d", err);
        return;
    }
    ALOGE("a buffer item is available with data %p, %d, %d", item->mFence.get(), item->mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
    err = item->mFence->waitForever("RpcBufferConsumerListener::onFrameAvailable");
    ALOGE("a buffer item is available after wait %p, %d, %d", item->mFence.get(), item->mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
    if (err != OK) {
        ALOGW("failed to wait for buffer fence: %d", err);
        // keep going
    }
    if (item->mGraphicBuffer != NULL) {
        ALOGV("RpcBufferConsumerListener::onFrameAvailable: setting mBufferSlot %d", item->mBuf);
        mBufferSlot[item->mBuf] = item->mGraphicBuffer;
    }
    ALOGE("a buffer item is available with data size 1:  %p, %d, %d", item->mFence.get(), item->mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
    int buf = item->mBuf;
    int frameNumber = item->mFrameNumber;
    sp<GraphicBuffer> src = mBufferSlot[item->mBuf];
    sp<Fence> fence = item->mFence;
    ALOGE("a buffer item is available with data size 2:  %p, %d, %d, %p", item->mFence.get(), item->mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS, src.get());
    delete item;
    
    int width = src->width;
    int height = src->height;
    int stride = src->stride;
    int format = src->format;
    int usage = src->usage;
    
    uint8_t const * src_bits = NULL;
    err = src->lock(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_RARELY, (void**)&src_bits);
    //ALOGE("a buffer item is available with data size 3: %p, %d, %d, %p, format: %#x", item->mFence.get(), item->mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS, item->mGraphicBuffer.get(), format);
    ALOGE_IF(err, "error locking src buffer %s", strerror(-err));
    // FIXME: set the bytes per pixel through the pixel format
    const size_t bpp = 1;
    const size_t bpr = stride * bpp;
    const size_t size = bpr * height + ((bpr + 1) / 2) * ((height + 1) / 2) * 2;
    //ALOGE("a buffer item is available with data size 4: %d, %p, %d, %d", size, item.mFence.get(), item.mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
    uint8_t* data = (uint8_t*) malloc(size);
    memcpy(data, src_bits, size);
    refreshPreviewWindow(src_bits, size, width, height, stride, format, usage);
    //ALOGE("a buffer item is available with data size 5: %d, %p, %d, %d", size, item.mFence.get(), item.mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
    if (src_bits)
        src->unlock();

    //ALOGE("a buffer item is available with data size: %d, %p, %d, %d", size, fence.get(), item.mBuf, BufferQueueDefs::NUM_BUFFER_SLOTS);
    // release the prevew buffer
    err = mConsumer->releaseBuffer(buf, frameNumber, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, fence);
    ALOGE_IF(err, "error release buffer %s, err, %d", strerror(-err), err);*/
    
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

