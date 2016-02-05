
#include "RpcAudioFlingerServer.h"

namespace android {

static void restoreToken(uid_t uid, pid_t pid) {
    // TODO: make sure that the pid will not collide to the local process
    int64_t token = uid;
    token <<= 32;
    token += (0x80000000 | pid);
    IPCThreadState::self()->restoreCallingIdentity(token);
}

#define RPC_SERVER_REQUEST_COMMON()                                         \
    uid_t uidval; \
    request->getArg((char*) &uidval, sizeof(uidval));                       \
    pid_t pidval;                                                           \
    request->getArg((char*) &pidval, sizeof(pidval));                       \
    restoreToken(uidval, pidval);

#define RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                          \
    RPC_SERVER_REQUEST_COMMON() \
    /* this serviceObj variable will be used by later method invocation */  \
    sp<_serviceType> serviceObj = *((sp<_serviceType>*) RpcUtilInst.idToObjMap[request->serviceId]);
    
#define RPC_SERVER_RESPONSE_NORET()                                         \
    RpcResponse* response = new RpcResponse(false);                         \
    return response;

#define RPC_SERVER_RESPONSE_RET()                                           \
    RpcResponse* response = new RpcResponse(true);                          \
    response->putRet((char*) &result, sizeof(result));                      \
    return response;
    
#define RPC_SERVER_FUNC00(_serviceType, _methName)                          \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    /* call the actual method */                                            \
    serviceObj->_methName();                                                \
    RPC_SERVER_RESPONSE_NORET()
    
#define RPC_SERVER_FUNC10(_serviceType, _methName, _arg1Type)               \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    _arg1Type arg1;                                                         \
    request->getArg((char*) &arg1, sizeof(arg1));                           \
    /* call the actual method */                                            \
    serviceObj->_methName(arg1);                                            \
    RPC_SERVER_RESPONSE_NORET()
    
#define RPC_SERVER_FUNC20(_serviceType, _methName, _arg1Type, _arg2Type)    \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    _arg1Type arg1;                                                         \
    request->getArg((char*) &arg1, sizeof(arg1));                           \
    _arg2Type arg2;                                                         \
    request->getArg((char*) &arg2, sizeof(arg2));                           \
    /* call the actual method */                                            \
    serviceObj->_methName(arg1, arg2);                                      \
    RPC_SERVER_RESPONSE_NORET()

#define RPC_SERVER_FUNC01(_serviceType, _methName, _retType)                \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    /* call the actual method */                                            \
    _retType result = serviceObj->_methName();                              \
    RPC_SERVER_RESPONSE_RET()

#define RPC_SERVER_FUNC11(_serviceType, _methName, _arg1Type, _retType)     \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    _arg1Type arg1; \
    request->getArg((char*) &arg1, sizeof(arg1));                           \
    /* call the actual method */                                            \
    _retType result = serviceObj->_methName(arg1);                          \
    RPC_SERVER_RESPONSE_RET()

#define RPC_SERVER_FUNC21(_serviceType, _methName, _arg1Type, _arg2Type, _retType) \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    _arg1Type arg1;                                                         \
    request->getArg((char*) &arg1, sizeof(arg1));                           \
    _arg2Type arg2;                                                         \
    request->getArg((char*) &arg2, sizeof(arg2));                           \
    /* call the actual method */                                            \
    _retType result = serviceObj->_methName(arg1, arg2);                    \
    RPC_SERVER_RESPONSE_RET()

#define RPC_SERVER_FUNC31(_serviceType, _methName, _arg1Type, _arg2Type, _arg3Type, _retType) \
    RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                              \
    _arg1Type arg1;                                                         \
    request->getArg((char*) &arg1, sizeof(arg1));                           \
    _arg2Type arg2;                                                         \
    request->getArg((char*) &arg2, sizeof(arg2));                           \
    _arg3Type arg3;                                                         \
    request->getArg((char*) &arg3, sizeof(arg3));                           \
    /* call the actual method */                                            \
    _retType result = serviceObj->_methName(arg1, arg2, arg3);              \
    RPC_SERVER_RESPONSE_RET()
    
#define RPC_GENERATE_SERVER_METHOD00(_class, _serviceType, _methName)       \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC00(_serviceType, _methName)                          \
    }
    
#define RPC_GENERATE_SERVER_METHOD10(_class, _serviceType, _methName, _arg1Type) \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC10(_serviceType, _methName, _arg1Type)               \
    }
    
#define RPC_GENERATE_SERVER_METHOD20(_class, _serviceType, _methName, _arg1Type, _arg2Type) \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC20(_serviceType, _methName, _arg1Type, _arg2Type)    \
    }
    
