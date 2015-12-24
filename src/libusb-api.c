#include "cameradae.h"
#include "libusb_api.h"


int CAMERACORE_libusb_init();
int CAMERACORE_libusb_hotplug_detect();
int CAMERACORE_libusb_hotplug_cb();
int CAMERACORE_libusb_device_info();
int CAMERACORE_libusb_device_access();
int CAMERACORE_libusb_device_mode();
int CAMERACORE_libusb_update_deviceList();
int CAMERACORE_libusb_device_connect();
int CAMERACORE_libusb_data_feed();
int CAMERACORE_libusb_transfer();
int CAMERACORE_libusb_transfer_cb();