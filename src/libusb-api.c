#include "cameradae.h"
#include "libusb-api.h"


extern FILE *fp; 
libusb_context* libusb_context_cameracore;
libusb_device_handle* device_handle_libusb;
struct Device* cameracore_usb_device;
uint8_t in_endpoint;
uint8_t out_endpoint;

libusb_hotplug_callback_handle HotplugArrivedCallbackHandle;
libusb_hotplug_callback_handle HotplugLeftCallbackHandle;


int CAMERACORE_libusb_init(struct Device* device){
  
  int libusb_ret = libusb_init(&libusb_context_cameracore);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[ AMERACORE_log]:CAMERACORE_libusb_init [libusb_init failed: ]");
    return -1;
  }

  if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:CAMERACORE_libusb_init [LIBUSB_CAP_HAS_HOTPLUG not supported]");
    return -1
  }

  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_cameracore, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                 LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, HotplugDeviceArrivedCallback, device,
                 &HotplugArrivedCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_hotplug_register_callback failed] ");
    // CAMERACORE_log(fp,libusb_ret);
    // CAMERACORE_log(fp, "]");
    return -1;
  }

  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_cameracore, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                 static_cast<libusb_hotplug_flag>(0), LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, HotplugDeviceLifedCallback, device,
                 &HotplugLeftCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_libusb_init [libusb_hotplug_register_callback failed: ");
    // CAMERACORE_log(fp,libusb_ret);
    // CAMERACORE_log(fp, "]");
    return -1;
  }
}


int HotplugDeviceArrivedCallback( libusb_context* context, 
                                  libusb_device* device_libusb, 
                                  libusb_hotplug_event event, 
                                  void* data) {
  CAMERACORE_log(fp, "[CAMERACORE_log]:HotplugDeviceArrivedCallback [libusb_hotplug_register_callback failed: ")
 
  //获取热插拔传递的device地址
  struct Device* device = static_cast<struct Device*>(data);
  DeviceArrived(device);
  return 0;
  }

}


int HotplugDeviceLifedCallback(   libusb_context* context, 
                                  libusb_device* device_libusb, 
                                  libusb_hotplug_event event, void* data){

  CAMERACORE_log(fp, "[CAMERACORE_log]:HotplugDeviceLifedCallback [libusb_hotplug_register_callback failed: ")
  /**********
  数据 参数转换    
  *************/
  DeviceLifed(device_libusb,device_handle_libusb);

}