#define RPC_GENERATE_SERVER_METHOD01(_class, _serviceType, _methName, _retType) \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC01(_serviceType, _methName, _retType)                \
    }
    
#define RPC_GENERATE_SERVER_METHOD11(_class, _serviceType, _methName, _arg1Type, _retType) \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC11(_serviceType, _methName, _arg1Type, _retType)     \
    }
    
#define RPC_GENERATE_SERVER_METHOD21(_class, _serviceType, _methName, _arg1Type, _arg2Type, _retType) \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC21(_serviceType, _methName, _arg1Type, _arg2Type, _retType) \
    }
    
#define RPC_GENERATE_SERVER_METHOD31(_class, _serviceType, _methName, _arg1Type, _arg2Type, _arg3Type, _retType) \
    static RpcResponse* Rpc_##_class##_methName(RpcRequest* request) {      \
        RPC_SERVER_FUNC31(_serviceType, _methName, _arg1Type, _arg2Type, _arg3Type, _retType) \
    }
    
// ----------------------------------------------------------------------------
//      RPC Track Handle in the server
// ----------------------------------------------------------------------------
RpcResponse* TrackHandle_destroy(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    // remove this object reference from id map, then it will be automatically defactored
    RpcUtilInst.idToObjMap.erase(request->serviceId);
    
    RpcResponse* response = new RpcResponse(false);
    
    return response;
}

// TrackHandle_start
RPC_GENERATE_SERVER_METHOD01(TH_, IAudioTrack, start, status_t)

// TrackHandle_stop
RPC_GENERATE_SERVER_METHOD00(TH_, IAudioTrack, stop)

// TrackHandle_flush
RPC_GENERATE_SERVER_METHOD00(TH_, IAudioTrack, flush)

// TrackHandle_pause
RPC_GENERATE_SERVER_METHOD00(TH_, IAudioTrack, pause)

// TrackHandle_attachAuxEffect
RPC_GENERATE_SERVER_METHOD11(TH_, IAudioTrack, attachAuxEffect, int, status_t)

RpcResponse* TrackHandle_setParameters(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    size_t len;
    request->getArg((char*) &len, sizeof(len));
    char keyValBuf[len];
    request->getArg(keyValBuf, len);
    String8 keyValuePairs;
    keyValuePairs.setTo(keyValBuf, len);
    sp<IAudioTrack> track = *((sp<IAudioTrack>*) RpcUtilInst.idToObjMap[request->serviceId]);
    
    // call the actual method
    status_t status = track->setParameters(keyValuePairs);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &status, sizeof(status));
    
    return response;
}

// TrackHandle_signal
RPC_GENERATE_SERVER_METHOD00(TH_, IAudioTrack, signal)

// ----------------------------------------------------------------------------
//      RPC Audio Flinger in the server
// ----------------------------------------------------------------------------
RpcResponse* AudioFlinger_createTrack(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    audio_stream_type_t streamType;
    request->getArg((char*) &streamType, sizeof(streamType));
    uint32_t sampleRate;
    request->getArg((char*) &sampleRate, sizeof(sampleRate));
    audio_format_t format;
    request->getArg((char*) &format, sizeof(format));
    audio_channel_mask_t channelMask;
    request->getArg((char*) &channelMask, sizeof(channelMask));
    size_t frameCount;
    request->getArg((char*) &frameCount, sizeof(frameCount));
    IAudioFlinger::track_flags_t flags;
    request->getArg((char*) &flags, sizeof(flags));
    int sharedBufferFlag;
    request->getArg((char*) &sharedBufferFlag, sizeof(sharedBufferFlag));
    // TODO: read static audio data from the argument
    const sp<IMemory> sharedBuffer = NULL;
    audio_io_handle_t output;
    request->getArg((char*) &output, sizeof(output));
    pid_t tid;
    request->getArg((char*) &tid, sizeof(tid));
    int sessionId;
    request->getArg((char*) &sessionId, sizeof(sessionId));
    int clientUid;
    request->getArg((char*) &clientUid, sizeof(clientUid));
    status_t status;
    request->getArg((char*) &status, sizeof(status));
    
    sp<AudioFlinger> audioFlinger = *((sp<AudioFlinger>*) RpcUtilInst.idToObjMap[request->serviceId]);
    int serviceObjId = RpcUtilInst.nextServiceObjId++;
    sp<IAudioTrack> track = audioFlinger->createTrack(streamType, sampleRate, format, channelMask,
            &frameCount, &flags, sharedBuffer, output, tid, &sessionId, clientUid, &status);
    
    RpcUtilInst.idToObjMap[serviceObjId] = &track;
    // TODO: register audio track method
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_DESTROY, &TrackHandle_destroy);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_START, &Rpc_TH_start);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_STOP, &Rpc_TH_stop);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_FLUSH, &Rpc_TH_flush);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_PAUSE, &Rpc_TH_pause);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_ATTACH_AUX_EFFECT, &Rpc_TH_attachAuxEffect);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_SET_PARAMETERS, &TrackHandle_setParameters);
    RpcUtilInst.rpcserver->registerFunc(serviceObjId, TH_METH_SIGNAL, &Rpc_TH_signal);
    
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &frameCount, sizeof(frameCount));
    response->putRet((char*) &flags, sizeof(flags));
    response->putRet((char*) &sessionId, sizeof(sessionId));
    response->putRet((char*) &status, sizeof(status));
    response->putRet((char*) &serviceObjId, sizeof(serviceObjId));
    
    ALOGE("rpc audio flinger finish creating track, service Id: %d, method id: %d, seqNo: %d", request->serviceId, request->methodId, request->seqNo);
    
    return response;
}

