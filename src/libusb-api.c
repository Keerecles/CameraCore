#include "cameradae.h"
#include "libusb_api.h"



int CAMERACORE_libusb_init();
int CAMERACORE_libusb_HotplugDeviceArrivedCallback();
int CAMERACORE_libusb_DeviceArrived();
int CAMERACORE_libusb_HotplugDeviceLifedCallback();
int CAMERACORE_libusb_getUsbDevice();
int CAMERACORE_libusb_UpdateDeviceList();
int CAMERACORE_libusb_DeviceConnect();
int CAMERACORE_libusb_DataFeed();
int CAMERACORE_libusb_transfer();
int CAMERACORE_libusb_transferCallback();
libusb_hotplug_callback_handle CAMERACORE_libusb_HotplugArrivedCallbackHandle();
libusb_hotplug_callback_handle CAMERACORE_libusb_HotplugLeftCallbackHandle();
int CAMERACORE_libusb_OnDeviceArrived();
int CAMERACORE_libusb_OnDeviceListUpdated();
int CAMERACORE_libusb_DeviceHandle(struct Device& device);
int CAMERACORE_libusb_GoogleDeviceHandle(struct Device& device);
int CAMERACORE_libusb_AppleDeviceHandle(struct Device& device);
int CAMERACORE_libusb_OnDeviceListUpdated();




bool IsGoogleAccessory(const PlatformUsbDevice* device);
bool IsAppleDevice(const PlatformUsbDevice* device);





libusb_context* libusb_context_;
libusb_device_handle* device_handle_libusb;
struct Device device;

int CAMERACORE_libusb_init(){
  
  int libusb_ret = libusb_init(&libusb_context_);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_init failed: ");
    CAMERACORE_log(fp, libusb_ret);
    CAMERACORE_log(fp, "]");
    return -1;
  }

  if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    CAMERACORE_log(fp, "LIBUSB_CAP_HAS_HOTPLUG not supported");
    return -1
  }

  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_, LIBUSB_HOTPLUG_EVENT_DeviceArrived,
                 LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, CAMERACORE_libusb_HotplugDeviceArrivedCallback, NULL,
                 &CAMERACORE_libusb_HotplugArrivedCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_hotplug_register_callback failed: ");
    CAMERACORE_log(fp,libusb_ret);
    CAMERACORE_log(fp, "]");
    return -1;
  }

  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                 static_cast<libusb_hotplug_flag>(0), LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, CAMERACORE_libusb_HotplugDeviceLifedCallback, NULL,
                 &CAMERACORE_libusb_HotplugLeftCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_hotplug_register_callback failed: ");
    CAMERACORE_log(fp,libusb_ret);
    CAMERACORE_log(fp, "]");
    return -1;
  }
}


int CAMERACORE_libusb_HotplugDeviceArrivedCallback(  libusb_context* context, libusb_device* device,
                                                  libusb_hotplug_event event, void* data) {
  CAMERACORE_log(fp, "[CAMERACORE_log]:CAMERACORE_libusb_HotplugDeviceArrivedCallback [libusb_hotplug_register_callback failed: ")
  /**********
  数据 参数转换    


  *************/

  CAMERACORE_libusb_DeviceArrived();


  return 0;
  }

}


int CAMERACORE_libusb_HotplugDeviceLifedCallback(  libusb_context* context, libusb_device* device,
                                                libusb_hotplug_event event, void* data){

CAMERACORE_log(fp, "[CAMERACORE_log]:CAMERACORE_libusb_HotplugDeviceLifedCallback [libusb_hotplug_register_callback failed: ")
  /**********
  数据 参数转换    


  *************/
CAMERACORE_libusb_device_lifed();

}


