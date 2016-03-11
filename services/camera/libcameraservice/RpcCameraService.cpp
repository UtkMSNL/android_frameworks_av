
#include "RpcCameraService.h"
#include "api1/RpcCameraClientProxy.h"
#include <camera/RpcICameraClientServer.h>
#include "system/camera_metadata.h"
#include <camera/CameraMetadata.h>

namespace android {
    
// ----------------------------------------------------------------------------
//      RPC Camera in the server
// ----------------------------------------------------------------------------

RpcCameraService::RpcCameraService() {}
RpcCameraService::~RpcCameraService() {}
    
int32_t RpcCameraService::getNumberOfCameras()
{
    RPC_CLIENT_FUNC01(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_NUMBER_OF_CAMERAS, int32_t)
}

status_t RpcCameraService::getCameraInfo(int cameraId, struct CameraInfo* cameraInfo)
{
    RPC_CLIENT_REQUEST_COMMON(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_CAMERA_INFO)
    
    request->putArg((char*) &cameraId, sizeof(cameraId));
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    response->getRet((char*) &cameraInfo->facing, sizeof(cameraInfo->facing));
    response->getRet((char*) &cameraInfo->orientation, sizeof(cameraInfo->orientation));
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    
    delete response;
    
    return result;
}

status_t RpcCameraService::getCameraCharacteristics(int cameraId, CameraMetadata* cameraInfo)
{
    if (cameraInfo == NULL) {
        return 0;
    }
    RPC_CLIENT_REQUEST_COMMON(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_CAMERA_CHARACTERISTICS)
    request->putArg((char*) &cameraId, sizeof(cameraId));
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    int hasMetadata;
    response->getRet((char*) &hasMetadata, sizeof(hasMetadata));
    if (!hasMetadata) {
        delete response;
        return result;
    }
    camera_metadata_t* metadata = NULL;

    int32_t blobSizeTmp = -1;
    response->getRet((char*) &blobSizeTmp, sizeof(blobSizeTmp));
    const size_t blobSize = static_cast<size_t>(blobSizeTmp);
    const size_t alignment = get_camera_metadata_alignment();

    // Special case: zero blob size means zero sized (NULL) metadata.
    if (blobSize == 0) {
        delete response;
        return result;
    }

    const size_t metadataSize = blobSize - alignment;

    do {
        char* blob = new char[blobSize];
        response->getRet(blob, blobSize);

        int32_t offsetTmp;
        response->getRet((char*) &offsetTmp, sizeof(offsetTmp));
        const size_t offset = static_cast<size_t>(offsetTmp);

        const uintptr_t metadataStart = reinterpret_cast<uintptr_t>(blob) + offset;
        const camera_metadata_t* tmp =
                       reinterpret_cast<const camera_metadata_t*>(metadataStart);
        metadata = allocate_copy_camera_metadata_checked(tmp, metadataSize);
        delete blob;
    } while(0);

    cameraInfo->clear();
    CameraMetadata out(metadata);
    cameraInfo->swap(out);
    
    return result;
}

status_t RpcCameraService::getCameraVendorTagDescriptor(sp<VendorTagDescriptor>& desc)
{
    RPC_CLIENT_REQUEST_COMMON(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_CAMERA_VENDOR_TAG_DESCRIPTOR)
    
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    int hasTag;
    response->getRet((char*) &hasTag, sizeof(hasTag));
    if (!hasTag) {
        delete response;
        return result;
    }
    int32_t tagCount = 0;
    response->getRet((char*) &tagCount, sizeof(tagCount));
    
    desc = new VendorTagDescriptor();
    desc->mTagCount = tagCount;

    uint32_t tag, sectionIndex;
    uint32_t maxSectionIndex = 0;
    int32_t tagType;
    Vector<uint32_t> allTags;
    for (int32_t i = 0; i < tagCount; ++i) {
        response->getRet((char*) &tag, sizeof(tag));
        response->getRet((char*) &tagType, sizeof(tagType));
        size_t len;
        response->getRet((char*) &len, sizeof(len));
        char buf[len];
        response->getRet((char*) buf, len);
        String8 tagName(buf, len);
        response->getRet((char*) &sectionIndex, sizeof(sectionIndex));

        maxSectionIndex = (maxSectionIndex >= sectionIndex) ? maxSectionIndex : sectionIndex;

        allTags.add(tag);
        desc->mTagToNameMap.add(tag, tagName);
        desc->mTagToSectionMap.add(tag, sectionIndex);
        desc->mTagToTypeMap.add(tag, tagType);
    }
    
    size_t sectionCount = 0;
    response->getRet((char*) &sectionCount, sizeof(sectionCount));
    for (size_t i = 0; i < sectionCount; ++i) {
        size_t len;
        response->getRet((char*) &len, sizeof(len));
        char buf[len];
        response->getRet((char*) buf, len);
        String8 sectionName(buf, len);
        desc->mSections.add(sectionName);
    }

    // Set up reverse mapping
    for (size_t i = 0; i < static_cast<size_t>(tagCount); ++i) {
        uint32_t tag = allTags[i];
        String8 sectionString = desc->mSections[desc->mTagToSectionMap.valueFor(tag)];

        ssize_t reverseIndex = -1;
        if ((reverseIndex = desc->mReverseMapping.indexOfKey(sectionString)) < 0) {
            KeyedVector<String8, uint32_t>* nameMapper = new KeyedVector<String8, uint32_t>();
            reverseIndex = desc->mReverseMapping.add(sectionString, nameMapper);
        }
        desc->mReverseMapping[reverseIndex]->add(desc->mTagToNameMap.valueFor(tag), tag);
    }

    delete response;
    return result;
}

status_t RpcCameraService::addListener(const sp<ICameraServiceListener>& listener)
{
    // FIXME: implement this listener 
    return 0;
}

status_t RpcCameraService::removeListener(const sp<ICameraServiceListener>& listener)
{
    // FIXME: implement this listener
    return 0;
}

status_t RpcCameraService::connect(const sp<ICameraClient>& cameraClient, int cameraId,
    const String16& clientPackageName, int clientUid, sp<ICamera>& device)
{
    RPC_CLIENT_REQUEST_COMMON(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_CONNECT)
    int cameraClientId = registerICameraClientForRpc(cameraClient);
    request->putArg((char*) &cameraClientId, sizeof(cameraClientId));
    request->putArg((char*) &cameraId, sizeof(cameraId));
    size_t len = clientPackageName.size();
    request->putArg((char*) &len, sizeof(len));
    request->putArg((char*) clientPackageName.string(), len * sizeof(char16_t));
    request->putArg((char*) &clientUid, sizeof(clientUid));
    
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    int remoteCameraId;
    response->getRet((char*) &remoteCameraId, sizeof(remoteCameraId));
    device = new RpcCameraClientProxy(remoteCameraId);
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    
    delete response;
    
    return result;
}

status_t RpcCameraService::connectPro(const sp<IProCameraCallbacks>& cameraCb,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<IProCameraUser>& device) { return 0; }

status_t RpcCameraService::connectDevice(
            const sp<ICameraDeviceCallbacks>& cameraCb,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICameraDeviceUser>& device) { return 0; }

status_t RpcCameraService::getLegacyParameters(int cameraId, String16* parameters)
{
    RPC_CLIENT_REQUEST_COMMON(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_GET_LEGACY_PARAMETERS)
    
    request->putArg((char*) &cameraId, sizeof(cameraId));
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    size_t len;
    response->getRet((char*) &len, sizeof(len));
    char paramBuf[len * sizeof(char16_t)];
    response->getRet(paramBuf, len * sizeof(char16_t));
    *parameters = String16(paramBuf, len);
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    
    delete response;
    
    return result;
}

status_t RpcCameraService::supportsCameraApi(int cameraId, int apiVersion)
{
    RPC_CLIENT_FUNC21(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_SUPPORTS_CAMERA_API, cameraId, apiVersion, status_t)
}

status_t RpcCameraService::connectLegacy(const sp<ICameraClient>& cameraClient, int cameraId, 
    int halVersion, const String16& clientPackageName, int clientUid, sp<ICamera>& device)
{
    RPC_CLIENT_REQUEST_COMMON(CameraRpcUtilInst.CAMERA_SERVICE_ID, CS_METH_CONNECT_LEGACY)
    int cameraClientId = registerICameraClientForRpc(cameraClient);
    request->putArg((char*) &cameraClientId, sizeof(cameraClientId));
    request->putArg((char*) &cameraId, sizeof(cameraId));
    request->putArg((char*) &halVersion, sizeof(halVersion));
    size_t len = clientPackageName.size();
    request->putArg((char*) &len, sizeof(len));
    request->putArg((char*) clientPackageName.string(), len * sizeof(char16_t));
    request->putArg((char*) &clientUid, sizeof(clientUid));
    
    RpcResponse* response = CameraRpcUtilInst.rpcclient->doRpc(request);
    int remoteCameraId;
    response->getRet((char*) &remoteCameraId, sizeof(remoteCameraId));
    device = new RpcCameraClientProxy(remoteCameraId);
    status_t result;
    response->getRet((char*) &result, sizeof(result));
    
    delete response;
    
    return result;
}

// ---------------------------------------------------------------------------
__attribute__ ((visibility ("default"))) void registerRpcCameraService() {
    registerRpcCameraClientProxy();
}

}
