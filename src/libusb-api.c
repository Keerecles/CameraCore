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

// First, use "lsusb" see vid and pid.  
// there is my printer(hp deskjet 1010) vid and pid.  
#define VID 0x18d1  
#define PID 0x4e26 



static int device_satus(libusb_device_handle *hd)  
    {  
      
        int interface = 0;  
        unsigned char byte;  
        libusb_control_transfer(hd, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,  
                LIBUSB_REQUEST_CLEAR_FEATURE,  
                0,  
                interface,  
                &byte, 1, 5000);  
      
        CAMERACORE_log(fp,"status:");  
        CAMERACORE_log(fp,byte);

    /** 
     * byte 
     * normal:0x18 
     * other :0x10 
     */  
        return 0;  
    }  

uint16_t libusbapi_get_VID()  { 
 }
uint16_t libusbapi_get_PID()  { 
 }



int main(int argc, char *argv[]) {
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
        CAMERACORE_log(fp,"%d Devices in list.\n");
        CAMERACORE_log(fp,cnt);  
      
        
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

}