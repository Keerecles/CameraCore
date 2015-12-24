#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>
#include "Cameradae.h" 




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