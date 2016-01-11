 #include "libusb-api.h"


extern FILE *fp; 
char in_buffer[1000];
char out_buffer[1000];
libusb_hotplug_callback_handle HotplugArrivedCallbackHandle;
libusb_hotplug_callback_handle HotplugLeftCallbackHandle;


int CAMERACORE_libusb_init(struct Device* device){
  CAMERACORE_log(fp, "[CAMERACORE_log]: CAMERACORE_libusb_init [START]\n");
  /*init the endpoint*/
  device->in_endpoint = 0 ;
  device->out_endpoint = 0;

  CAMERACORE_log(fp, "[CAMERACORE_log]: CAMERACORE_libusb_init [libusb_init]\n");
  int libusb_ret = libusb_init(&(device->libusb_context_cameracore));
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: CAMERACORE_libusb_init [libusb_init failed]\n");
    return -1;
  }

  if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: CAMERACORE_libusb_init [LIBUSB_CAP_HAS_HOTPLUG not supported]\n");
    return -1;
  }
  CAMERACORE_log(fp, "[CAMERACORE_log]: CAMERACORE_libusb_init [For device In Call libusb_hotplug_register_callback()]\n");
  libusb_ret = libusb_hotplug_register_callback(
                 device->libusb_context_cameracore, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                 LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, HotplugDeviceArrivedCallback, device,
                 &HotplugArrivedCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]: CAMERACORE_libusb_init [libusb_hotplug_register_callback-In failed]\n");
    //CAMERACORE_log(fp,libusb_ret);
    // CAMERACORE_log(fp, "]");
    return -1;
  }

  CAMERACORE_log(fp, "[CAMERACORE_log]: CAMERACORE_libusb_init [For device Out Call libusb_hotplug_register_callback()]\n");
  libusb_ret = libusb_hotplug_register_callback(
                 device->libusb_context_cameracore, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                 LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, HotplugDeviceLifedCallback, device,
                 &HotplugLeftCallbackHandle);

  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp,"[CAMERACORE_log]: CAMERACORE_libusb_init [libusb_hotplug_register_callback-Out failed]\n");
    return -1;
  }
  return 0;
}


int HotplugDeviceArrivedCallback( struct libusb_context* context, 
                                  struct libusb_device* device_libusb, 
                                  libusb_hotplug_event event, 
                                  void* data) {

  int i = 0;
  int err = 0;
  CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceArrivedCallback [HotplugDeviceArrivedCallback START]\n");
  //获取热插拔传递的device地址
  CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceArrivedCallback [Get the data pointer from libusb_hotplug_register_callback]\n");
  struct Device* device =(struct Device*)(data);
  // discover devices  获取热插拔传递的device
  struct libusb_device **list;
  CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceArrivedCallback [libusb_get_device_list()]\n");
  int cnt = libusb_get_device_list(device->libusb_context_cameracore, &list);
  if (cnt < 0){
    CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceArrivedCallback [Getting device list failed]\n");
  }

  for (i = 0; i < cnt; i++) {
      libusb_device *dev = list[i];
      struct libusb_device_descriptor dsp;
      int libusb_ret = libusb_get_device_descriptor(dev, &dsp);
      if (LIBUSB_SUCCESS != libusb_ret) {
        CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceArrivedCallback [libusb_get_device_descriptor failed]\n");
        return -1;
      }
      if (!is_interesting(&dsp)) {
          device->device_libusb =dev;
          g_printerr ("Device IdVendor %d: \n", dsp.idVendor);
          g_printerr ("Device IdProduct %d: \n", dsp.idProduct);
          CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceArrivedCallback [Call DeviceArrived()]\n");
          DeviceArrived(device);
          break;
      }
  }
 
  libusb_free_device_list(list, 1);
  return 0;
}



int is_interesting(struct libusb_device_descriptor* desptr){
    /*
      判断设备类型 工作模式的确定
    */
    if (IsAppleDevice(desptr)) {
      //AppleDeviceHandle(device);
      CAMERACORE_log(fp, "[CAMERACORE_log]: is_interesting [Apple device has been found]\n");
      return 0;
    } 
    if (IsGoogleAccessory(desptr)){
                  
                  //GoogleDeviceHandle(device);
                  CAMERACORE_log(fp, "[CAMERACORE_log]: is_interesting [Android device has been found]\n");
                  return 0;
    }
    
    else {  CAMERACORE_log(fp, "[CAMERACORE_log]: is_interesting [No interesting device]\n");
            return -1;
    }
}

