/*
** Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
** Not a Contribution.
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** This file was modified by Dolby Laboratories, Inc. The portions of the
** code that are surrounded by "DOLBY..." are copyrighted and
** licensed separately, as follows:
**
**  (C) 2011-2014 Dolby Laboratories, Inc.
** This file was modified by DTS, Inc. The portions of the
** code that are surrounded by "DTS..." are copyrighted and
** licensed separately, as follows:
**
**  (C) 2013 DTS, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**    http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License
*/


#define LOG_TAG "RpcAudioFlinger"

#include "RpcAudioFlinger.h"

namespace android {

#define RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                  \
    RpcRequest* request = new RpcRequest(_serviceId,                    \
            _methId, RpcUtilInst.rpcclient->socketFd, true);            \
    uid_t uidval = IPCThreadState::self()->getCallingUid();             \
    pid_t pidval = IPCThreadState::self()->getCallingPid();             \
    request->putArg((char*) &uidval, sizeof(uidval));                      \
    request->putArg((char*) &pidval, sizeof(pidval));
    
#define RPC_CLIENT_RESPONSE_NORET()                                     \
    RpcResponse* response = RpcUtilInst.rpcclient->doRpc(request);      \
    delete response; 
    
#define RPC_CLIENT_RESPONSE_RET(_retType)                               \
    RpcResponse* response = RpcUtilInst.rpcclient->doRpc(request);      \
    _retType result;                                                    \
    response->getRet((char*) &result, sizeof(result));                  \
    delete response;                                                    \
    return result;

#define RPC_CLIENT_FUNC00(_serviceId, _methId)                          \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    RPC_CLIENT_RESPONSE_NORET()

#define RPC_CLIENT_FUNC10(_serviceId, _methId, _arg1)                   \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    request->putArg((char*) &_arg1, sizeof(_arg1));                     \
    RPC_CLIENT_RESPONSE_NORET()

#define RPC_CLIENT_FUNC20(_serviceId, _methId, _arg1, _arg2)            \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    request->putArg((char*) &_arg1, sizeof(_arg1));                     \
    request->putArg((char*) &_arg2, sizeof(_arg2));                     \
    RPC_CLIENT_RESPONSE_NORET()
        
#define RPC_CLIENT_FUNC01(_serviceId, _methId, _retType)                \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    RPC_CLIENT_RESPONSE_RET(_retType)  

#define RPC_CLIENT_FUNC11(_serviceId, _methId, _arg1, _retType)         \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    request->putArg((char*) &_arg1, sizeof(_arg1));                     \
    RPC_CLIENT_RESPONSE_RET(_retType) 

#define RPC_CLIENT_FUNC21(_serviceId, _methId, _arg1, _arg2, _retType)  \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    request->putArg((char*) &_arg1, sizeof(_arg1));                     \
    request->putArg((char*) &_arg2, sizeof(_arg2));                     \
    RPC_CLIENT_RESPONSE_RET(_retType)

#define RPC_CLIENT_FUNC31(_serviceId, _methId,                          \
                                        _arg1, _arg2, _arg3, _retType)  \
    RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                      \
    request->putArg((char*) &_arg1, sizeof(_arg1));                     \
    request->putArg((char*) &_arg2, sizeof(_arg2));                     \
    request->putArg((char*) &_arg3, sizeof(_arg3));                     \
    RPC_CLIENT_RESPONSE_RET(_retType)

// ----------------------------------------------------------------------------

//RpcAudioFlinger::RpcAudioFlinger()
//    : BnAudioFlinger()
//{
//}

void RpcAudioFlinger::onFirstRef()
{
}

RpcAudioFlinger::~RpcAudioFlinger()
{
    //while (!mRecordThreads.isEmpty()) {
        // closeInput_nonvirtual() will remove specified entry from mRecordThreads
    //    closeInput_nonvirtual(mRecordThreads.keyAt(0));
    //}
    //while (!mPlaybackThreads.isEmpty()) {
        // closeOutput_nonvirtual() will remove specified entry from mPlaybackThreads
    //    closeOutput_nonvirtual(mPlaybackThreads.keyAt(0));
    //}
}

