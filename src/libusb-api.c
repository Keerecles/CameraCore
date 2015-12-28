#include "cameradae.h"
#include "libusb_api.h"



libusb_context* libusb_context_;
libusb_device_handle* device_handle_libusb;
struct Device device;
uint8_t in_endpoint;
uint8_t out_endpoint;


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
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, HotplugDeviceArrivedCallback, NULL,
                 &HotplugArrivedCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_hotplug_register_callback failed: ");
    CAMERACORE_log(fp,libusb_ret);
    CAMERACORE_log(fp, "]");
    return -1;
  }

  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                 static_cast<libusb_hotplug_flag>(0), LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, HotplugDeviceLifedCallback, NULL,
                 &HotplugLeftCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_hotplug_register_callback failed: ");
    CAMERACORE_log(fp,libusb_ret);
    CAMERACORE_log(fp, "]");
    return -1;
  }
}


int HotplugDeviceArrivedCallback(  libusb_context* context, libusb_device* device,
                                                  libusb_hotplug_event event, void* data) {
  CAMERACORE_log(fp, "[CAMERACORE_log]:HotplugDeviceArrivedCallback [libusb_hotplug_register_callback failed: ")
  /**********
  数据 参数转换    


  *************/

  DeviceArrived();

  return 0;
  }

}


int HotplugDeviceLifedCallback(  libusb_context* context, libusb_device* device,
                                                libusb_hotplug_event event, void* data){

CAMERACORE_log(fp, "[CAMERACORE_log]:HotplugDeviceLifedCallback [libusb_hotplug_register_callback failed: ")
  /**********
  数据 参数转换    


  *************/
CAMERACORE_libusb_device_lifed();

}


int DeviceArrived(libusb_device* device_libusb){
  
  libusb_device_descriptor descriptor;
  int libusb_ret = libusb_get_device_descriptor(device_libusb, &descriptor);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "libusb_get_device_descriptor failed: " << libusb_ret);
    CAMERACORE_log(fp, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
  return;
  }
  libusb_ret = libusb_open(device_libusb, &device_handle_libusb);
  if (libusb_ret != LIBUSB_SUCCESS) {
    CAMERACORE_log(fp, "libusb_open failed: " << libusb_error_name(libusb_ret));
    CAMERACORE_log(fp, "exit. Condition: libusb_ret != LIBUSB_SUCCESS");
    return;
  }

  int configuration;
  libusb_ret = libusb_get_configuration(device_handle_libusb, &configuration);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "libusb_get_configuration failed: " << libusb_error_name(
                   libusb_ret));
    CAMERACORE_log(fp, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
    return;
  }

  if (configuration != kUsbConfiguration) {
    libusb_ret = libusb_set_configuration(device_handle_libusb, kUsbConfiguration);
    if (LIBUSB_SUCCESS != libusb_ret) {
      CAMERACORE_log(fp, "libusb_set_configuration failed: " << libusb_error_name(
                     libusb_ret));
      CAMERACORE_log(fp, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
      return;
    }
  }

  libusb_ret = libusb_claim_interface(device_handle_libusb, 0);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "libusb_claim_interface failed: " << libusb_error_name(libusb_ret));
    CloseDeviceHandle(device_handle_libusb);
    CAMERACORE_log(fp, "exit. Condition: LIBUSB_SUCCESS != libusb_ret");
    return;
  }


  CAMERACORE_libusb_getUsbDeviceInfo( struct Device& device,
                                      libusb_device& device_libusb,
                                      libusb_device_descriptor& descriptor,
                                      libusb_device_handle& device_handle_libusb);


  /***********
        push the device to the device list
  ****************/
  UpdateDeviceList();

  OnDeviceArrived(device);
  
}

int OnDeviceArrived(){

    /*
      判断设备类型 工作模式的确定
    */
    if (IsAppleDevice(device)) {
      GoogleDeviceHandle(device);
    } 
      else {  if (IsGoogleAccessory(device)) {
                  AppleDeviceHandle(device);
              }   
                else {  TurnIntoAccessoryMode(device);
                }
      }

    

    /*
      多设备的处理 buffer分配 endpoint的分配
    */
    DeviceConnect();



    /********************************************
      向上层通知 设备链接成功，可以进行数据传输




    *********************************************/
}


