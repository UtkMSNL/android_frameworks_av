
#include "RpcCameraServiceServer.h"
#include "api1/RpcCameraClientServer.h"
#include <camera/RpcICameraClientProxy.h>
#include <camera/ICameraService.h>
#include "system/camera_metadata.h"
#include <camera/CameraMetadata.h>

namespace android {
    
// ----------------------------------------------------------------------------
//      RPC Camera in the server
// ----------------------------------------------------------------------------

// CameraService_getNumberOfCameras
RPC_GENERATE_SERVER_METHOD01(CS_, ICameraService, getNumberOfCameras, int32_t)

// CameraService_getCameraInfo
RpcResponse* CameraService_getCameraInfo(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    int cameraId;
    request->getArg((char*) &cameraId, sizeof(cameraId));
    CameraInfo cameraInfo = CameraInfo();
    memset(&cameraInfo, 0, sizeof(cameraInfo));
    sp<ICameraService> cameraService = *((sp<ICameraService>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = cameraService->getCameraInfo(cameraId, &cameraInfo);
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &cameraInfo.facing, sizeof(cameraInfo.facing));
    response->putRet((char*) &cameraInfo.orientation, sizeof(cameraInfo.orientation));
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

#define ALIGN_TO(val, alignment) \
    (((uintptr_t)(val) + ((alignment) - 1)) & ~((alignment) - 1))
// CameraService_getCameraCharacteristics
RpcResponse* CameraService_getCameraCharacteristics(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    int cameraId;
    request->getArg((char*) &cameraId, sizeof(cameraId));
    CameraMetadata info;
    sp<ICameraService> cameraService = *((sp<ICameraService>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = cameraService->getCameraCharacteristics(cameraId, &info);
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    
    const camera_metadata_t* metadata = info.getAndLock();
    if (metadata == NULL) {
        int hasMetadata = 0;
        response->putRet((char*) &hasMetadata, sizeof(hasMetadata));
        return response;
    }

    const size_t metadataSize = get_camera_metadata_compact_size(metadata);
    const size_t alignment = get_camera_metadata_alignment();
    const size_t blobSize = metadataSize + alignment;
    response->putRet((char*) &blobSize, sizeof(blobSize));

    size_t offset = 0;
    do {
        char* blob = new char[blobSize];
        const uintptr_t metadataStart = ALIGN_TO(blob, alignment);
        offset = metadataStart - reinterpret_cast<uintptr_t>(blob);
        copy_camera_metadata(reinterpret_cast<void*>(metadataStart), metadataSize, metadata);

        response->putRet((char*) blob, blobSize);
        delete blob;
    } while(false);

    response->putRet((char*) &offset, sizeof(offset));
    info.unlock(metadata);
    return response;
}

// CameraService_getCameraVendorTagDescriptor
RpcResponse* CameraService_getCameraVendorTagDescriptor(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    sp<VendorTagDescriptor> tagDesc;
    sp<ICameraService> cameraService = *((sp<ICameraService>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = cameraService->getCameraVendorTagDescriptor(tagDesc);
    RpcResponse* response = new RpcResponse(true);
    response->putRet((char*) &result, sizeof(result));
    int hasTag;
    if (tagDesc == NULL) {
        hasTag = 0;
        response->putRet((char*) &hasTag, sizeof(hasTag));
    } else {
        hasTag = 1;
        response->putRet((char*) &hasTag, sizeof(hasTag));
        
        response->putRet((char*) &tagDesc->mTagCount, sizeof(tagDesc->mTagCount));
        size_t size = tagDesc->mTagToNameMap.size();
        uint32_t tag, sectionIndex;
        int32_t tagType;
        for (size_t i = 0; i < size; ++i) {
            tag = tagDesc->mTagToNameMap.keyAt(i);
            String8 tagName = tagDesc->mTagToNameMap[i];
            sectionIndex = tagDesc->mTagToSectionMap.valueFor(tag);
            tagType = tagDesc->mTagToTypeMap.valueFor(tag);
            response->putRet((char*) &tag, sizeof(tag));
            response->putRet((char*) &tagType, sizeof(tagType));
            size_t len = tagName.size();
            response->putRet((char*) &len, sizeof(len));
            response->putRet((char*) tagName.string(), len);
            response->putRet((char*) &sectionIndex, sizeof(sectionIndex));
        }

        size_t numSections = tagDesc->mSections.size();
        response->putRet((char*) &numSections, sizeof(numSections));
        if (numSections > 0) {
            for (size_t i = 0; i < numSections; ++i) {
                size_t len = tagDesc->mSections[i].size();
                response->putRet((char*) &len, sizeof(len));
                response->putRet((char*) tagDesc->mSections[i].string(), len);
            }
        }
    }
    return response;
}

// CameraService_addListener
RpcResponse* CameraService_addListener(RpcRequest* request) {
    // TODO:
    return NULL;
}

// CameraService_removeListener
RpcResponse* CameraService_removeListener(RpcRequest* request) {
    // TODO:
    return NULL;
}

// CameraService_connect
RpcResponse* CameraService_connect(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    int cameraClientId;
    request->getArg((char*) &cameraClientId, sizeof(cameraClientId));
    int cameraId;
    request->getArg((char*) &cameraId, sizeof(cameraId));
    size_t len;
    request->getArg((char*) &len, sizeof(len));
    char nameBuf[len * sizeof(char16_t)];
    request->getArg(nameBuf, len * sizeof(char16_t));
    String16 clientPackageName(nameBuf, len);
    int clientUid;
    request->getArg((char*) &clientUid, sizeof(clientUid));
    sp<ICameraClient> cameraClient(new RpcICameraClientProxy(cameraClientId));
    sp<ICameraService> cameraService = *((sp<ICameraService>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    sp<ICamera> device;
    status_t result = cameraService->connect(cameraClient, cameraId, clientPackageName, clientUid, device);
    
    RpcResponse* response = new RpcResponse(true);
    int cameraSrvId = registerCameraClientForRpc(device);
    response->putRet((char*) &cameraSrvId, sizeof(cameraSrvId));
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// CameraService_connectPro
RpcResponse* CameraService_connectPro(RpcRequest* request) {
    // TODO:
    return NULL;
}

// CameraService_connectDevice
RpcResponse* CameraService_connectDevice(RpcRequest* request) {
    // TODO:
    return NULL;
}

// CameraService_getLegacyParameters
RpcResponse* CameraService_getLegacyParameters(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    int cameraId;
    request->getArg((char*) &cameraId, sizeof(cameraId));
    String16 parameters;
    sp<ICameraService> cameraService = *((sp<ICameraService>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = cameraService->getLegacyParameters(cameraId, &parameters);
    RpcResponse* response = new RpcResponse(true);
    size_t len = parameters.size();
    response->putRet((char*) &len, sizeof(len));
    response->putRet((char*) parameters.string(), len * sizeof(char16_t));
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// CameraService_supportsCameraApi
RPC_GENERATE_SERVER_METHOD21(CS_, ICameraService, supportsCameraApi, int, int, status_t)

// CameraService_connectLegacy
RpcResponse* CameraService_connectLegacy(RpcRequest* request) {
    RPC_SERVER_REQUEST_COMMON()
    
    int cameraClientId;
    request->getArg((char*) &cameraClientId, sizeof(cameraClientId));
    int cameraId;
    request->getArg((char*) &cameraId, sizeof(cameraId));
    int halVersion;
    request->getArg((char*) &halVersion, sizeof(halVersion));
    size_t len;
    request->getArg((char*) &len, sizeof(len));
    char nameBuf[len * sizeof(char16_t)];
    request->getArg(nameBuf, len * sizeof(char16_t));
    String16 clientPackageName(nameBuf, len);
    int clientUid;
    request->getArg((char*) &clientUid, sizeof(clientUid));
    sp<ICameraClient> cameraClient(new RpcICameraClientProxy(cameraClientId));
    sp<ICamera> device;
    sp<ICameraService> cameraService = *((sp<ICameraService>*) CameraRpcUtilInst.idToObjMap[request->serviceId]);
    status_t result = cameraService->connectLegacy(cameraClient, cameraId, halVersion, clientPackageName, clientUid, device);
    
    RpcResponse* response = new RpcResponse(true);
    int cameraSrvId = registerCameraClientForRpc(device);
    response->putRet((char*) &cameraSrvId, sizeof(cameraSrvId));
    response->putRet((char*) &result, sizeof(result));
    
    return response;
}

// ---------------------------------------------------------------------------
__attribute__ ((visibility ("default"))) void registerRpcCameraServiceServer()
{
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_NUMBER_OF_CAMERAS, &Rpc_CS_getNumberOfCameras);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_CAMERA_INFO, &CameraService_getCameraInfo);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_CAMERA_CHARACTERISTICS, &CameraService_getCameraCharacteristics);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_CAMERA_VENDOR_TAG_DESCRIPTOR, &CameraService_getCameraVendorTagDescriptor);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_ADD_LISTENER, &CameraService_addListener);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_REMOVE_LISTENER, &CameraService_removeListener);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_CONNECT, &CameraService_connect);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_CONNECT_PRO, &CameraService_connectPro);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_CONNECT_DEVICE, &CameraService_connectDevice);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_LEGACY_PARAMETERS, &CameraService_getLegacyParameters);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_SUPPORTS_CAMERA_API, &Rpc_CS_supportsCameraApi);
    CameraRpcUtilInst.rpcserver->registerFunc(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_CONNECT_LEGACY, &CameraService_connectLegacy);
}

}