int HotplugDeviceLifedCallback(   struct libusb_context* context, 
                                  struct libusb_device* device_libusb, 
                                  libusb_hotplug_event event, 
                                  void* data){

  CAMERACORE_log(fp, "[CAMERACORE_log]: HotplugDeviceLifedCallback [Get the data pointer]\n");
  /**********
  数据 参数转换    
  *************/
  struct Device* device = (struct Device*)(data);
  DeviceLifed(device);
  return 0;
}


int DeviceArrived(  struct Device* device){
  
  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [START]\n");
  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call libusb_get_device_descriptor()]\n");
  int libusb_ret = libusb_get_device_descriptor(device->device_libusb, &device->device_descriptor);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceArrived [libusb_get_device_descriptor failed]\n");
    return -1;
  }
  g_printerr ("In DeviceArrived Device idVendor %d: \n", device->device_descriptor.idVendor);
  g_printerr ("In DeviceArrived Device idProduct %d: \n", device->device_descriptor.idProduct);

  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call libusb_open()]\n");
  libusb_ret = libusb_open(device->device_libusb, &(device->device_handle_libusb));

  if (libusb_ret != LIBUSB_SUCCESS) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [libusb_open failed]\n");
    return -1;
  }

  int configuration;
  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call libusb_get_configuration()]\n");
  libusb_ret = libusb_get_configuration(device->device_handle_libusb, &configuration);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [libusb_get_configuration failed]\n");
    return -1;
  }

  if (configuration != kUsbConfiguration) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call libusb_set_configuration()]\n");
    libusb_ret = libusb_set_configuration(device->device_handle_libusb, kUsbConfiguration);
    if (LIBUSB_SUCCESS != libusb_ret) {
      CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [libusb_set_configuration failed]\n");
      return -1;
    }
  }

  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call libusb_claim_interface()]\n");
  
  libusb_ret = libusb_claim_interface(device->device_handle_libusb, 0);
  
  if (LIBUSB_SUCCESS != libusb_ret) {
    
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [libusb_claim_interface failed]\n");
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call CloseDeviceHandle()]\n");
    CloseDeviceHandle(device->device_handle_libusb);
    return -1;
  }


  /***********
        push the device to the device list
  ****************/
  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call UpdateDeviceList()]\n");
  UpdateDeviceList();
  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceArrived [call OnDeviceArrived()]\n");
  OnDeviceArrived(device);
  return 0;
}

int OnDeviceArrived(struct Device* device){
    /*
      多设备的处理 buffer分配 endpoint的分配
    */
    CAMERACORE_log(fp, "[CAMERACORE_log]: OnDeviceArrived [call DeviceConnect()]\n");
    DeviceConnect(device);
    return 0;
}


void DeviceLifed(  struct Device * device){

    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceLifed [Device has lifed]\n");


    //libusb_handle_events_completed(libusb_context_cameracore, &completed);

    libusb_hotplug_deregister_callback(device->libusb_context_cameracore, HotplugArrivedCallbackHandle);
    libusb_hotplug_deregister_callback(device->libusb_context_cameracore, HotplugLeftCallbackHandle);

    libusb_close(device->libusb_context_cameracore);

}


int TurnIntoAccessoryMode(struct libusb_device_descriptor* desptr){
  //留作其他设备接口
  return 0;
}


int IsGoogleAccessory(struct libusb_device_descriptor* desptr) {
  return (kAoaVid == desptr->idVendor);
}


int IsAppleDevice(struct libusb_device_descriptor* desptr) {
  return (kAppleVid == desptr->idVendor) &&
    ((kApplePid1 == desptr->idProduct) ||
     (kApplePid2 == desptr->idProduct) ||
     (kApplePid3 == desptr->idProduct) ||
     (kApplePid4 == desptr->idProduct) ||
     (kApplePid5 == desptr->idProduct) ||
     (kApplePid6 == desptr->idProduct) ||
     (kApplePid7 == desptr->idProduct));
}


