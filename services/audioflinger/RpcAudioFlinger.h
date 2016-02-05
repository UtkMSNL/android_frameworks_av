/*
** Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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
** limitations under the License.
**
*/

#ifndef ANDROID_RPC_AUDIO_FLINGER_H
#define ANDROID_RPC_AUDIO_FLINGER_H

#include "AudioFlinger.h"
#include "RpcAudioCommon.h"
#include "rpc/share_rpc.h"

namespace android {

// ----------------------------------------------------------------------------

class RpcAudioFlinger :
    public BinderService<RpcAudioFlinger>,
    public BnAudioFlinger
{
    friend class BinderService<RpcAudioFlinger>;   // for RpcAudioFlinger()
public:
    static const char* getServiceName() ANDROID_API { return "media.rpc_audio_flinger"; }

    // IAudioFlinger interface, in binder opcode order
    virtual sp<IAudioTrack> createTrack(
                                audio_stream_type_t streamType,
                                uint32_t sampleRate,
                                audio_format_t format,
                                audio_channel_mask_t channelMask,
                                size_t *pFrameCount,
                                IAudioFlinger::track_flags_t *flags,
                                const sp<IMemory>& sharedBuffer,
                                audio_io_handle_t output,
                                pid_t tid,
                                int *sessionId,
                                int clientUid,
                                status_t *status /*non-NULL*/);

#ifdef QCOM_DIRECTTRACK
    virtual sp<IDirectTrack> createDirectTrack(
                                pid_t pid,
                                uint32_t sampleRate,
                                audio_channel_mask_t channelMask,
                                audio_io_handle_t output,
                                int *sessionId,
                                IDirectTrackClient* client,
                                audio_stream_type_t streamType,
                                status_t *status);
    virtual void deleteEffectSession();
#endif

    virtual sp<IAudioRecord> openRecord(
                                audio_io_handle_t input,
                                uint32_t sampleRate,
                                audio_format_t format,
                                audio_channel_mask_t channelMask,
                                size_t *pFrameCount,
                                IAudioFlinger::track_flags_t *flags,
                                pid_t tid,
                                int *sessionId,
                                size_t *notificationFrames,
                                sp<IMemory>& cblk,
                                sp<IMemory>& buffers,
                                status_t *status /*non-NULL*/);

    virtual     uint32_t    sampleRate(audio_io_handle_t output) const;
    virtual     audio_format_t format(audio_io_handle_t output) const;
    virtual     size_t      frameCount(audio_io_handle_t output) const;
    virtual     uint32_t    latency(audio_io_handle_t output) const;

    virtual     status_t    setMasterVolume(float value);
    virtual     status_t    setMasterMute(bool muted);

    virtual     float       masterVolume() const;
    virtual     bool        masterMute() const;

    virtual     status_t    setStreamVolume(audio_stream_type_t stream, float value,
                                            audio_io_handle_t output);
    virtual     status_t    setStreamMute(audio_stream_type_t stream, bool muted);

    virtual     float       streamVolume(audio_stream_type_t stream,
                                         audio_io_handle_t output) const;
    virtual     bool        streamMute(audio_stream_type_t stream) const;

    virtual     status_t    setMode(audio_mode_t mode);

    virtual     status_t    setMicMute(bool state);
    virtual     bool        getMicMute() const;

    virtual     status_t    setParameters(audio_io_handle_t ioHandle, const String8& keyValuePairs);
    virtual     String8     getParameters(audio_io_handle_t ioHandle, const String8& keys) const;

    virtual     void        registerClient(const sp<IAudioFlingerClient>& client);
#ifdef QCOM_DIRECTTRACK
    virtual    status_t     deregisterClient(const sp<IAudioFlingerClient>& client);
#endif
    virtual     size_t      getInputBufferSize(uint32_t sampleRate, audio_format_t format,
                                               audio_channel_mask_t channelMask) const;

    virtual status_t openOutput(audio_module_handle_t module,
                                audio_io_handle_t *output,
                                audio_config_t *config,
                                audio_devices_t *devices,
                                const String8& address,
                                uint32_t *latencyMs,
                                audio_output_flags_t flags);

    virtual audio_io_handle_t openDuplicateOutput(audio_io_handle_t output1,
                                                  audio_io_handle_t output2);

    virtual status_t closeOutput(audio_io_handle_t output);

    virtual status_t suspendOutput(audio_io_handle_t output);

    virtual status_t restoreOutput(audio_io_handle_t output);

    virtual status_t openInput(audio_module_handle_t module,
                               audio_io_handle_t *input,
                               audio_config_t *config,
                               audio_devices_t *device,
                               const String8& address,
                               audio_source_t source,
                               audio_input_flags_t flags);

    virtual status_t closeInput(audio_io_handle_t input);

    virtual status_t invalidateStream(audio_stream_type_t stream);

    virtual status_t setVoiceVolume(float volume);

    virtual status_t getRenderPosition(uint32_t *halFrames, uint32_t *dspFrames,
                                       audio_io_handle_t output) const;

    virtual uint32_t getInputFramesLost(audio_io_handle_t ioHandle) const;

    virtual audio_unique_id_t newAudioUniqueId();

    virtual void acquireAudioSessionId(int audioSession, pid_t pid);

    virtual void releaseAudioSessionId(int audioSession, pid_t pid);

    virtual status_t queryNumberEffects(uint32_t *numEffects) const;

    virtual status_t queryEffect(uint32_t index, effect_descriptor_t *descriptor) const;

    virtual status_t getEffectDescriptor(const effect_uuid_t *pUuid,
                                         effect_descriptor_t *descriptor) const;

    virtual sp<IEffect> createEffect(
                        effect_descriptor_t *pDesc,
                        const sp<IEffectClient>& effectClient,
                        int32_t priority,
                        audio_io_handle_t io,
                        int sessionId,
                        status_t *status /*non-NULL*/,
                        int *id,
                        int *enabled);

    virtual status_t moveEffects(int sessionId, audio_io_handle_t srcOutput,
                        audio_io_handle_t dstOutput);

    virtual audio_module_handle_t loadHwModule(const char *name);

    virtual uint32_t getPrimaryOutputSamplingRate();
    virtual size_t getPrimaryOutputFrameCount();

    virtual status_t setLowRamDevice(bool isLowRamDevice);

    /* List available audio ports and their attributes */
    virtual status_t listAudioPorts(unsigned int *num_ports,
                                    struct audio_port *ports);

    /* Get attributes for a given audio port */
    virtual status_t getAudioPort(struct audio_port *port);

    /* Create an audio patch between several source and sink ports */
    virtual status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle);

