#include "Cameradae.h"



extern GQueue *FSMtoGst_queue ;
DLT_IMPORT_CONTEXT(Camera_Daemon);

int State_Accept(){
    
    int _pipeline_tate_t;
    
    /*Get the command from the command-adapter*/
    while(g_queue_is_empty(FSMtoGst_queue)){

      DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("FSMtoGst_queue is empty! Gstreamer_Pipeline block here!"));
    
    }

    _pipeline_tate_t = g_queue_pop_head(FSMtoGst_queue);
    
    if(_pipeline_tate_t==NULL){
      DLT_LOG(Camera_Daemon, DLT_LOG_ERROR, DLT_STRING("Pop the empty FSMtoGst_queue ! "));
      pthread_exit(retval);
    }

    
    DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Gstreamer_Pipeline end!"));

    return _pipeline_tate_t;
}


int CAMERACORE_send_camerastatus_signal(int signal_to_client)
{
  int i_ret = 0;
  int parameter = 0;
  GVariant *parameters = NULL;

  DLT_LOG(PowerManager, DLT_LOG_INFO, DLT_STRING("CAMERACORE_send_camerastatus_signal  Start!"));


  parameters = g_variant_new("(i)", signal_to_client);

  /* dbus发送signal函数 */
  DLT_LOG(PowerManager, DLT_LOG_INFO, DLT_STRING("CAMERACORE_send_camerastatus_signal g_dbus_connection_emit_signal start"));
  g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                  TIZEN_PREFIX,   /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
                  CAMERADAE_OBJ_PATH,
                  CAMERADAE_SERVICE,
                  "camerastatus",
                  parameters,
                  NULL);
  
  DLT_LOG(PowerManager, DLT_LOG_INFO, DLT_STRING("CAMERACORE_send_camerastatus_signal g_dbus_connection_emit_signal end"));
  return 0;
}

int CAMERACORE_send_snapstatus_signal(boolean signal_to_client){
  int i_ret = 0;
  int parameter = 0;
  GVariant *parameters = NULL;

  DLT_LOG(PowerManager, DLT_LOG_INFO, DLT_STRING("CAMERACORE_send_snapstatus_signal"));


  parameters = g_variant_new("(b)", signal_to_client);

  /* dbus发送signal函数 */
  DLT_LOG(PowerManager, DLT_LOG_INFO, DLT_STRING("CAMERACORE_send_snapstatus_signal g_dbus_connection_emit_signal start"));
  g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                  TIZEN_PREFIX,   /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
                  CAMERADAE_OBJ_PATH,
                  CAMERADAE_SERVICE,
                  "snapstatus",
                  parameters,
                  NULL);
  DLT_LOG(PowerManager, DLT_LOG_INFO, DLT_STRING("CAMERACORE_send_snapstatus_signal g_dbus_connection_emit_signal end"));
  return 0;
}