void GoogleDeviceHandle(struct Device* device){
  /*
    针对google设备的接口
  */
  DeviceHandle(device);
}
void AppleDeviceHandle(struct Device* device){

  /*
    针对apple设备的接口
  */

  DeviceHandle(device);

}




int DeviceHandle(struct Device* device){

/*
  填充数据
*/

  CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceHandle [DeviceHandle() has been done]\n");  
  return 0;
}

int UpdateDeviceList(){
   /* push the device to the device list
      参考sdl transport_adapter_impl.cc L373 SerchDeviceDone 来更新列表
      device 列表 是否需要自己定制？
      */
    CAMERACORE_log(fp, "[CAMERACORE_log]: UpdateDeviceList [Devicelist has been update]\n");  
    OnDeviceListUpdated();
    return 0;
}


int OnDeviceListUpdated(){

    /*
      通知上层 列表更新完成 并使上层进行相应的操作
    */
    CAMERACORE_log(fp, "[CAMERACORE_log]: OnDeviceListUpdated [Inform the higher level to aquire the respond]\n");
    return 0;
}

int DeviceConnect(struct Device* device){
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
 


  // if (!FindEndpoints(device)) {
  //   CAMERACORE_log(fp, "[CAMERACORE_log]:DeviceConnect [Device cannt connect because endPoints was not found]");
  //   return -1;
  // }

   /********************************************
      向上层通知 设备链接成功，可以进行数据传输
    *********************************************/
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceConnect [Call OnDeviceConnect]\n");
    OnDeviceConnect();

  return  0;
}


// int FindEndpoints(struct Device* device){

//   struct libusb_config_descriptor* config;
//   // const struct libusb_interface* interface;
//   // const struct libusb_interface_descriptor* iface_desc;
//   // const struct libusb_endpoint_descriptor* endpoint_desc;
//   int find_in_endpoint = 1;
//   int find_out_endpoint = 1;
//   int i,k,j;
//   const int endpoint_dir;
  
//   const int libusb_ret = libusb_get_active_config_descriptor(device->device_libusb, &config);
//   if (LIBUSB_SUCCESS != libusb_ret) {
//     CAMERACORE_log(fp, "[CAMERACORE_log]:libusb_get_active_config_descriptor failed: ");
//     return -1;
//   }
  
//     for ( i = 0; i < config->bNumInterfaces; ++i) {
//     struct libusb_interface& interface = config->interface[i];
//     for (j = 0; j < interface.num_altsetting; ++j) {
//       struct libusb_interface_descriptor& iface_desc = interface.altsetting[j];
//       for (k = 0; k < iface_desc.bNumEndpoints; ++k) {
//         struct libusb_endpoint_descriptor& endpoint_desc =
//           iface_desc.endpoint[k];

//         uint8_t endpoint_dir =
//           endpoint_desc.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK;
//         if (find_in_endpoint && endpoint_dir == LIBUSB_ENDPOINT_IN) {
//           device->in_endpoint = endpoint_desc.bEndpointAddress;
//           device->in_endpoint_max_packet_size = endpoint_desc.wMaxPacketSize;
//           find_in_endpoint = -1;
//         } else if (find_out_endpoint && endpoint_dir == LIBUSB_ENDPOINT_OUT) {
//           device->out_endpoint = endpoint_desc.bEndpointAddress;
//           device->out_endpoint_max_packet_size = endpoint_desc.wMaxPacketSize;
//           find_out_endpoint = -1;
//         }
//       }
//     }
//   }
//   libusb_free_config_descriptor(config);

//   const int result = !(find_in_endpoint || find_out_endpoint);
//   //CAMERACORE_log(fp, "exit with " << (result ? "1" : "-1"));
//   return result;
// }

