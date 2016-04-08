
#define LOG_TAG "RpcAudioPolicyService"

#include "RpcAudioPolicyService.h"

namespace android {

// ----------------------------------------------------------------------------
RpcAudioPolicyService::RpcAudioPolicyService() {}
RpcAudioPolicyService::~RpcAudioPolicyService() {}

audio_io_handle_t RpcAudioPolicyService::getOutput(audio_stream_type_t stream,
                                        uint32_t samplingRate,
                                        audio_format_t format,
                                        audio_channel_mask_t channelMask,
                                        audio_output_flags_t flags,
                                        const audio_offload_info_t *offloadInfo) 
{ 
    RPC_CLIENT_REQUEST_COMMON(AudioRpcUtilInst.AUDIO_POLICY_SERVICE_ID, APS_METH_GET_OUTPUT)
    request->putArg((char*) &stream, sizeof(stream));
    request->putArg((char*) &samplingRate, sizeof(samplingRate));
    request->putArg((char*) &format, sizeof(format));
    request->putArg((char*) &channelMask, sizeof(channelMask));
    request->putArg((char*) &flags, sizeof(flags));
    bool flag = 0;
    if (offloadInfo == NULL) {
        flag = 0;
        request->putArg((char*) &flag, sizeof(flag));
    } else {
        flag = 1;
        request->putArg((char*) &flag, sizeof(flag));
        request->putArg((char*) offloadInfo, sizeof(audio_offload_info_t));
    }

    RpcResponse* response = AudioRpcUtilInst.rpcclient->doRpc(request);
    audio_io_handle_t result;
    response->getRet((char*) &result, sizeof(result));
    
    delete response;
    
    return result;    
}

status_t RpcAudioPolicyService::getOutputForAttr(const audio_attributes_t *attr,
                                      audio_io_handle_t *output,
                                      audio_session_t session,
                                      audio_stream_type_t *stream,
                                      uint32_t samplingRate,
                                      audio_format_t format,
                                      audio_channel_mask_t channelMask,
                                      audio_output_flags_t flags,
                                      const audio_offload_info_t *offloadInfo)
{
    RPC_CLIENT_REQUEST_COMMON(AudioRpcUtilInst.AUDIO_POLICY_SERVICE_ID, APS_METH_GET_OUTPUT_FOR_ATTR)
    bool flag = 0;
    if (attr == NULL) {
        flag = 0;
        request->putArg((char*) &flag, sizeof(flag));
    } else {
        flag = 1;
        request->putArg((char*) &flag, sizeof(flag));
        request->putArg((char*) attr, sizeof(audio_attributes_t));
    }
    request->putArg((char*) &session, sizeof(session));  
    if (stream == NULL) {
        flag = 0;
        request->putArg((char*) &flag, sizeof(flag));
    } else {
        flag = 1;
        request->putArg((char*) &flag, sizeof(flag));
        request->putArg((char*) stream, sizeof(audio_stream_type_t));
    }         
    request->putArg((char*) &samplingRate, sizeof(samplingRate));
    request->putArg((char*) &format, sizeof(format));
    request->putArg((char*) &channelMask, sizeof(channelMask));
    request->putArg((char*) &flags, sizeof(flags));
    if (offloadInfo == NULL) {
        flag = 0;
        request->putArg((char*) &flag, sizeof(flag));
    } else {
        flag = 1;
        request->putArg((char*) &flag, sizeof(flag));
        request->putArg((char*) offloadInfo, sizeof(audio_offload_info_t));
    }

    RpcResponse* response = AudioRpcUtilInst.rpcclient->doRpc(request);
    response->getRet((char*) output, sizeof(audio_io_handle_t));
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    if (stream != NULL) {
        response->getRet((char*) stream, sizeof(audio_stream_type_t));
    }
    
    delete response;
    
    return result;    
}

}; // namespace android
