
#define LOG_TAG "RpcAudioPolicyService"

#include "RpcAudioPolicyServiceServer.h"

namespace android {

// ----------------------------------------------------------------------------
RpcResponse* AudioPolicyService_getOutput(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()

    audio_stream_type_t stream;
    request->getArg((char*) &stream, sizeof(stream));
    uint32_t samplingRate;
    request->getArg((char*) &samplingRate, sizeof(samplingRate));
    audio_format_t format;
    request->getArg((char*) &format, sizeof(format));
    audio_channel_mask_t channelMask;
    request->getArg((char*) &channelMask, sizeof(channelMask));
    audio_output_flags_t flags;
    request->getArg((char*) &flags, sizeof(flags));
    bool hasOffloadInfo;
    audio_offload_info_t offloadInfo;
    if (hasOffloadInfo) {
        request->getArg((char*) &offloadInfo, sizeof(offloadInfo));
    }
    
    sp<AudioPolicyService> audioPolicyService = *((sp<AudioPolicyService>*) AudioRpcUtilInst.idToObjMap[request->serviceId]);
    audio_io_handle_t result = audioPolicyService->getOutput(stream, samplingRate, format, channelMask, flags, hasOffloadInfo ? &offloadInfo : NULL);
    
    //ALOGE("rpc audio service the output is: %d", output);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

RpcResponse* AudioPolicyService_getOutputForAttr(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()

    bool hasAttributes;
    audio_attributes_t attr;
    request->getArg((char*) &hasAttributes, sizeof(hasAttributes));
    if (hasAttributes) {
        request->getArg((char*) &attr, sizeof(attr));
    }
    audio_io_handle_t output;
    audio_session_t session;
    request->getArg((char*) &session, sizeof(session));
    bool hasStream;
    audio_stream_type_t stream;
    request->getArg((char*) &hasStream, sizeof(hasStream));
    if (hasStream) {
        request->getArg((char*) &stream, sizeof(stream));
    }
    uint32_t samplingRate;
    request->getArg((char*) &samplingRate, sizeof(samplingRate));
    audio_format_t format;
    request->getArg((char*) &format, sizeof(format));
    audio_channel_mask_t channelMask;
    request->getArg((char*) &channelMask, sizeof(channelMask));
    audio_output_flags_t flags;
    request->getArg((char*) &flags, sizeof(flags));
    bool hasOffloadInfo;
    audio_offload_info_t offloadInfo;
    if (hasOffloadInfo) {
        request->getArg((char*) &offloadInfo, sizeof(offloadInfo));
    }
    
    sp<AudioPolicyService> audioPolicyService = *((sp<AudioPolicyService>*) AudioRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = audioPolicyService->getOutputForAttr(hasAttributes ? &attr : NULL, &output, session, hasStream ? &stream : NULL, samplingRate, format, channelMask, flags, hasOffloadInfo ? &offloadInfo : NULL);
    
    //ALOGE("rpc audio service the output is: %d", output);
    
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &output, sizeof(output));
    response->putRet((char*) &result, sizeof(result));
    if (hasStream) {
        response->putRet((char*) &stream, sizeof(stream));
    }
    
    return response;
}

// ---------------------------------------------------------------------------
__attribute__ ((visibility ("default"))) void registerRpcAudioPolicyServiceServer() {
    AudioRpcUtilInst.rpcserver->registerFunc(AudioRpcUtilInst.AUDIO_POLICY_SERVICE_ID, APS_METH_GET_OUTPUT, &AudioPolicyService_getOutput);
    AudioRpcUtilInst.rpcserver->registerFunc(AudioRpcUtilInst.AUDIO_POLICY_SERVICE_ID, APS_METH_GET_OUTPUT_FOR_ATTR, &AudioPolicyService_getOutputForAttr);
    
}

}; // namespace android