int CAMERACORE_libusb_DeviceArrived(libusb_device* device_libusb){
  LOG4CXX_TRACE(logger_, "enter. libusb_device* " << device_libusb);
  
  
  libusb_device_descriptor descriptor;
  int libusb_ret = libusb_get_device_descriptor(device_libusb, &descriptor);
  if (LIBUSB_SUCCESS != libusb_ret) {
    LOG4CXX_ERROR(logger_, "libusb_get_device_descriptor failed: " << libusb_ret);
    LOG4CXX_TRACE(logger_, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
  return;
  }
  libusb_ret = libusb_open(device_libusb, &device_handle_libusb);
  if (libusb_ret != LIBUSB_SUCCESS) {
    LOG4CXX_ERROR(logger_, "libusb_open failed: " << libusb_error_name(libusb_ret));
    LOG4CXX_TRACE(logger_, "exit. Condition: libusb_ret != LIBUSB_SUCCESS");
    return;
  }

  int configuration;
  libusb_ret = libusb_get_configuration(device_handle_libusb, &configuration);
  if (LIBUSB_SUCCESS != libusb_ret) {
    LOG4CXX_INFO(logger_, "libusb_get_configuration failed: " << libusb_error_name(
                   libusb_ret));
    LOG4CXX_TRACE(logger_, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
    return;
  }

  if (configuration != kUsbConfiguration) {
    libusb_ret = libusb_set_configuration(device_handle_libusb, kUsbConfiguration);
    if (LIBUSB_SUCCESS != libusb_ret) {
      LOG4CXX_INFO(logger_, "libusb_set_configuration failed: " << libusb_error_name(
                     libusb_ret));
      LOG4CXX_TRACE(logger_, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
      return;
    }
  }

  libusb_ret = libusb_claim_interface(device_handle_libusb, 0);
  if (LIBUSB_SUCCESS != libusb_ret) {
    LOG4CXX_INFO(logger_, "libusb_claim_interface failed: " << libusb_error_name(libusb_ret));
    CloseDeviceHandle(device_handle_libusb);
    LOG4CXX_TRACE(logger_, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
    return;
  }


  CAMERACORE_libusb_getUsbDevice( struct Device& device,
                                  libusb_device& device_libusb,
                                  libusb_device_descriptor& descriptor,
                                  libusb_device_handle& device_handle_libusb);

  
  


  /***********
        push the device to the device list
  ****************/
  CAMERACORE_libusb_UpdateDeviceList();

  CAMERACORE_libusb_OnDeviceArrived(device);
  
}

int CAMERACORE_libusb_OnDeviceArrived(){

    /*********************************
        多设备的处理
    */



    /*********************************
       工作模式的确定
    */




    /*********************************
       设备操作
    */
    if (IsAppleDevice(device)) {
      CAMERACORE_libusb_GoogleDeviceHandle(device);
  } else {
    if (IsGoogleAccessory(device)) {
      CAMERACORE_libusb_AppleDeviceHandle(device);
    } else {
      TurnIntoAccessoryMode(device);
    }
  }
}


int CAMERACORE_libusb_device_lifed(libusb_device* device_libusb){}




void CAMERACORE_libusb_device_getInfo( libusb_device* device_libusb,
                                    libusb_device_handle* device_handle_libusb){
  const uint8_t bus_number = libusb_get_bus_number(device_libusb);
  const uint8_t device_address = libusb_get_device_address(device_libusb);

}


libusb_hotplug_callback_handle CAMERACORE_libusb_HotplugArrivedCallbackHandle(){}
libusb_hotplug_callback_handle CAMERACORE_libusb_HotplugLeftCallbackHandle(){}
int CAMERACORE_libusb_TurnIntoAccessoryMode(device){
  //留作其他设备接口
}


bool IsGoogleAccessory(const Device* device) {
  return (kAoaVid == device->vendor_id()) &&
    ((kAoaPid1 == device->product_id()) || (kAoaPid2 == device->product_id()));
}


bool IsAppleDevice(const Device* device) {
  return (kAppleVid == device->vendor_id()) &&
    ((kApplePid1 == device->product_id()) ||
     (kApplePid2 == device->product_id()) ||
     (kApplePid3 == device->product_id()) ||
     (kApplePid4 == device->product_id()) ||
     (kApplePid5 == device->product_id()) ||
     (kApplePid6 == device->product_id()) ||
     (kApplePid7 == device->product_id()));
}


int CAMERACORE_libusb_GoogleDeviceHandle(struct Device& device){
  /*
    针对google设备的接口
  */
  CAMERACORE_libusb_DeviceHandle(device);



}
int CAMERACORE_libusb_AppleDeviceHandle(struct Device& device){

  /*
    针对apple设备的接口
  */

  CAMERACORE_libusb_DeviceHandle(device);

}




int CAMERACORE_libusb_DeviceHandle(struct Device& device){






}

int CAMERACORE_libusb_UpdateDeviceList(){
   /* push the device to the device list
      参考sdl transport_adapter_impl.cc L373 SerchDeviceDone 来更新列表
      device 列表 是否需要自己定制？
      */

    CAMERACORE_libusb_OnDeviceListUpdated();


}


int CAMERACORE_libusb_OnDeviceListUpdated(){

    /*
      通知上层 列表更新完成 并使上层进行相应的操作
    */


}

int CAMERACORE_libusb_DeviceConnect(){
  /*
  buffer分配 
  多台设备的连接判断
  usb endpoint分配等等 
  */


}