void *Gstreamer_Pipeline(void*){
  
  GstElement *pipeline, *video_source, *tee;
  GstElement *video_queue, *vpu_enc, *rtp_h264, *udp_sink; 
  GstElement *snapshot_queue, *pic_format, *pic_file_sink;
  
  GstBus *bus;
  GstMessage *msg;
  GstPadTemplate *tee_src_pad_template;
  GstPad *tee_video_pad, *tee_snapshot_pad;
  GstPad *queue_video_pad, *queue_snapshot_pad;
   

  /* Initialize GStreamer */
  gst_init();
   
  /* Create the elements */


  video_source = gst_element_factory_make ("imxv4l2", "video_source");
  tee = gst_element_factory_make ("tee", "tee");
  video_queue = gst_element_factory_make ("queue", "video_queue");
  vpu_enc = gst_element_factory_make ("vpuenc", "vpu_enc");
  rtp_h264 = gst_element_factory_make ("rtph264pay", "rtp_h264");
  udp_sink = gst_element_factory_make ("udpsink", "udp_sink");
  snapshot_queue = gst_element_factory_make ("queue", "snapshot_queue");
  pic_enc_format = gst_element_factory_make ("jpegenc", "pic_enc_format");
  pic_file_sink = gst_element_factory_make ("filesink", "pic_file_sink");
  
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("VehicleTravlingDataRecoderTest-pipeline");
   
  if (!pipeline || !video_source || !tee || !video_queue || !vpu_enc || !rtp_h264 || !udp_sink || !snapshot_queue || !pic_enc_format || !pic_file_sink) {
    g_printerr ("Not all elements could be created.\n");
    pthread_exit(NULL);
  }
   
  /* Configure elements */
  //g_object_set (audio_source, "freq", 215.0f, NULL);
  g_object_set (rtp_h264, "pt", 96, NULL);
  g_object_set (udp_sink, "host", "127.0.0.1", "port", 8004, NULL);  //??how to get dynamic host ip
  g_object_set (pic_file_sink, "location", "/home/app/VTDR/capture/", NULL);//how to create radom jpeg file

  /* Link all elements that can be automatically linked because they have "Always" pads */
  gst_bin_add_many (GST_BIN (pipeline), video_source, tee, video_queue, vpu_enc, rtp_h264, udp_sink,
      snapshot_queue, pic_enc_format, pic_file_sink,NULL);
  if (gst_element_link_many (video_source, tee, NULL) != TRUE ||
      gst_element_link_many (video_queue, vpu_enc, rtp_h264, udp_sink, NULL) != TRUE ||
      gst_element_link_many (snapshot_queue, pic_enc_format,pic_file_sink, NULL) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    pthread_exit(NULL);
  }
   
  
   /*Aquire the "Request" pads */
    tee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (tee), "src%d");
    
    tee_video_pad = gst_element_request_pad (tee, tee_src_pad_template, NULL, NULL);
    g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (tee_video_pad));
    video_queue_pad = gst_element_get_static_pad (video_queue, "sink");
  
    tee_snap_pad = gst_element_request_pad (tee, tee_src_pad_template, NULL, NULL);
    g_print ("Obtained request pad %s for video branch.\n", gst_pad_get_name (tee_snap_pad));
    snapshot_queue_pad = gst_element_get_static_pad (snapshot_queue, "sink");
 


  while(true){
    /*Accept the state send by FSM */
    int _pipeline_tate_t = PIPELINE_STATE_NULL;
    _pipeline_tate_t = State_Accept();   //There is a block here when acceptting no state data
    g_printerr ("Pipeline state has been accepted by Gstreamer.\n");
    /* According to the state to change the gstreamer pipeline */
    switch(_pipeline_tate_t){
      case  PIPELINE_STATE_NULL     : 
            

            CAMERACORE_send_camerastatus_signal(0);
            break; 

      case  PIPELINE_STATE_VIDEO    :  
            if ( gst_pad_link (tee_video_pad, video_video_pad) != GST_PAD_LINK_OK){
                g_printerr ("Video pipeline could not be linked.\n");
                CAMERACORE_send_camerastatus_signal(-1);
                gst_object_unref (pipeline);
                pthread_exit(NULL);
            };
            else{ CAMERACORE_send_camerastatus_signal(1);}
           break;

      case  PIPELINE_STATE_SNAPSHOT :  
            g_object_set (video_source, "num-buffers", 1, NULL);
            if (  gst_pad_link (tee_snap_pad, snapshot_queue_pad) != GST_PAD_LINK_OK){
                  g_printerr ("Snapshot pipeline could not be linked.\n");
                  CAMERACORE_send_snapstatus_signal(FALSE);
                  gst_object_unref (pipeline);
                  pthread_exit(NULL);
            };
            else{ CAMERACORE_send_snapstatus_signal(TRUE);}
            break;
      default :
            break;
    }
   
    /* Start playing the pipeline */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
   
    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

  }


  gst_object_unref (video_queue_pad);
  gst_object_unref (snapshot_queue_pad);
  /* Release the request pads from the Tee, and unref them */
  gst_element_release_request_pad (tee, tee_audio_pad);
  gst_element_release_request_pad (tee, tee_snap_pad);
  gst_object_unref (tee_video_pad);
  gst_object_unref (tee_snap_pad);
   
  /* Free resources */
  if (msg != NULL)
  gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
   
  gst_object_unref (pipeline);
  
  closelog();

}