sp<RpcAudioFlinger::RpcDummyClient> RpcAudioFlinger::registerPidRpc(pid_t pid)
{
    Mutex::Autolock _cl(mClientLock);
    // If pid is already in the mClients wp<> map, then use that entry
    // (for which promote() is always != 0), otherwise create a new entry and Client.
    sp<RpcDummyClient> client = mClients.valueFor(pid).promote();
    if (client == 0) {
        client = new RpcDummyClient(this, pid);
        mClients.add(pid, client);
    }

    return client;
}

// IAudioFlinger interface


sp<IAudioTrack> RpcAudioFlinger::createTrack(
        audio_stream_type_t streamType,
        uint32_t sampleRate,
        audio_format_t format,
        audio_channel_mask_t channelMask,
        size_t *frameCount,
        IAudioFlinger::track_flags_t *flags,
        const sp<IMemory>& sharedBuffer,
        audio_io_handle_t output,
        pid_t tid,
        int *sessionId,
        int clientUid,
        status_t *status)
{
    RPC_CLIENT_REQUEST_COMMON(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_CREATE_TRACK)
    
    request->putArg((char*) &streamType, sizeof(streamType));
    request->putArg((char*) &sampleRate, sizeof(sampleRate));
    request->putArg((char*) &format, sizeof(format));
    request->putArg((char*) &channelMask, sizeof(channelMask));
    request->putArg((char*) frameCount, sizeof(size_t));
    request->putArg((char*) flags, sizeof(IAudioFlinger::track_flags_t));
    int sharedBufferFlag = (sharedBuffer == NULL) ? 0 : 1;
    request->putArg((char*) &sharedBufferFlag, sizeof(sharedBufferFlag));
    request->putArg((char*) &output, sizeof(output));
    request->putArg((char*) &tid, sizeof(tid));
    request->putArg((char*) sessionId, sizeof(int));
    request->putArg((char*) &clientUid, sizeof(clientUid));
    request->putArg((char*) status, sizeof(status_t));
    // TODO: transmit the shared buffer data to the remote
    if (sharedBufferFlag) {
    
    }
    
    RpcResponse* response = RpcUtilInst.rpcclient->doRpc(request);
    response->getRet((char*) frameCount, sizeof(size_t));
    response->getRet((char*) flags, sizeof(IAudioFlinger::track_flags_t));
    response->getRet((char*) sessionId, sizeof(int));
    response->getRet((char*) status, sizeof(status_t));
    int remoteAudioTrackId;
    response->getRet((char*) &remoteAudioTrackId, sizeof(remoteAudioTrackId));
    
    delete response;
    
    sp<RpcDummyClient> client = registerPidRpc(pidval);
    sp<RpcDummyTrack> track = new RpcDummyTrack(client, format, *frameCount, (sharedBuffer != 0) ? sharedBuffer->pointer() : NULL, true, channelMask);
    sp<RpcTrackHandleProxy> trackHandle = new RpcTrackHandleProxy(track, remoteAudioTrackId);
    
    return trackHandle;
}

uint32_t RpcAudioFlinger::sampleRate(audio_io_handle_t output) const {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SAMPLE_RATE, output, uint32_t)
}

audio_format_t RpcAudioFlinger::format(audio_io_handle_t output) const {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_FORMAT, output, audio_format_t)
}

size_t RpcAudioFlinger::frameCount(audio_io_handle_t output) const {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_FRAME_COUNT, output, size_t)
}

uint32_t RpcAudioFlinger::latency(audio_io_handle_t output) const {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_LATENCY, output, uint32_t)
}

status_t RpcAudioFlinger::setMasterVolume(float value) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MASTER_VOLUME, value, status_t)
}

status_t RpcAudioFlinger::setMasterMute(bool muted) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MASTER_MUTE, muted, status_t)
}

float RpcAudioFlinger::masterVolume() const {
    RPC_CLIENT_FUNC01(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_MASTER_VOLUME, float)
}