int PostOutTransfer(struct Device * device) {
  CAMERACORE_log(fp, "[CAMERACORE_log]: In Fuc. PostOutTransfer [Call libusb_alloc_transfer]\n");
  struct libusb_transfer* out_transfer = libusb_alloc_transfer(16);
  if (0 == out_transfer) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:PostOutTransfer [libusb_alloc_transfer failed]\n");
    return -1;
  }
  CAMERACORE_log(fp, "[CAMERACORE_log]: In Fuc. PostOutTransfer [Call libusb_fill_iso_transfer()]\n");
  libusb_fill_iso_transfer(out_transfer,device->device_handle_libusb,device->out_endpoint,out_buffer,
        sizeof(out_transfer), 16,OutTransferCallback, NULL, 0);
  CAMERACORE_log(fp, "[CAMERACORE_log]: In Fuc. PostOutTransfer [Call libusb_set_iso_packet_lengths()]\n");
  libusb_set_iso_packet_lengths(out_transfer, sizeof(out_buffer)/16);
  
  CAMERACORE_log(fp, "[CAMERACORE_log]: In Fuc. PostOutTransfer [Call libusb_submit_transfer()]\n");
  const int libusb_ret = libusb_submit_transfer(out_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]:PostOutTransfer [libusb_submit_transfer failed]\n");
    AbortConnection();
    return -1;
  }
  return 0;
}

int PostInTransfer(struct Device * device){

  struct libusb_transfer* in_transfer = libusb_alloc_transfer(0);
  if (NULL == in_transfer) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: DeviceConnect [libusb_alloc_transfer failed]\n");
    return -1;
  }

  libusb_fill_bulk_transfer(in_transfer, device->device_handle_libusb, device->in_endpoint,
                            in_buffer, device->in_endpoint_max_packet_size,
                            InTransferCallback, NULL, 0);
  const int libusb_ret = libusb_submit_transfer(in_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    CAMERACORE_log(fp, "[CAMERACORE_log]: PostInTransfer [libusb_submit_transfer failed]\n");
    // CAMERACORE_log(fp,libusb_error_name(libusb_ret));
    // CAMERACORE_log(fp,"]");
    return -1;
  }
  
  return 0;
}

void InTransferCallback(struct libusb_transfer* transfer){
  
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
    CAMERACORE_log(fp, "[CAMERACORE_log]: InTransferCallback USB incoming transfer failed: ");
    // CAMERACORE_log(fp,libusb_error_name(transfer->status));
    // CAMERACORE_log(fp,"]");

    /*
      向上层发出 “传输失败”的event，并要求应答
  */
  }
  // if (disconnecting_) {
  //   waiting_in_transfer_cancel_ = -1;
  // } else {
  //   if (!PostInTransfer()) {
  //     CAMERACORE_log(fp, "USB incoming transfer failed with "
  //                   << "LIBUSB_TRANSFER_NO_DEVICE. Abort connection.");
  //     AbortConnection();
  //   }
  // }
}


void OutTransferCallback(struct libusb_transfer* transfer){

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
   
    CAMERACORE_log(fp, "[CAMERACORE_log]: OutTransferCallback USB incoming transfer failed: ");
    // CAMERACORE_log(fp,libusb_error_name(transfer->status));
    // CAMERACORE_log(fp,"]");
    /*
      向上层发出 “传输失败”的event，并要求应答
  */
  } 
}


void CAMERACORE_libusb_SendData(struct Device* device){
  /*
  设置buffer属性
  */
  CAMERACORE_log(fp, "[CAMERACORE_log]: In CAMERACORE_libusb_SendData [Send data to usb device]\n");
  CAMERACORE_log(fp, "[CAMERACORE_log]: In CAMERACORE_libusb_SendData [Call PostOutTransfer()]\n");
  PostOutTransfer(device);
  CAMERACORE_log(fp, "[CAMERACORE_log]: In CAMERACORE_libusb_SendData [Finished to call PostOutTransfer()]\n");
  /*
  视频传输优化逻辑
  */
}


void CloseDeviceHandle(struct libusb_device_handle* device_handle_libusb){


}

void AbortConnection(){

}

void OnDeviceConnect(){
  /*
    向上层通知 设备链接成功，可以进行数据传输
  */
  CAMERACORE_log(fp, "[CAMERACORE_log]: OnDeviceConnect [Inform the higher level ,\nthe DeviceConnect has completed and data transform is ready]\n");

}