// AudioFlinger_sampleRate
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, sampleRate, audio_io_handle_t, uint32_t)

// AudioFlinger_format
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, format, audio_io_handle_t, audio_format_t)

// AudioFlinger_frameCount
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, frameCount, audio_io_handle_t, size_t)

// AudioFlinger_latency
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, latency, audio_io_handle_t, uint32_t)

// AudioFlinger_setMasterVolume
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, setMasterVolume, float, status_t)

// AudioFlinger_setMasterMute
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, setMasterMute, bool, status_t)

// AudioFlinger_masterVolume
RPC_GENERATE_SERVER_METHOD01(AF_, AudioFlinger, masterVolume, float)

// AudioFlinger_masterMute
RPC_GENERATE_SERVER_METHOD01(AF_, AudioFlinger, masterMute, bool)

// AudioFlinger_setStreamVolume
RPC_GENERATE_SERVER_METHOD31(AF_, AudioFlinger, setStreamVolume, audio_stream_type_t, float, audio_io_handle_t, status_t)

// AudioFlinger_setStreamMute
RPC_GENERATE_SERVER_METHOD21(AF_, AudioFlinger, setStreamMute, audio_stream_type_t, bool, status_t)

// AudioFlinger_streamVolume
RPC_GENERATE_SERVER_METHOD21(AF_, AudioFlinger, streamVolume, audio_stream_type_t, audio_io_handle_t, float)

// AudioFlinger_streamMute
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, streamMute, audio_stream_type_t, bool)

// AudioFlinger_setMode
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, setMode, audio_mode_t, status_t)

// AudioFlinger_setMicMute
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, setMicMute, bool, status_t)

// AudioFlinger_getMicMute
RPC_GENERATE_SERVER_METHOD01(AF_, AudioFlinger, getMicMute, bool)

// AudioFlinger_setParameters
RpcResponse* AudioFlinger_setParameters(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()

    audio_io_handle_t ioHandle;
    request->getArg((char*) &ioHandle, sizeof(ioHandle));
    size_t len;
    request->getArg((char*) &len, sizeof(len));
    char keyValuePairsBuf[len];
    request->getArg(keyValuePairsBuf, len);
    String8 keyValuePairs;
    keyValuePairs.setTo(keyValuePairsBuf, len);
    
    sp<AudioFlinger> audioFlinger = *((sp<AudioFlinger>*) RpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = audioFlinger->setParameters(ioHandle, keyValuePairs);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// AudioFlinger_getParameters
RpcResponse* AudioFlinger_getParameters(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()

    audio_io_handle_t ioHandle;
    request->getArg((char*) &ioHandle, sizeof(ioHandle));
    size_t len;
    request->getArg((char*) &len, sizeof(len));
    char keysBuf[len];
    request->getArg(keysBuf, len);
    String8 keys;
    keys.setTo(keysBuf, len);
    
    sp<AudioFlinger> audioFlinger = *((sp<AudioFlinger>*) RpcUtilInst.idToObjMap[request->serviceId]);
    String8 result = audioFlinger->getParameters(ioHandle, keys);
    
    RpcResponse* response = new RpcResponse(true);
    len = result.length();
    response->putRet((char*) &len, sizeof(len));
    response->putRet((char*) result.string(), len);
    
    return response;
}

// AudioFlinger_getInputBufferSize
RPC_GENERATE_SERVER_METHOD31(AF_, AudioFlinger, getInputBufferSize, uint32_t, audio_format_t, audio_channel_mask_t, size_t)

// AudioFlinger_invalidateStream
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, invalidateStream, audio_stream_type_t, status_t)

// AudioFlinger_setVoiceVolume
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, setVoiceVolume, float, status_t)

