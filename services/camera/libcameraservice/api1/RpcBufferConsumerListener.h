#ifndef RPC_BUFFER_CONSUMER_LISTENER_H_
#define RPC_BUFFER_CONSUMER_LISTENER_H_

#include <gui/BufferQueue.h>

namespace android {

class RpcBufferConsumerListener : public BufferQueue::ConsumerListener {
public:
    sp<IGraphicBufferConsumer> mConsumer;
    sp<GraphicBuffer> mBufferSlot[BufferQueue::NUM_BUFFER_SLOTS];
    
protected:
    // BufferQueue::ConsumerListener interface, called when a new frame of
    // data is available.  If we're executing and a codec buffer is
    // available, we acquire the buffer, copy the GraphicBuffer reference
    // into the codec buffer, and call Empty[This]Buffer.  If we're not yet
    // executing or there's no codec buffer available, we just increment
    // mNumFramesAvailable and return.
    virtual void onFrameAvailable(const BufferItem& item);
    
    // onFrameReplaced is called from queueBuffer if the frame being queued is
    // replacing an existing slot in the queue. Any call to queueBuffer that
    // doesn't call onFrameAvailable will call this callback instead. The item
    // passed to the callback will contain all of the information about the
    // queued frame except for its GraphicBuffer pointer, which will always be
    // null.
    //
    // This is called without any lock held and can be called concurrently
    // by multiple threads.
    virtual void onFrameReplaced(const BufferItem& /* item */); /* Asynchronous */

    // BufferQueue::ConsumerListener interface, called when the client has
    // released one or more GraphicBuffers.  We clear out the appropriate
    // set of mBufferSlot entries.
    virtual void onBuffersReleased();

    // BufferQueue::ConsumerListener interface, called when the client has
    // changed the sideband stream. GraphicBufferSource doesn't handle sideband
    // streams so this is a no-op (and should never be called).
    virtual void onSidebandStreamChanged();

};

}  // namespace android

#endif  // RPC_BUFFER_CONSUMER_LISTENER_H_