int DeviceArrived(  struct Device* device){
  
  
  int libusb_ret = libusb_get_device_descriptor(device->device_libusb, device->device_descriptor);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceArrived [libusb_get_device_descriptor failed: ");
    // CAMERACORE_log(fp,libusb_ret);
    // CAMERACORE_log(fp,"]");
  return;
  }
  libusb_ret = libusb_open(device->device_libusb, &(device->device_handle_libusb));
  if (libusb_ret != LIBUSB_SUCCESS) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceArrived [libusb_open failed: ");
    // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
    // CAMERACORE_log(fp,"]");
    return;
  }

  int configuration;
  libusb_ret = libusb_get_configuration(device->device_handle_libusb, &configuration);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceArrived [libusb_get_configuration failed: ");
    // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
    // CAMERACORE_log(fp,"]");
    return;
  }

  if (configuration != kUsbConfiguration) {
    libusb_ret = libusb_set_configuration(device->device_handle_libusb, kUsbConfiguration);
    if (LIBUSB_SUCCESS != libusb_ret) {
      CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceArrived [libusb_set_configuration failed: ");
      // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
      // CAMERACORE_log(fp,"]");
      return;
    }
  }

  libusb_ret = libusb_claim_interface(device->device_handle_libusb, 0);
  if (LIBUSB_SUCCESS != libusb_ret) {
    
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceArrived [libusb_claim_interface failed: ");
    // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
    // CAMERACORE_log(fp,"]");
    CloseDeviceHandle(device->device_handle_libusb);
    return;
  }


  /***********
        push the device to the device list
  ****************/
  UpdateDeviceList();

  OnDeviceArrived(device);
  
}

int OnDeviceArrived(struct Device* device){

    /*
      判断设备类型 工作模式的确定
    */
    if (IsAppleDevice(device)) {
      AppleDeviceHandle(device);
    } 
      else {  if (IsGoogleAccessory(device)) {
                  
                  GoogleDeviceHandle(device);
              }   
                else {  TurnIntoAccessoryMode(device);
                }
      }


    /*
      多设备的处理 buffer分配 endpoint的分配
    */
    DeviceConnect();
}


int DeviceLifed(  libusb_device* 
                  device_libusb,
                  libusb_device_handle* 
                  device_handle_libusb){

    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceLifed [Device has lifed]");


    libusb_handle_events_completed(libusb_context_cameracore, &completed);

    libusb_hotplug_deregister_callback(libusb_context_cameracore, HotplugArrivedCallbackHandle);
    libusb_hotplug_deregister_callback(libusb_context_cameracore, HotplugLeftCallbackHandle);

    libusb_close(libusb_context_cameracore);

}


int TurnIntoAccessoryMode(struct Device* device){
  //留作其他设备接口
}


bool IsGoogleAccessory(const struct Device* device) {
  return (kAoaVid == device.vendor_id) &&
    ((kAoaPid1 == device.product_id) || (kAoaPid2 == device->product_id()));
}


bool IsAppleDevice(const struct Device* device) {
  return (kAppleVid == device->vendor_id()) &&
    ((kApplePid1 == device->product_id()) ||
     (kApplePid2 == device->product_id()) ||
     (kApplePid3 == device->product_id()) ||
     (kApplePid4 == device->product_id()) ||
     (kApplePid5 == device->product_id()) ||
     (kApplePid6 == device->product_id()) ||
     (kApplePid7 == device->product_id()));
}


int GoogleDeviceHandle(struct Device* device){
  /*
    针对google设备的接口
  */
  DeviceHandle(device);



}
int AppleDeviceHandle(struct Device* device){

  /*
    针对apple设备的接口
  */

  DeviceHandle(device);

}




int DeviceHandle(struct Device* device){

/*
  填充数据

*/
}

int UpdateDeviceList(){
   /* push the device to the device list
      参考sdl transport_adapter_impl.cc L373 SerchDeviceDone 来更新列表
      device 列表 是否需要自己定制？
      */
    CAMERACORE_log(fp, "[CAMERACORE_log]:UpdateDeviceList [In UpdateDeviceList updatedevicelist has been done]");  
    OnDeviceListUpdated();
}


int OnDeviceListUpdated(){

    /*
      通知上层 列表更新完成 并使上层进行相应的操作
    */
    CAMERACORE_log(fp, "[CAMERACORE_log]:OnDeviceListUpdated [Inform the higher level to aquire the respond]");
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
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceConnect [EndPoints was not found]");
    return false;
  }

   /********************************************
      向上层通知 设备链接成功，可以进行数据传输
    *********************************************/
    OnDeviceConnect();

  return  true;
}


bool FindEndpoints(){

  struct libusb_config_descriptor* config;
  const int libusb_ret = libusb_get_active_config_descriptor(libusb_device_, &config);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:libusb_get_active_config_descriptor failed: "
                  << libusb_error_name(libusb_ret));
    CAMERACORE_log(fp, "[CAMERACORE_log]:exit with FALSE. Condition: LIBUSB_SUCCESS != libusb_ret");
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

bool PostOutTransfer(libusb_device_handle* device_handle_libusb) {
  
  out_transfer = libusb_alloc_transfer(0);
  if (0 == out_transfer) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:PostOutTransfer [libusb_alloc_transfer failed]");
    return false;
  }
  libusb_fill_bulk_transfer(out_transfer, device_handle_libusb, out_endpoint,
                            out_buffer, out_endpoint_max_packet_size,
                            OutTransferCallback, this, 0);
  const int libusb_ret = libusb_submit_transfer(out_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:PostOutTransfer [libusb_submit_transfer failed:] ");
    // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
    // CAMERACORE_log(fp,"[CAMERACORE_log]:PostOutTransfer [Abort connection]");

    AbortConnection();
    return false;
  }
  return true;
}

bool PostInTransfer(libusb_device_handle* device_handle_libusb){

  in_transfer = libusb_alloc_transfer(0);
  if (NULL == in_transfer) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceConnect [libusb_alloc_transfer failed]");
    return false;
  }

  libusb_fill_bulk_transfer(in_transfer, device_handle_libusb, in_endpoint,
                            in_buffer, in_endpoint_max_packet_size,
                            InTransferCallback, this, 0);
  const int libusb_ret = libusb_submit_transfer(in_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:PostInTransfer [libusb_submit_transfer failed: ");
    // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
    // CAMERACORE_log(fp,"]");
    return false;
  }
  
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
    CAMERACORE_log(fp, "[CAMERACORE_log]:InTransferCallback USB incoming transfer failed: ");
    // CAMERACORE_log(fp,libusb_error_name(transfer->status));
    // CAMERACORE_log(fp,"]");

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
   
    CAMERACORE_log(fp, "[CAMERACORE_log]:OutTransferCallback USB incoming transfer failed: ");
    // CAMERACORE_log(fp,libusb_error_name(transfer->status));
    // CAMERACORE_log(fp,"]");
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
 
}


int CAMERACORE_libusb_SendData(){
  /*
  设置buffer属性
  */


  /*
  视频传输优化逻辑
  */
}


void CloseDeviceHandle(libusb_device_handle* device_handle_libusb){


}

void AbortConnection(){

}

void OnDeviceConnect(){
  /*
    向上层通知 设备链接成功，可以进行数据传输
  */
  CAMERACORE_log(fp, "[CAMERACORE_log]:OnDeviceConnect [Inform the higher level ,
                    the DeviceConnect has completed and data transform is ready]");

}