bool RpcAudioFlinger::masterMute() const {
    RPC_CLIENT_FUNC01(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_MASTER_MUTE, bool)
}

status_t RpcAudioFlinger::setStreamVolume(audio_stream_type_t stream, float value, audio_io_handle_t output) {
    RPC_CLIENT_FUNC31(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_STREAM_VOLUME, stream, value, output, status_t)
}

status_t RpcAudioFlinger::setStreamMute(audio_stream_type_t stream, bool muted) {
    RPC_CLIENT_FUNC21(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_STREAM_MUTE, stream, muted, status_t)
}

float RpcAudioFlinger::streamVolume(audio_stream_type_t stream, audio_io_handle_t output) const {
    RPC_CLIENT_FUNC21(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_STREAM_VOLUME, stream, output, float)
}

bool RpcAudioFlinger::streamMute(audio_stream_type_t stream) const {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_STREAM_MUTE, stream, bool)
}

status_t RpcAudioFlinger::setMode(audio_mode_t mode) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MODE, mode, status_t)
}

status_t RpcAudioFlinger::setMicMute(bool state) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MIC_MUTE, state, status_t)
}

bool RpcAudioFlinger::getMicMute() const {
    RPC_CLIENT_FUNC01(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_MIC_MUTE, bool)
}

status_t RpcAudioFlinger::setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs) {
    RPC_CLIENT_REQUEST_COMMON(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_PARAMETERS)
    
    request->putArg((char*) &ioHandle, sizeof(ioHandle));
    size_t len = keyValuePairs.length();
    request->putArg((char*) &len, sizeof(len));
    request->putArg((char*) keyValuePairs.string(), len);
    
    RPC_CLIENT_RESPONSE_RET(status_t)
}

String8 RpcAudioFlinger::getParameters(audio_io_handle_t ioHandle, const String8& keys) const {
    RPC_CLIENT_REQUEST_COMMON(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_PARAMETERS)
    
    request->putArg((char*) &ioHandle, sizeof(ioHandle));
    size_t len = keys.length();
    request->putArg((char*) &len, sizeof(len));
    request->putArg((char*) keys.string(), len);
    
    RpcResponse* response = RpcUtilInst.rpcclient->doRpc(request);
    response->getRet((char*) &len, sizeof(len));
    char keyValBuf[len];
    response->getRet(keyValBuf, len);
    String8 result;
    result.setTo(keyValBuf, len);
    
    delete response;
    
    return result;
}

//TODO: may need to add support 
void RpcAudioFlinger::registerClient(const sp<IAudioFlingerClient>& client){}

size_t RpcAudioFlinger::getInputBufferSize(uint32_t sampleRate, audio_format_t format, audio_channel_mask_t channelMask) const {
    RPC_CLIENT_FUNC31(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_INPUT_BUFFER_SIZE, sampleRate, format, channelMask, size_t)
}

status_t RpcAudioFlinger::invalidateStream(audio_stream_type_t stream) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_INVALIDATE_STREAM, stream, status_t)
}

status_t RpcAudioFlinger::setVoiceVolume(float volume) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_VOICE_VOLUME, volume, status_t)
}

status_t getRenderPosition(uint32_t *halFrames, uint32_t *dspFrames, audio_io_handle_t output) const {
    RPC_CLIENT_REQUEST_COMMON(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_RENDER_POSITION)
    
    request->putArg((char*) halFrames, sizeof(uint32_t));
    request->putArg((char*) dspFrames, sizeof(uint32_t));
    request->putArg((char*) &output, sizeof(output));
    
    RpcResponse* response = RpcUtilInst.rpcclient->doRpc(request);
    response->getRet((char*) halFrames, sizeof(uint32_t));
    response->getRet((char*) dspFrames, sizeof(uint32_t));
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    
    delete response;
    
    return result;
}

uint32_t RpcAudioFlinger::getInputFramesLost(audio_io_handle_t ioHandle) const {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_INPUT_FRAMES_LOST, ioHandle, uint32_t)
}

