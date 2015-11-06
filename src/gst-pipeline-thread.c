#include "Cameradae.h"


extern GQueue *FSMtoGst_queue ;
extern GQueue *SingaltoDbus_queue ;

//DLT_IMPORT_CONTEXT(Camera_Daemon);

int CAMERACORE_State_Accept(){
    
    int _pipeline_tate_t;
    
    /*Get the command from the command-adapter*/
    while(g_queue_is_empty(FSMtoGst_queue)){ }

    _pipeline_tate_t = g_queue_pop_head(FSMtoGst_queue);
    
    //DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Thread Gstreamer_Pipeline : CAMERACORE_State_Accept FINISH!"));
    syslog(LOG_INFO,"Thread Gstreamer_Pipeline : CAMERACORE_State_Accept FINISH!/n",argv[0]);
    return _pipeline_tate_t;
}

void CAMERACORE_Send_Signal(const gchar *method_name, int result){

    GVariant *Result = NULL;
    SignaltoDbus _signal ;
    Result = g_variant_new("(i)", signal_to_client);

    _signal.signalname = & method_name;
    _signal.result = Result;
    
    g_queue_push_tail(SingaltoDbus_queue, _signal);
  
  //  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Signal has been send to signalthread."));
    syslog(LOG_INFO,"Thread Gstreamer_Pipeline : CAMERACORE_Send_Signal has send the signal to d-bus/n",argv[0]);
  
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
  video_source = gst_element_factory_make ("imxv4l2src", "video_source");
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
  }
   
  /* Configure elements */
  g_object_set (rtp_h264, "pt", 96, NULL);
  g_object_set (udp_sink, "host", "127.0.0.1", "port", 8004, NULL);  
  g_object_set (pic_file_sink, "location", "/home/app/VTDR/capture/", NULL);//how to create radom jpeg file

  /* Link all elements that can be automatically linked because they have "Always" pads */
  gst_bin_add_many (GST_BIN (pipeline), video_source, tee, video_queue, vpu_enc, rtp_h264, udp_sink,
      snapshot_queue, pic_enc_format, pic_file_sink,NULL);
  if (gst_element_link_many (video_source, tee, NULL) != TRUE ||
      gst_element_link_many (video_queue, vpu_enc, rtp_h264, udp_sink, NULL) != TRUE ||
      gst_element_link_many (snapshot_queue, pic_enc_format,pic_file_sink, NULL) != TRUE) {
    
      g_printerr ("Elements could not be linked.\n");
      gst_object_unref (pipeline);
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
    int _pipeline_tate_t = PIPELINE_STATE_VIDEO_STOP;
    _pipeline_tate_t = CAMERACORE_State_Accept();   //There is a block here when acceptting no state data
    g_printerr ("Pipeline state has been accepted by Gstreamer.\n");
    /* According to the state to change the gstreamer pipeline */
    switch(_pipeline_tate_t){
      case  PIPELINE_STATE_VIDEO_STOP     : 
            CAMERACORE_Send_Signal(OpenCamera, SIGNAL_CAMERASTATUS_STOP);
            break; 

      case  PIPELINE_STATE_VIDEO_START    :  
            if (  gst_pad_link (tee_video_pad, video_video_pad) != GST_PAD_LINK_OK){
                  g_printerr ("Video pipeline could not be linked.\n");
                  CAMERACORE_Send_Signal(OpenCamera,SIGNAL_CAMERASTATUS_FAILED);
                  gst_object_unref (pipeline);
            };
            else{ CAMERACORE_Send_Signal(OpenCamera,SIGNAL_CAMERASTATUS_START);}
            break;

      case  PIPELINE_STATE_CAPTUREPICTURE :  
            g_object_set (video_source, "num-buffers", 1, NULL);
            if (  gst_pad_link (tee_snap_pad, snapshot_queue_pad) != GST_PAD_LINK_OK){
                  g_printerr ("Snapshot pipeline could not be linked.\n");
                  CAMERACORE_Send_Signal(CapturePicture,SIGNAL_CAPTUREPICTURE_FAILED);
                  gst_object_unref (pipeline);
            };
            else{ CAMERACORE_Send_Signal(CapturePicture,SIGNAL_CAPTUREPICTURE_SUCCESSFUL);}
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
  
}