/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_RPCAUDIOPOLICYSERVICE_H
#define ANDROID_RPCAUDIOPOLICYSERVICE_H

#include "AudioPolicyService.h"
#include <rpc/share_rpc.h>
#include <media/RpcAudioCommon.h>

namespace android {

// ----------------------------------------------------------------------------

class RpcAudioPolicyService :
    public BinderService<RpcAudioPolicyService>,
    public BnAudioPolicyService,
    public IBinder::DeathRecipient
{
    friend class BinderService<RpcAudioPolicyService>;

public:
    // for BinderService
    static const char *getServiceName() ANDROID_API { return "media.rpc_audio_policy"; }

    virtual status_t setDeviceConnectionState(audio_devices_t device,
                                              audio_policy_dev_state_t state,
                                              const char *device_address) { return -1; }
    virtual audio_policy_dev_state_t getDeviceConnectionState(
                                                                audio_devices_t device,
                                                                const char *device_address) { return (audio_policy_dev_state_t) 0; }
    virtual status_t setPhoneState(audio_mode_t state) { return -1; }
    virtual status_t setForceUse(audio_policy_force_use_t usage, audio_policy_forced_cfg_t config) { return -1; }
    virtual audio_policy_forced_cfg_t getForceUse(audio_policy_force_use_t usage) { return (audio_policy_forced_cfg_t) 0; }
    virtual audio_io_handle_t getOutput(audio_stream_type_t stream,
                                        uint32_t samplingRate = 0,
                                        audio_format_t format = AUDIO_FORMAT_DEFAULT,
                                        audio_channel_mask_t channelMask = 0,
                                        audio_output_flags_t flags =
                                                AUDIO_OUTPUT_FLAG_NONE,
                                        const audio_offload_info_t *offloadInfo = NULL);
                                        
    virtual status_t getOutputForAttr(const audio_attributes_t *attr,
                                      audio_io_handle_t *output,
                                      audio_session_t session,
                                      audio_stream_type_t *stream,
                                      uint32_t samplingRate = 0,
                                      audio_format_t format = AUDIO_FORMAT_DEFAULT,
                                      audio_channel_mask_t channelMask = 0,
                                      audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE,
                                      const audio_offload_info_t *offloadInfo = NULL);
    virtual status_t startOutput(audio_io_handle_t output,
                                 audio_stream_type_t stream,
                                 audio_session_t session) { return -1; }
    virtual status_t stopOutput(audio_io_handle_t output,
                                audio_stream_type_t stream,
                                audio_session_t session) { return -1; }
    virtual void releaseOutput(audio_io_handle_t output,
                               audio_stream_type_t stream,
                               audio_session_t session) { }
    virtual status_t getInputForAttr(const audio_attributes_t *attr,
                                     audio_io_handle_t *input,
                                     audio_session_t session,
                                     uint32_t samplingRate,
                                     audio_format_t format,
                                     audio_channel_mask_t channelMask,
                                     audio_input_flags_t flags) { return -1; }
    virtual status_t startInput(audio_io_handle_t input,
                                audio_session_t session) { return -1; }
    virtual status_t stopInput(audio_io_handle_t input,
                               audio_session_t session) { return -1; }
    virtual void releaseInput(audio_io_handle_t input,
                              audio_session_t session) { }
    virtual status_t initStreamVolume(audio_stream_type_t stream,
                                      int indexMin,
                                      int indexMax) { return -1; }
    virtual status_t setStreamVolumeIndex(audio_stream_type_t stream,
                                          int index,
                                          audio_devices_t device) { return -1; }
    virtual status_t getStreamVolumeIndex(audio_stream_type_t stream,
                                          int *index,
                                          audio_devices_t device) { return -1; }

    virtual uint32_t getStrategyForStream(audio_stream_type_t stream) { return -1; }
    virtual audio_devices_t getDevicesForStream(audio_stream_type_t stream) { return -1; }

    virtual audio_io_handle_t getOutputForEffect(const effect_descriptor_t *desc) { return -1; }
    virtual status_t registerEffect(const effect_descriptor_t *desc,
                                    audio_io_handle_t io,
                                    uint32_t strategy,
                                    int session,
                                    int id) { return -1; }
    virtual status_t unregisterEffect(int id) { return -1; }
    virtual status_t setEffectEnabled(int id, bool enabled) { return -1; }
    virtual bool isStreamActive(audio_stream_type_t stream, uint32_t inPastMs = 0) const { return -1; }
    virtual bool isStreamActiveRemotely(audio_stream_type_t stream, uint32_t inPastMs = 0) const { return -1; }
    virtual bool isSourceActive(audio_source_t source) const { return -1; }

    virtual status_t queryDefaultPreProcessing(int audioSession,
                                              effect_descriptor_t *descriptors,
                                              uint32_t *count) { return -1; }


    // IBinder::DeathRecipient
    virtual     void        binderDied(const wp<IBinder>& who) {}

    // RefBase
    virtual     void        onFirstRef() {}

    //
    // Helpers for the struct audio_policy_service_ops implementation.
    // This is used by the audio policy manager for certain operations that
    // are implemented by the policy service.
    //
    virtual void setParameters(audio_io_handle_t ioHandle,
                               const char *keyValuePairs,
                               int delayMs) {}

    virtual status_t setStreamVolume(audio_stream_type_t stream,
                                     float volume,
                                     audio_io_handle_t output,
                                     int delayMs = 0) { return -1; }
    virtual status_t startTone(audio_policy_tone_t tone, audio_stream_type_t stream) { return -1; }
    virtual status_t stopTone() { return -1; }
    virtual status_t setVoiceVolume(float volume, int delayMs = 0) { return -1; }
    virtual bool isOffloadSupported(const audio_offload_info_t &config) { return -1; }

    virtual status_t listAudioPorts(audio_port_role_t role,
                                    audio_port_type_t type,
                                    unsigned int *num_ports,
                                    struct audio_port *ports,
                                    unsigned int *generation) { return -1; }
    virtual status_t getAudioPort(struct audio_port *port) { return -1; }
    virtual status_t createAudioPatch(const struct audio_patch *patch,
                                       audio_patch_handle_t *handle) { return -1; }
    virtual status_t releaseAudioPatch(audio_patch_handle_t handle) { return -1; }
    virtual status_t listAudioPatches(unsigned int *num_patches,
                                      struct audio_patch *patches,
                                      unsigned int *generation) { return -1; }
    virtual status_t setAudioPortConfig(const struct audio_port_config *config) { return -1; }

    virtual void registerClient(const sp<IAudioPolicyServiceClient>& client) { }

    virtual status_t acquireSoundTriggerSession(audio_session_t *session,
                                           audio_io_handle_t *ioHandle,
                                           audio_devices_t *device) { return -1; }

    virtual status_t releaseSoundTriggerSession(audio_session_t session) { return -1; }

    virtual audio_mode_t getPhoneState() { return (audio_mode_t) 0; }

    virtual status_t registerPolicyMixes(Vector<AudioMix> mixes, bool registration) { return -1; }

private:
                        RpcAudioPolicyService() ANDROID_API;
    virtual             ~RpcAudioPolicyService();
};

}; // namespace android

#endif // ANDROID_RPCAUDIOPOLICYSERVICE_H
