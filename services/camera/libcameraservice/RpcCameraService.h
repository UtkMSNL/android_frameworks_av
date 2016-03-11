#ifndef ANDROID_RPC_CAMERA_SERVICE_H
#define ANDROID_RPC_CAMERA_SERVICE_H

#include "CameraService.h"
#include <camera/RpcCameraCommon.h>
#include <rpc/share_rpc.h>

namespace android {

// ----------------------------------------------------------------------------

class RpcCameraService :
    public BinderService<RpcCameraService>,
    public BnCameraService,
    public IBinder::DeathRecipient,
    public camera_module_callbacks_t
{
    friend class BinderService<RpcCameraService>;   // for RpcAudioFlinger()
public:
    static const char* getServiceName() ANDROID_API { return "media.rpc_camera"; }
    
    virtual int32_t  getNumberOfCameras();
    virtual status_t getCameraInfo(int cameraId,
            /*out*/
            struct CameraInfo* cameraInfo);

    virtual status_t getCameraCharacteristics(int cameraId,
            /*out*/
            CameraMetadata* cameraInfo);

    virtual status_t getCameraVendorTagDescriptor(
            /*out*/
            sp<VendorTagDescriptor>& desc);

    // Returns 'OK' if operation succeeded
    // - Errors: ALREADY_EXISTS if the listener was already added
    virtual status_t addListener(const sp<ICameraServiceListener>& listener)
                                                                           ;
    // Returns 'OK' if operation succeeded
    // - Errors: BAD_VALUE if specified listener was not in the listener list
    virtual status_t removeListener(const sp<ICameraServiceListener>& listener)
                                                                           ;
    /**
     * clientPackageName and clientUid are used for permissions checking.  if
     * clientUid == USE_CALLING_UID, then the calling UID is used instead. Only
     * trusted callers can set a clientUid other than USE_CALLING_UID.
     */
    virtual status_t connect(const sp<ICameraClient>& cameraClient,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICamera>& device);

    virtual status_t connectPro(const sp<IProCameraCallbacks>& cameraCb,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<IProCameraUser>& device);

    virtual status_t connectDevice(
            const sp<ICameraDeviceCallbacks>& cameraCb,
            int cameraId,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICameraDeviceUser>& device);

    virtual status_t getLegacyParameters(
            int cameraId,
            /*out*/
            String16* parameters);

    /**
     * Returns OK if device supports camera2 api,
     * returns -EOPNOTSUPP if it doesn't.
     */
    virtual status_t supportsCameraApi(
            int cameraId, int apiVersion);

    /**
     * Connect the device as a legacy device for a given HAL version.
     * For halVersion, use CAMERA_API_DEVICE_VERSION_* for a particular
     * version, or CAMERA_HAL_API_VERSION_UNSPECIFIED for a service-selected version.
     */
    virtual status_t connectLegacy(const sp<ICameraClient>& cameraClient,
            int cameraId, int halVersion,
            const String16& clientPackageName,
            int clientUid,
            /*out*/
            sp<ICamera>& device);
            
    void binderDied(const wp<IBinder>& who) {}

private:            
    RpcCameraService() ANDROID_API;
    virtual ~RpcCameraService();
};
// ---------------------------------------------------------------------------
void registerRpcCameraService();

}

#endif
