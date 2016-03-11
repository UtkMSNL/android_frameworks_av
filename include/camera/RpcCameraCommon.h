#ifndef ANDROID_RPC_CAMERA_COMMON_H
#define ANDROID_RPC_CAMERA_COMMON_H

// method id for RpcCameraService
#define CS_METH_GET_NUMBER_OF_CAMERAS 1
#define CS_METH_GET_CAMERA_INFO 2
#define CS_METH_GET_CAMERA_CHARACTERISTICS 3
#define CS_METH_GET_CAMERA_VENDOR_TAG_DESCRIPTOR 4
#define CS_METH_ADD_LISTENER 5
#define CS_METH_REMOVE_LISTENER 6
#define CS_METH_CONNECT 7
#define CS_METH_CONNECT_PRO 8
#define CS_METH_CONNECT_DEVICE 9
#define CS_METH_GET_LEGACY_PARAMETERS 10
#define CS_METH_SUPPORTS_CAMERA_API 11
#define CS_METH_CONNECT_LEGACY 12

// method id for CameraClient
#define CC_METH_REFRESH_PREVIEW_WINDOW 1
#define CC_METH_DISCONNECT 2
#define CC_METH_CONNECT 3
#define CC_METH_LOCK 4
#define CC_METH_UNLOCK 5
#define CC_METH_SET_PREVIEW_TARGET 6
#define CC_METH_SET_PREVIEW_CALLBACK_FLAG 7
#define CC_METH_SET_PREVIEW_CALLBACK_TARGET 8
#define CC_METH_START_PREVIEW 9
#define CC_METH_STOP_PREVIEW 10
#define CC_METH_PREVIEW_ENABLED 11
#define CC_METH_STORE_METADATA_IN_BUFFERS 12
#define CC_METH_START_RECORDING 13
#define CC_METH_STOP_RECORDING 14
#define CC_METH_RECORDING_ENABLED 15
#define CC_METH_RELEASE_RECORDING_FRAME 16
#define CC_METH_AUTO_FOCUS 17
#define CC_METH_CANCEL_AUTO_FOCUS 18
#define CC_METH_TAKE_PICTURE 19
#define CC_METH_SET_PARAMETERS 20
#define CC_METH_GET_PARAMETERS 21
#define CC_METH_SEND_COMMAND 22

// method id for RpcICameraClientProxy
#define CCP_METH_NOTIFY_CALLBACK 1
#define CCP_METH_DATA_CALLBACK 2
#define CCP_METH_DATA_CALLBACK_TIMESTAMP 3

namespace android {

static void restoreCameraToken(uid_t uid, pid_t pid) {
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
    restoreCameraToken(uidval, pidval);

#define RPC_SERVER_REQUEST_GETSRVOBJ(_serviceType)                          \
    RPC_SERVER_REQUEST_COMMON() \
    /* this serviceObj variable will be used by later method invocation */  \
    sp<_serviceType> serviceObj = *((sp<_serviceType>*) CameraRpcUtilInst.idToObjMap[request->serviceId]); 
    
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
            _methId, CameraRpcUtilInst.rpcclient->socketFd, true);            \
    uid_t uidval = IPCThreadState::self()->getCallingUid();             \
    pid_t pidval = IPCThreadState::self()->getCallingPid();             \
    request->putArg((char*) &uidval, sizeof(uidval));                      \
    request->putArg((char*) &pidval, sizeof(pidval));
    
#define RPC_CLIENT_RESPONSE_NORET()                                     \
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);      \
    delete response; 
    
#define RPC_CLIENT_RESPONSE_RET(_retType)                               \
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);      \
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