audio_unique_id_t RpcAudioFlinger::newAudioUniqueId() {
    RPC_CLIENT_FUNC01(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_NEW_AUDIO_UNIQUE_ID, audio_unique_id_t)
}

void RpcAudioFlinger::acquireAudioSessionId(int audioSession, pid_t pid) {
    RPC_CLIENT_FUNC20(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_ACQUIRE_AUDIO_SESSION_ID, audioSession, pid)
}

void RpcAudioFlinger::releaseAudioSessionId(int audioSession, pid_t pid) {
    RPC_CLIENT_FUNC20(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_RELEASE_AUDIO_SESSION_ID, audioSession, pid)
}

uint32_t RpcAudioFlinger::getPrimaryOutputSamplingRate() {
    RPC_CLIENT_FUNC01(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_PRIMARY_OUTPUT_SAMPLING_RATE, uint32_t)
}

size_t RpcAudioFlinger::getPrimaryOutputFrameCount() {
    RPC_CLIENT_FUNC01(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_PRIMARY_OUTPUT_FRAME_COUNT, size_t)
}

status_t RpcAudioFlinger::setLowRamDevice(bool isLowRamDevice) {
    RPC_CLIENT_FUNC11(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_LOW_RAM_DEVICE, isLowRamDevice, status_t)
}

// ----------------------------------------------------------------------------
//      RPC Dummy client
// ----------------------------------------------------------------------------

RpcAudioFlinger::RpcDummyTrack::RpcDummyTrack(
            const sp<RpcAudioFlinger::RpcDummyClient>& client,
            audio_format_t format,
            size_t frameCount,
            void *buffer,
            bool isOut,
            audio_channel_mask_t channelMask) {
    // ALOGD("Creating track with %d buffers @ %d bytes", bufferCount, bufferSize);
    // TODO: check if the static playback can use the "ALLOC_NONE" alloc type
    uint32_t channelCount = isOut ?
                audio_channel_count_from_out_mask(channelMask) :
                audio_channel_count_from_in_mask(channelMask);
    size_t frameSize = audio_is_linear_pcm(format) ?
                channelCount * audio_bytes_per_sample(format) : sizeof(int8_t);
    size_t size = sizeof(audio_track_cblk_t);
    size_t bufferSize = (((buffer == NULL) && audio_is_linear_pcm(format)) ? roundup(frameCount) : frameCount) * frameSize;
    if (buffer == NULL) {
        size += bufferSize;
    }

    mCblkMemory = client->heap()->allocate(size);
    if (mCblkMemory == 0 ||
            (mCblk = static_cast<audio_track_cblk_t *>(mCblkMemory->pointer())) == NULL) {
        ALOGE("not enough memory for AudioTrack size=%u", size);
        client->heap()->dump("AudioTrack");
        mCblkMemory.clear();
        return;
    }

    // construct the shared structure in-place.
    if (mCblk != NULL) {
        new(mCblk) audio_track_cblk_t();
        // clear all buffers
        if (buffer == NULL) {
            mBuffer = (char*)mCblk + sizeof(audio_track_cblk_t);
            memset(mBuffer, 0, bufferSize);
        } else {
            mBuffer = buffer;
        }

    }
} 

// ----------------------------------------------------------------------------

RpcAudioFlinger::RpcDummyClient::RpcDummyClient(const sp<RpcAudioFlinger>& audioFlinger, pid_t pid)
    :   mAudioFlinger(audioFlinger),
        // FIXME should be a "k" constant not hard-coded, in .h or ro. property, see 4 lines below
        mMemoryDealer(new MemoryDealer(2048*1024, "RpcAudioFlinger::RpcDummyClient")), //2MB
        mPid(pid),
        mTimedTrackCount(0)
{
    // 1 MB of address space is good for 32 tracks, 8 buffers each, 4 KB/buffer
}

// ----------------------------------------------------------------------------
//      RPC Track Handle Proxy
// ----------------------------------------------------------------------------

