#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>
#include <gst/gst.h>
#include <dbus/dbus.h>
#include <gst/video/video.h>
#include <gst/app/gstappsink.h>
#include "libusb-api.h"
#include "cameradae.h" 

FILE *fp; 
extern char in_buffer[1000];
extern char out_buffer[1000];
/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  
  GstElement *pipeline, *video_source;
  GstElement *video_queue, *vpu_enc,*video_app_sink;

  guint sourceid;        /* To control the GSource */
  GMainLoop *main_loop;  /* GLib's Main Loop */
} CustomData;
  


/* The appsink has received a buffer */
static void new_sample (GstElement *sink, CustomData *data) {
  GstSample *sample;
  
  /* Retrieve the buffer */
  g_signal_emit_by_name (sink, "pull-sample", &sample);
  if (sample) {
    /* 我们可以用GstBuffer的GST_BUFFER_DATA宏来获得数据指针和用GST_BUFFER_SIZE宏来获得数据大小。 
       结合usb in_buffer 的设置，将数据传输的节点设置好，
       然后调用libusb的CAMERACORE_libusb_SendData() 进行数据传输
    */

    CAMERACORE_log(fp,"[CAMERACORE_log]: In new_sample Fuc. [*******************]\n");
    CAMERACORE_log(fp,"[CAMERACORE_log]: In new_sample Fuc. [Here you should ]\n");
    CAMERACORE_log(fp,"[CAMERACORE_log]: In new_sample Fuc. [Call Fuc. CAMERACORE_libusb_SendData()]\n");
    CAMERACORE_libusb_SendData(data->usbdevice);
    CAMERACORE_log(fp,"[CAMERACORE_log]: In new_sample Fuc. [SendData has been done]\n");
    gst_sample_unref (sample);
  }
}
  
/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg, CustomData *data) {
  GError *err;
  gchar *debug_info;
  
  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  CAMERACORE_log(fp,"[CAMERACORE_log]: In error_cb Fuc. [Error received from element ");
  CAMERACORE_log(fp,GST_OBJECT_NAME (msg->src));
  CAMERACORE_log(fp,err->message);
  CAMERACORE_log(fp,"]\n");
  CAMERACORE_log(fp,"Debugging information: ");
  CAMERACORE_log(fp,debug_info ? debug_info : "none\n");
 

  g_clear_error (&err);
  g_free (debug_info);
  
  g_main_loop_quit (data->main_loop);
}
  
  

void CAMERACORE_log(FILE *file, char *log_){
    
    char *p_log[300];
    memset(p_log, '\0', 300);
    sprintf(p_log, log_);
    fwrite(p_log, strlen(p_log), 1, file);
    fflush(file);
}




int main(int argc, char *argv[]) {

  CustomData data;
  struct Device *usbdevice;

  GstVideoInfo info;
  GstCaps *video_caps;
  GstBus *bus;
  

  fp = fopen("/CAMERACORE_log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
  } 
  CAMERACORE_log(fp,"[CAMERACORE_log]: In Fuc. main [Main Start]\n");
  CAMERACORE_log(fp,"[CAMERACORE_log]: In Fuc. main [Gstreamer Init]\n");
  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  CAMERACORE_log(fp,"[CAMERACORE_log]: In Fuc. main [libusb init]\n");
  int libusb_ret = CAMERACORE_libusb_init(usbdevice);
  if( libusb_ret != 0){
    CAMERACORE_log(fp,"[CAMERACORE_log]: In Fuc. main [libusb init failed]\n");
  }
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [libusb init finished]\n");

  /* Initialize cumstom data structure */
  memset (&data, 0, sizeof (data));
  
  /* Create the elements */
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [Crete the elements]\n");
  data.video_source = gst_element_factory_make ("imxv4l2src", "video_source");
  if (!data.video_source){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element video_source]\n");}
  data.video_queue = gst_element_factory_make ("queue", "video_queue");
  if (!data.video_queue){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element video_queue]\n");} 
  data.vpu_enc = gst_element_factory_make ("vpuenc", "vpu_enc");
  if (!data.vpu_enc){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element vpu_enc]\n");} 
  
  data.video_app_sink = gst_element_factory_make ("appsink", "video_app_sink");
  if (!data.video_app_sink){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element video_app_sink]\n");} 
  
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [Finished to create elements]\n");
  

  /* Create the empty pipeline */
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [Create the pipeline]\n");
  data.pipeline = gst_pipeline_new ("VehicleTravlingDataRecoderTest-pipeline");
  if (!data.pipeline){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element pipeline]\n");} 
  
  if (!data.pipeline || !data.video_source || !data.video_queue || !data.vpu_enc || !data.video_app_sink) {
    CAMERACORE_log(fp,"Not all elements could be created.\n");
    return -1;
  }
  CAMERACORE_log(fp,"[CAMERACORE_log]: In Fuc. main [Finished to create the pipeline]\n");
  /* Configure appsink */
  
  CAMERACORE_log(fp,"[CAMERACORE_log]: In Fuc. main [Configure appsink]\n");
  gst_video_info_set_format (&info, GST_VIDEO_FORMAT_UNKNOWN, 1280,720);
  video_caps = gst_video_info_to_caps (&info);
  g_object_set (data.video_app_sink, "emit-signals", TRUE, "caps", video_caps, NULL);
  g_signal_connect (data.video_app_sink, "new-sample", G_CALLBACK (new_sample), &data);
  gst_caps_unref (video_caps);
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [Finished to configure appsink]\n");
  /* Link all elements that can be automatically linked because they have "Always" pads */
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [Link all the elements]\n");
  gst_bin_add_many (GST_BIN (data.pipeline), data.video_source, data.video_queue, data.vpu_enc, data.video_app_sink, NULL);
  
  if (gst_element_link_many (data.video_source, data.video_queue, NULL) != TRUE ||
      gst_element_link_many (data.video_queue, data.vpu_enc, data.video_app_sink, NULL) != TRUE ) {
    CAMERACORE_log(fp,"Elements could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [All the elements are linked]\n");
  
  /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
  bus = gst_element_get_bus (data.pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, &data);
  gst_object_unref (bus);
  
  /* Start playing the pipeline */
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [Start playing the pipeline]\n");
  gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  CAMERACORE_log(fp,"[CAMERECORE_log]: In Fuc. main [After Start playing the pipeline]\n");
  /* Create a GLib Main Loop and set it to run */
  data.main_loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (data.main_loop);
  
  /* Free resources */
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}