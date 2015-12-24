#include "cameradae.h"
#include "libusb_api.h"


int Cameracore_

void SubmitControlTransfer(ControlTransferSequenceState* sequence_state) {
  
  UsbControlTransfer* transfer = sequence_state->CurrentTransfer();
  if (NULL == transfer) {
    LOG4CXX_TRACE(logger_, "exit. Condition: NULL == transfer");
    return;
  }

  libusb_transfer* libusb_transfer = libusb_alloc_transfer(0);
  if (0 == libusb_transfer) {
    LOG4CXX_ERROR(logger_, "libusb_alloc_transfer failed");
    sequence_state->Finish();
    LOG4CXX_TRACE(logger_, "exit. Condition: 0 == libusb_transfer");
    return;
  }

  
  const libusb_request_type request_type = LIBUSB_REQUEST_TYPE_VENDOR;

  libusb_endpoint_direction endpoint_direction = LIBUSB_ENDPOINT_IN;

  if (transfer->Direction() == UsbControlTransfer::IN) {
    endpoint_direction = LIBUSB_ENDPOINT_IN;
  } 
  else if (transfer->Direction() == UsbControlTransfer::OUT) {
    endpoint_direction = LIBUSB_ENDPOINT_OUT;
  } 
  else {
    NOTREACHED();
  }

  const uint8_t request = transfer->Request();
  const uint16_t value = transfer->Value();
  const uint16_t index = transfer->Index();
  const uint16_t length = transfer->Length();

  unsigned char* buffer = static_cast<unsigned char*>(malloc(length + LIBUSB_CONTROL_SETUP_SIZE));
  if (NULL == buffer) {
    LOG4CXX_ERROR(logger_, "buffer allocation failed");
    libusb_free_transfer(libusb_transfer);
    sequence_state->Finish();
    LOG4CXX_TRACE(logger_, "exit. Condition: NULL == buffer");
    return;
  }

  libusb_fill_control_setup(buffer, request_type | endpoint_direction, request,
                            value, index, length);

  if (0 != length && endpoint_direction == LIBUSB_ENDPOINT_OUT) {
    const char* data = static_cast<UsbControlOutTransfer*>(transfer)->Data();
    memcpy(buffer + LIBUSB_CONTROL_SETUP_SIZE, data, length);
  }
  libusb_fill_control_transfer(
    libusb_transfer, sequence_state->device()->GetLibusbHandle(), buffer,
    UsbTransferSequenceCallback, sequence_state, 0);
  libusb_transfer->flags = LIBUSB_TRANSFER_FREE_BUFFER;

  const int libusb_ret = libusb_submit_transfer(libusb_transfer);
  if (LIBUSB_SUCCESS != libusb_ret) {
    LOG4CXX_ERROR(logger_, "libusb_submit_transfer failed: "
                  << libusb_error_name(libusb_ret));
    libusb_free_transfer(libusb_transfer);
    sequence_state->Finish();
  }
  LOG4CXX_TRACE(logger_, "exit");
}


 	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices  
    libusb_device_handle *dev_handle; //a device handle  
    libusb_context *ctx = NULL; //a libusb session  
    int r; //for return values  
        ssize_t cnt; //holding number of devices in list  
       


        r = libusb_init(&ctx); //initialize the library for the session we just declared  
        if(r < 0) {  
            CAMERACORE_log(fp,"Init Error\n"); //there was an error  
            return 1;  
        }  
       

       // libusb_set_debug(ctx, LIBUSB_LOG_LEVEL_INFO); //set verbosity level to 3, as suggested in the documentation  
      
       

        cnt = libusb_get_device_list(ctx, &devs); //get the list of devices  
        if(cnt < 0) {  
            CAMERACORE_log(fp,"Get Device Error\n"); //there was an error  
            return 1;  
        }  
        CAMERACORE_log(fp,cnt);
        CAMERACORE_log(fp," Devices in list.\n");
          
      
        
        dev_handle = libusb_open_device_with_vid_pid(ctx, VID, PID); //these are vendorID and productID I found for my usb device  
        if(dev_handle == NULL)  
            CAMERACORE_log(fp,"Cannot open device\n");  
        else  
            CAMERACORE_log(fp,"Device Opened\n");  
        

        libusb_free_device_list(devs, 1); //free the list, unref the devices in it  
      
        if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached  
            CAMERACORE_log(fp,"Kernel Driver Active\n");  
            if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it  
            CAMERACORE_log(fp,"Kernel Driver Detached!\n");  
        }  
        
        r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)  
        if(r < 0) {  
            CAMERACORE_log(fp,"Cannot Claim Interface\n");  
            return 1;  
        }  
        CAMERACORE_log(fp,"Claimed Interface\n");  
      
        device_satus(dev_handle);  
      
        libusb_close(dev_handle); //close the device we opened  
        libusb_exit(ctx); //needs to be called to end the  
      
        return 0; 






TransportAdapter::Error UsbHandler::Init() {
  
  int libusb_ret = libusb_init(&libusb_context_);

  
  libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG);

  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                 LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, ArrivedCallback, this,
                 &arrived_callback_handle_);


  libusb_ret = libusb_hotplug_register_callback(
                 libusb_context_, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
                 static_cast<libusb_hotplug_flag>(0), LIBUSB_HOTPLUG_MATCH_ANY,
                 LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, LeftCallback, this,
                 &left_callback_handle_);


  thread_->start();
 
}