    /* Release an audio patch */
    virtual status_t releaseAudioPatch(audio_patch_handle_t handle);

    /* List existing audio patches */
    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches);

    /* Set audio port configuration */
    virtual status_t setAudioPortConfig(const struct audio_port_config *config);

    /* Get the HW synchronization source used for an audio session */
    virtual audio_hw_sync_t getAudioHwSyncForSession(audio_session_t sessionId);

    virtual     status_t    onTransact(
                                uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags);

#ifdef QCOM_DIRECTTRACK
    bool applyEffectsOn(void *token,
                        int16_t *buffer1,
                        int16_t *buffer2,
                        int size,
                        bool force);
#endif

    // end of IAudioFlinger interface

private:
    virtual                 ~RpcAudioFlinger();

    // RefBase
    virtual     void        onFirstRef();
    
    // --- RpcDummyClient ---
    class RpcDummyClient : public AudioFlinger::Client {
    public:
                            RpcDummyClient(const sp<RpcAudioFlinger>& audioFlinger, pid_t pid);
        virtual             ~RpcDummyClient();
    private:
        int mRemoteServiceId;
        const sp<RpcAudioFlinger> mAudioFlinger;
        const sp<MemoryDealer>    mMemoryDealer;
        const pid_t               mPid;
        
        int                 mTimedTrackCount;
        //const int mRemoteServiceId;
    }; 
    
    mutable Mutex mClientLock;
    // protected by mClientLock
    DefaultKeyedVector< pid_t, wp<RpcDummyClient> > mClients;
    
    class RpcDummyTrack : public RefBase {
    public:
        RpcDummyTrack(
                const sp<RpcAudioFlinger::RpcDummyClient>& client,
                audio_format_t format,
                size_t frameCount,
                void *buffer,
                bool isOut,
                audio_channel_mask_t channelMask);
        sp<IMemory> getCblk() const { return mCblkMemory; }

    private:
        sp<IMemory> mCblkMemory;
        audio_track_cblk_t* mCblk;
        void* mBuffer;
    };

    // server side of the client's IAudioTrack
    class RpcTrackHandleProxy : public AudioFlinger::TrackHandle {
    public:
                            RpcTrackHandleProxy(const sp<RpcAudioFlinger::RpcDummyTrack>& track, const int remoteServiceId);
        virtual             ~RpcTrackHandleProxy();
        virtual sp<IMemory> getCblk() const;
        virtual status_t    start();
        virtual void        stop();
        virtual void        flush();
        virtual void        pause();
        virtual status_t    attachAuxEffect(int effectId);
        virtual status_t    allocateTimedBuffer(size_t size,
                                                sp<IMemory>* buffer);
        virtual status_t    queueTimedBuffer(const sp<IMemory>& buffer,
                                             int64_t pts);
        virtual status_t    setMediaTimeTransform(const LinearTransform& xform,
                                                  int target);
        virtual status_t    setParameters(const String8& keyValuePairs);
        virtual status_t    getTimestamp(AudioTimestamp& timestamp);
        virtual void        signal(); // signal playback thread for a change in control block

        virtual status_t onTransact(
            uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags);

    private:
        void doRpc(u4 methodId);
        const int mRemoteServiceId;
        const sp<RpcDummyTrack> mTrack;
    };

private:
    sp<RpcDummyClient>  registerPidRpc(pid_t pid);    // always returns non-0

};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_RPC_AUDIO_FLINGER_H
