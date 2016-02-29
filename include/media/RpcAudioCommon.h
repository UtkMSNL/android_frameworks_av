#ifndef ANDROID_RPC_AUDIO_COMMON_H
#define ANDROID_RPC_AUDIO_COMMON_H

// method id for audio flinger
#define AF_METH_CREATE_TRACK 1
#define AF_METH_SAMPLE_RATE 2
#define AF_METH_FORMAT 3
#define AF_METH_FRAME_COUNT 4
#define AF_METH_LATENCY 5
#define AF_METH_SET_MASTER_VOLUME 6
#define AF_METH_SET_MASTER_MUTE 7
#define AF_METH_MASTER_VOLUME 8
#define AF_METH_MASTER_MUTE 9
#define AF_METH_SET_STREAM_VOLUME 10
#define AF_METH_SET_STREAM_MUTE 11
#define AF_METH_STREAM_VOLUME 12
#define AF_METH_STREAM_MUTE 13
#define AF_METH_SET_MODE 14
#define AF_METH_SET_MIC_MUTE 15
#define AF_METH_GET_MIC_MUTE 16
#define AF_METH_SET_PARAMETERS 17
#define AF_METH_GET_PARAMETERS 18
#define AF_METH_GET_INPUT_BUFFER_SIZE 19
#define AF_METH_INVALIDATE_STREAM 20
#define AF_METH_SET_VOICE_VOLUME 21
#define AF_METH_GET_INPUT_FRAMES_LOST 22
#define AF_METH_NEW_AUDIO_UNIQUE_ID 23
#define AF_METH_ACQUIRE_AUDIO_SESSION_ID 24
#define AF_METH_RELEASE_AUDIO_SESSION_ID 25
#define AF_METH_GET_PRIMARY_OUTPUT_SAMPLING_RATE 26
#define AF_METH_GET_PRIMARY_OUTPUT_FRAME_COUNT 27
#define AF_METH_SET_LOW_RAM_DEVICE 28
#define AF_METH_GET_RENDER_POSITION 29

// method id for track handle
#define TH_METH_DESTROY 1
#define TH_METH_START 2
#define TH_METH_STOP 3
#define TH_METH_FLUSH 4
#define TH_METH_PAUSE 5
#define TH_METH_ATTACH_AUX_EFFECT 6
#define TH_METH_SET_PARAMETERS 7
#define TH_METH_SIGNAL 8
#define TH_METH_SETUP_RPC_BUFFER_SYNC 9

// method id for AudioPolicyService
#define APS_METH_GET_OUTPUT_FOR_ATTR 1

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
    sp<_serviceType> serviceObj = *((sp<_serviceType>*) AudioRpcUtilInst.idToObjMap[request->serviceId]); 
    
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
    
// -------------------------------------------------------------------------

#define RPC_CLIENT_REQUEST_COMMON(_serviceId, _methId)                  \
    RpcRequest* request = new RpcRequest(_serviceId,                    \
            _methId, AudioRpcUtilInst.rpcclient->socketFd, true);            \
    uid_t uidval = IPCThreadState::self()->getCallingUid();             \
    pid_t pidval = IPCThreadState::self()->getCallingPid();             \
    request->putArg((char*) &uidval, sizeof(uidval));                      \
    request->putArg((char*) &pidval, sizeof(pidval));
    
#define RPC_CLIENT_RESPONSE_NORET()                                     \
    RpcResponse* response = AudioRpcUtilInst.rpcclient->doRpc(request);      \
    delete response; 
    
#define RPC_CLIENT_RESPONSE_RET(_retType)                               \
    RpcResponse* response = AudioRpcUtilInst.rpcclient->doRpc(request);      \
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

};

#endif
