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
/* The appsink has received a buffer */
static void new_sample (GstElement *sink) {
  GstSample *sample;
  
  /* Retrieve the buffer */
  g_signal_emit_by_name (sink, "push-sample", &sample);
  if (sample) {
    /* 我们可以用GstBuffer的GST_BUFFER_DATA宏来获得数据指针和用GST_BUFFER_SIZE宏来获得数据大小。 
       结合usb in_buffer 的设置，将数据传输的节点设置好，
       然后调用libusb的CAMERACORE_libusb_SendData() 进行数据传输
    */
    CAMERACORE_log(fp,"*******************");




    
    gst_sample_unref (sample);
  }
}
  
/* This function is called when an error message is posted on the bus */
static void error_cb (GstBus *bus, GstMessage *msg) {
  GError *err;
  gchar *debug_info;
  
  /* Print error details on the screen */
  gst_message_parse_error (msg, &err, &debug_info);
  CAMERACORE_log(fp,"Error received from element ");
  CAMERACORE_log(fp, GST_OBJECT_NAME (msg->src));
  CAMERACORE_log(fp, err->message);
  g_clear_error (&err);
  g_free (debug_info);

}
  

void CAMERACORE_log(FILE *file, char *log_){
    
    char *p_log[300];
    memset(p_log, '\0', 300);
    sprintf(p_log, log_);
    fwrite(p_log, strlen(p_log), 1, file);
    fflush(file);
}




int main(int argc, char *argv[]) {
  
  gst_init(NULL,NULL);
  GstElement *pipeline, *video_source;
  GstElement *video_queue, *vpu_enc,*video_app_sink; 

  GstBuffer *buffer;
  GstMapInfo map;
  GstVideoInfo info;
  GstCaps *video_caps;

  GstBus *bus;
  GstMessage *msg;


  fp = fopen("/CAMERACORE_log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
  } 

  
   
  /* Create the elements */
  video_source = gst_element_factory_make ("imxv4l2src", "video_source");
  video_queue = gst_element_factory_make ("queue", "video_queue");
  vpu_enc = gst_element_factory_make ("vpuenc", "vpu_enc");
  video_app_sink = gst_element_factory_make ("appsink", "video_app_sink");
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("VehicleTravlingDataRecoderTest-pipeline");
  
   if (!pipeline){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element pipeline]\n");}
  // if (!tee){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element tee]\n");}
   if (!video_queue){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element vpu_enc]\n");}
   if (!vpu_enc){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element pipeline]\n");}
  // if (!rtp_h264){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element rtp_h264]\n");}
  // if (!udp_sink){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element udp_sink]\n");}
  // if (!snapshot_queue){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element snapshot_queue]\n");}
  // if (!pic_enc_format){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element pic_enc_format]\n");}
  // if (!pic_file_sink){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element pic_file_sink]\n");}
   if (!video_app_sink){CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Fail to create element video_app_sink]\n");}
  
  if (!pipeline || !video_source ||  !video_queue || !vpu_enc || !video_app_sink) {
    //g_printerr ("Not all elements could be created.\n");
    CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Gstreamer_Pipeline [Not all elements could be created.]\n");
  }


  /* Configure appsink */
  gst_video_info_set_format (&info, GST_VIDEO_FORMAT_UNKNOWN, 1280,720);
  video_caps = gst_video_info_to_caps (&info);

  g_object_set (video_app_sink, "emit-signals", TRUE, "caps", video_caps, NULL);
  g_signal_connect (video_app_sink, "new-sample", G_CALLBACK (new_sample), NULL);
  gst_caps_unref (video_caps);



  /* Link all elements that can be automatically linked because they have "Always" pads */
  gst_bin_add_many (GST_BIN (pipeline), video_source, video_queue, vpu_enc, video_app_sink,NULL);
  // if (gst_element_link_many (video_source, tee, NULL) != TRUE ||
  //     gst_element_link_many (video_queue, vpu_enc, rtp_h264, udp_sink, NULL) != TRUE ||
  //     gst_element_link_many (snapshot_queue, pic_enc_format,pic_file_sink, NULL) != TRUE) {
  //     CAMERACORE_log(fp,"Elements could not be linked\n");
  //     gst_object_unref (pipeline);
  // }


  
  /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
  
  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, NULL);
  
  /* Start playing the pipeline */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
   
  /* Free resources */
  if (msg != NULL)
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  closelog();
}