int DeviceLifed(libusb_device* device_libusb){

     CAMERACORE_log(fp, "Device has lifed");
}







libusb_hotplug_callback_handle HotplugArrivedCallbackHandle(){}
libusb_hotplug_callback_handle HotplugLeftCallbackHandle(){}
int TurnIntoAccessoryMode(device){
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


int GoogleDeviceHandle(struct Device& device){
  /*
    针对google设备的接口
  */
  DeviceHandle(device);



}
int AppleDeviceHandle(struct Device& device){

  /*
    针对apple设备的接口
  */

  DeviceHandle(device);

}




int DeviceHandle(struct Device& device){

/*
  填充数据

*/
}

int UpdateDeviceList(){
   /* push the device to the device list
      参考sdl transport_adapter_impl.cc L373 SerchDeviceDone 来更新列表
      device 列表 是否需要自己定制？
      */

    OnDeviceListUpdated();


}


int OnDeviceListUpdated(){

    /*
      通知上层 列表更新完成 并使上层进行相应的操作
    */


}

int DeviceConnect(){
  /*
  (1)多台设备的连接判断
  (2)usb endpoint分配等等 
  (3)buffer分配 
  */


  /*  deviceID 和 appID 对应起来 (device_uid_, app_handle_)
      设备状态更新（SDL中有device状态相关设置，现在未移植过来）
      设备事件通知到上层，并等待应答（现在未移植实现）
      参考SDL 中 usb_connection.cc 中L277
  */


  if (!FindEndpoints()) {
    CAMERACORE_log(fp, "EndPoints was not found");
    CAMERACORE_log(fp, "exit with FALSE. Condition: !FindEndpoints()");
    return false;
  }
  /*buffer size  */
  in_buffer = new unsigned char[in_endpoint_max_packet_size];
  in_transfer = libusb_alloc_transfer(0);
  if (NULL == in_transfer) {
    CAMERACORE_log(fp, "libusb_alloc_transfer failed");
    CAMERACORE_log(fp, "exit with FALSE. Condition: NULL == in_transfer");
    return false;
  }

  return  true;
}


bool FindEndpoints(){

  struct libusb_config_descriptor* config;
  const int libusb_ret = libusb_get_active_config_descriptor(libusb_device_, &config);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "libusb_get_active_config_descriptor failed: "
                  << libusb_error_name(libusb_ret));
    CAMERACORE_log(fp, "exit with FALSE. Condition: LIBUSB_SUCCESS != libusb_ret");
    return false;
  }

  bool find_in_endpoint = true;
  bool find_out_endpoint = true;

  for (int i = 0; i < config->bNumInterfaces; ++i) {
    const libusb_interface& interface = config->interface[i];
    for (int i = 0; i < interface.num_altsetting; ++i) {
      const libusb_interface_descriptor& iface_desc = interface.altsetting[i];
      for (int i = 0; i < iface_desc.bNumEndpoints; ++i) {
        const libusb_endpoint_descriptor& endpoint_desc =
          iface_desc.endpoint[i];

        const uint8_t endpoint_dir =
          endpoint_desc.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK;
        if (find_in_endpoint && endpoint_dir == LIBUSB_ENDPOINT_IN) {
          in_endpoint = endpoint_desc.bEndpointAddress;
          in_endpoint_max_packet_size = endpoint_desc.wMaxPacketSize;
          find_in_endpoint = false;
        } else if (find_out_endpoint && endpoint_dir == LIBUSB_ENDPOINT_OUT) {
          out_endpoint = endpoint_desc.bEndpointAddress;
          out_endpoint_max_packet_size = endpoint_desc.wMaxPacketSize;
          find_out_endpoint = false;
        }
      }
    }
  }
  libusb_free_config_descriptor(config);

  const bool result = !(find_in_endpoint || find_out_endpoint);
  CAMERACORE_log(fp, "exit with " << (result ? "TRUE" : "FALSE"));
  return result;
}