RpcAudioFlinger::RpcTrackHandleProxy::RpcTrackHandleProxy(const sp<RpcAudioFlinger::RpcDummyTrack>& track, const int remoteServiceId)
    : TrackHandle(),
      mRemoteServiceId(remoteServiceId),
      mTrack(track)
{
}

RpcAudioFlinger::RpcTrackHandleProxy::~RpcTrackHandleProxy() {
    RPC_CLIENT_FUNC00(mRemoteServiceId, TH_METH_DESTROY)
    
    // TODO: check if we need to release the data pointer here
    //mTrack = NULL;
}

sp<IMemory> RpcAudioFlinger::RpcTrackHandleProxy::getCblk() const {
    return mTrack->getCblk();
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::start() {
    RPC_CLIENT_FUNC01(mRemoteServiceId, TH_METH_START, status_t)
}

void RpcAudioFlinger::RpcTrackHandleProxy::stop() {
    RPC_CLIENT_FUNC00(mRemoteServiceId, TH_METH_STOP)
}

void RpcAudioFlinger::RpcTrackHandleProxy::flush() {
    RPC_CLIENT_FUNC00(mRemoteServiceId, TH_METH_FLUSH)
}

void RpcAudioFlinger::RpcTrackHandleProxy::pause() {
    RPC_CLIENT_FUNC00(mRemoteServiceId, TH_METH_PAUSE)
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::attachAuxEffect(int EffectId)
{
    RPC_CLIENT_FUNC11(mRemoteServiceId, TH_METH_ATTACH_AUX_EFFECT, EffectId, status_t);
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::allocateTimedBuffer(size_t size,
                                                         sp<IMemory>* buffer) {
    // TODO: support remote timed buffer
    return 0;
    /*if (!mTrack->isTimedTrack())
        return INVALID_OPERATION;

    PlaybackThread::TimedTrack* tt =
            reinterpret_cast<PlaybackThread::TimedTrack*>(mTrack.get());
    return tt->allocateTimedBuffer(size, buffer);*/
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::queueTimedBuffer(const sp<IMemory>& buffer,
                                                     int64_t pts) {
    // TODO: support remote timed buffer
    return 0;
    /*if (!mTrack->isTimedTrack())
        return INVALID_OPERATION;

    if (buffer == 0 || buffer->pointer() == NULL) {
        ALOGE("queueTimedBuffer() buffer is 0 or has NULL pointer()");
        return BAD_VALUE;
    }

    PlaybackThread::TimedTrack* tt =
            reinterpret_cast<PlaybackThread::TimedTrack*>(mTrack.get());
    return tt->queueTimedBuffer(buffer, pts);*/
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::setMediaTimeTransform(
    const LinearTransform& xform, int target) {
    // TODO: support remote timed buffer
    return 0;
    /*
    if (!mTrack->isTimedTrack())
        return INVALID_OPERATION;

    PlaybackThread::TimedTrack* tt =
            reinterpret_cast<PlaybackThread::TimedTrack*>(mTrack.get());
    return tt->setMediaTimeTransform(
        xform, static_cast<TimedAudioTrack::TargetTimeline>(target));*/
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::setParameters(const String8& keyValuePairs) {
    RPC_CLIENT_REQUEST_COMMON(mRemoteServiceId, TH_METH_SET_PARAMETERS)
    
    size_t len = keyValuePairs.length();
    request->putArg((char*) &len, sizeof(len));
    request->putArg((char*) keyValuePairs.string(), len);
    
    RPC_CLIENT_RESPONSE_RET(status_t)
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::getTimestamp(AudioTimestamp& timestamp)
{
    // TODO: support remote timed buffer
    return 0;
    /*
    return mTrack->getTimestamp(timestamp);*/
}


void RpcAudioFlinger::RpcTrackHandleProxy::signal()
{
    RPC_CLIENT_FUNC00(mRemoteServiceId, TH_METH_SIGNAL)
}

status_t RpcAudioFlinger::RpcTrackHandleProxy::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    return BnAudioTrack::onTransact(code, data, reply, flags);
}

}; // namespace android
