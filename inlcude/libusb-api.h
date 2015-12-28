#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types .h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
//#include <dlt/dlt.h>
#include <libusb-1.0/libusb.h>
#include "Cameradae.h" 


#defined in_endpoint_max_packet_size   2764800;  //The sise of one frame of 720P video is 2764800Byte
#defined out_endpoint_max_packet_size  2764800;

struct Device
{
  uint8_t bus_number_;
  uint8_t address_;
  uint16_t vendor_id_;
  uint16_t product_id_;
  libusb_device_descriptor device_descriptor_;
  libusb_device_handle* libusb_device_handle_;
  libusb_device* libusb_device_;
};