// AudioFlinger_getRenderPosition
RpcResponse* AudioFlinger_getRenderPosition(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    uint32_t halFrames;
    request->getArg((char*) &halFrames, sizeof(halFrames));
    uint32_t dspFrames;
    request->getArg((char*) &dspFrames, sizeof(dspFrames));
    audio_io_handle_t output;
    request->getArg((char*) &output, sizeof(output));
    
    sp<AudioFlinger> audioFlinger = *((sp<AudioFlinger>*) RpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = audioFlinger->getRenderPosition(&halFrames, &dspFrames, output);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &halFrames, sizeof(halFrames));
    response->putRet((char*) &dspFrames, sizeof(dspFrames));
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// AudioFlinger_getInputFramesLost
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, getInputFramesLost, audio_io_handle_t, uint32_t)

// AudioFlinger_newAudioUniqueId
RPC_GENERATE_SERVER_METHOD01(AF_, AudioFlinger, newAudioUniqueId, audio_unique_id_t)

// AudioFlinger_acquireAudioSessionId
RPC_GENERATE_SERVER_METHOD20(AF_, AudioFlinger, acquireAudioSessionId, int, pid_t)

// AudioFlinger_releaseAudioSessionId
RPC_GENERATE_SERVER_METHOD20(AF_, AudioFlinger, releaseAudioSessionId, int, pid_t)

// AudioFlinger_getPrimaryOutputSamplingRate
RPC_GENERATE_SERVER_METHOD01(AF_, AudioFlinger, getPrimaryOutputSamplingRate, uint32_t)

// AudioFlinger_getPrimaryOutputFrameCount
RPC_GENERATE_SERVER_METHOD01(AF_, AudioFlinger, getPrimaryOutputFrameCount, size_t)

// AudioFlinger_setLowRamDevice
RPC_GENERATE_SERVER_METHOD11(AF_, AudioFlinger, setLowRamDevice, bool, status_t)

// ---------------------------------------------------------------------------
__attribute__ ((visibility ("default"))) void registerRpcSensorService() {
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_CREATE_TRACK, &AudioFlinger_createTrack);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SAMPLE_RATE, &Rpc_AF_sampleRate);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_FORMAT, &Rpc_AF_format);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_FRAME_COUNT, &Rpc_AF_frameCount);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_LATENCY, &Rpc_AF_latency);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MASTER_VOLUME, &Rpc_AF_setMasterVolume);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MASTER_MUTE, &Rpc_AF_setMasterMute);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_MASTER_VOLUME, &Rpc_AF_masterVolume);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_MASTER_MUTE, &Rpc_AF_masterMute);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_STREAM_VOLUME, &Rpc_AF_setStreamVolume);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_STREAM_MUTE, &Rpc_AF_setStreamMute);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_STREAM_VOLUME, &Rpc_AF_streamVolume);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_STREAM_MUTE, &Rpc_AF_streamMute);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MODE, &Rpc_AF_setMode);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_MIC_MUTE, &Rpc_AF_setMicMute);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_MIC_MUTE, &Rpc_AF_getMicMute);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_PARAMETERS, &AudioFlinger_setParameters);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_PARAMETERS, &AudioFlinger_getParameters);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_INPUT_BUFFER_SIZE, &Rpc_AF_getInputBufferSize);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_INVALIDATE_STREAM, &Rpc_AF_invalidateStream);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_VOICE_VOLUME, &Rpc_AF_setVoiceVolume);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_RENDER_POSITION, &AudioFlinger_getRenderPosition);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_INPUT_FRAMES_LOST, &Rpc_AF_getInputFramesLost);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_NEW_AUDIO_UNIQUE_ID, &Rpc_AF_newAudioUniqueId);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_ACQUIRE_AUDIO_SESSION_ID, &Rpc_AF_acquireAudioSessionId);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_RELEASE_AUDIO_SESSION_ID, &Rpc_AF_releaseAudioSessionId);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_PRIMARY_OUTPUT_SAMPLING_RATE, &Rpc_AF_getPrimaryOutputSamplingRate);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_GET_PRIMARY_OUTPUT_FRAME_COUNT, &Rpc_AF_getPrimaryOutputFrameCount);
    RpcUtilInst.rpcserver->registerFunc(RpcUtilInst.AUDIO_SERVICE_ID, AF_METH_SET_LOW_RAM_DEVICE, &Rpc_AF_setLowRamDevice);
}

}