bool PostOutTransfer() {
  
  out_transfer = libusb_alloc_transfer(0);
  if (0 == out_transfer) {
    CAMERACORE_log(fp, "libusb_alloc_transfer failed");
    CAMERACORE_log(fp, "exit with FALSE. Condition: 0 == out_transfer");
    return false;
  }
  libusb_fill_bulk_transfer(out_transfer, device_handle_libusb, out_endpoint,
                            out_buffer, out_endpoint_max_packet_size,
                            OutTransferCallback, this, 0);
  const int libusb_ret = libusb_submit_transfer(out_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "libusb_submit_transfer failed: "
                  << libusb_error_name(libusb_ret) << ". Abort connection.");
    AbortConnection();
    CAMERACORE_log(fp, "exit with FALSE. Condition: "
                  << "LIBUSB_SUCCESS != libusb_fill_bulk_transfer");
    return false;
  }
  CAMERACORE_log(fp, "exit with TRUE");
  return true;
}

bool PostInTransfer(){

  libusb_fill_bulk_transfer(in_transfer, device_handle_libusb, in_endpoint,
                            in_buffer, in_endpoint_max_packet_size,
                            InTransferCallback, this, 0);
  const int libusb_ret = libusb_submit_transfer(in_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "libusb_submit_transfer failed: "
                  << libusb_error_name(libusb_ret));
    CAMERACORE_log(fp,
                  "exit with FALSE. Condition: LIBUSB_SUCCESS != libusb_submit_transfer");
    return false;
  }
  CAMERACORE_log(fp, "exit with TRUE");
  return true;
}

void InTransferCallback(libusb_transfer* transfer){
  
  /*
  InTransferCallback/OutTransferCallback 主要完成 在数据传输成功后，
  向上层发出 “传输成功”的event，并要求应答
  现在无需移植实现
  参考SDL transport_adapter_listener_impl.cc L175
  */

  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {

  /*
      向上层发出 “传输成功”的event，并要求应答
  */
  } 
  else {
    CAMERACORE_log(fp, "USB incoming transfer failed: "
                  << libusb_error_name(transfer->status));
    /*
      向上层发出 “传输失败”的event，并要求应答
  */
  }
  // if (disconnecting_) {
  //   waiting_in_transfer_cancel_ = false;
  // } else {
  //   if (!PostInTransfer()) {
  //     CAMERACORE_log(fp, "USB incoming transfer failed with "
  //                   << "LIBUSB_TRANSFER_NO_DEVICE. Abort connection.");
  //     AbortConnection();
  //   }
  // }
  CAMERACORE_log(fp, "exit");
}


void OutTransferCallback(libusb_transfer* transfer){

  /*
  InTransferCallback/OutTransferCallback 主要完成 在数据传输成功后，
  向上层发出 “传输成功”的event，并要求应答
  现在无需移植实现
  参考SDL transport_adapter_listener_impl.cc L175
  */

  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {

  /*
      向上层发出 “传输成功”的event，并要求应答
  */
  } 
  else {
    CAMERACORE_log(fp, "USB incoming transfer failed: "
                  << libusb_error_name(transfer->status));
    /*
      向上层发出 “传输失败”的event，并要求应答
  */
  }
  // if (disconnecting_) {
  //   waiting_in_transfer_cancel_ = false;
  // } else {
  //   if (!PostInTransfer()) {
  //     CAMERACORE_log(fp, "USB incoming transfer failed with "
  //                   << "LIBUSB_TRANSFER_NO_DEVICE. Abort connection.");
  //     AbortConnection();
  //   }
  // }
  CAMERACORE_log(fp, "exit");
}



int CAMERACORE_libusb_getUsbDeviceInfo( struct Device& device,
                                  libusb_device& device_libusb,
                                  libusb_device_descriptor& descriptor,
                                  libusb_device_handle& device_handle_libusb){
  const uint8_t bus_number = libusb_get_bus_number(device_libusb);
  const uint8_t device_address = libusb_get_device_address(device_libusb);
}

int CAMERACORE_libusb_SendData(){
  /*
  设置buffer属性
  */


  /*
  视频传输优化逻辑
  */
}