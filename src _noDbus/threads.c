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
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(Camera_Daemon);

#define COMMAND_1               1
#define COMMAND_2               2

#define PIPELINE_STATE_NULL     0
#define PIPELINE_STATE_VIDEO    1
#define PIPELINE_STATE_SNAPSHOT 2


//Structure of the command
struct command
{  
    int _command;                 // keep the command coming from Command_Adopter
    pthread_mutex_t lock;         // the mutex lock 
    pthread_cond_t isupdated;     // Is the _command updated
    pthread_cond_t isaccepted;
};

//Structure of the pipeline state
struct pipeline_state
{
    int _state;                 // keep the command coming from Command_Adopter
    pthread_mutex_t lock;         // the mutex lock 
    pthread_cond_t isupdated;     // Is the _state updated
    pthread_cond_t isaccepted;
};

//
static struct command command_t;
static struct pipeline_state pipeline_state_t;



int Command_SendtoFSM(int _command_){
  pthread_mutex_lock(&command_t->lock);
    
  while(pthread_cond _wait(&command_t->isaccepted, &command_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("Command_Adopter block to wiat the pipeline_state accepted."));}
    
  //get the command updated
  command_t->_command = _command_;

  pthread_cond_signal(&pthread_cond_t isupdated);
  pthread_mutex_unlock(&command_t->lock);
  return 0;
}


int State_SendtoGst(int _command_){  

  pthread_mutex_lock(&pipeline_state_t->lock);
    
  while(pthread_cond _wait(&pipeline_state_t->isaccepted, &pipeline_state_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("FSM block to wiat the pipeline_state accepted."));}
    
  //get the state updated
  pipeline_state_t->_state = _command_;
  

  pthread_cond_signal(&pthread_cond_t isupdated);
  pthread_mutex_unlock(&pipeline_state_t->lock);
  return 0;
}

int State_Accept(){
    
    int _pipeline_tate_t;
    pthread_mutex_lock(&pipeline_state_t->lock);
    
    while(pthread_cond _wait(&pipeline_state_t->isupdated, &pipeline_state_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("Gstreamer block to wiat the state updated.");}
    
    //get the state update
    _pipeline_tate_t = pipeline_state_t->_state;

    pthread_cond_signal(&pthread_cond_t isaccepted);
    pthread_mutex_unlock(&pipeline_state_t->lock);
    return _pipeline_tate_t;
}




void *Command_Adopter(void*){
  int _command_t = COMMAND_0;  
  
  for(;;){
    //Command_Accept() is D-bus communication
    _command_t = Command_Accept();

    if (!Command_SendtoFSM(_command_t){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Command has been send to FSM."));}
  }
}




int *FSM(void*){
  
  int _command_t = COMMAND_0;  
 

  for(;;){

    pthread_mutex_lock(&command_t->lock);
    
    while(pthread_cond _wait(&command_t->isupdated, &command_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("FSM block to wiat the command updated."));}
    
    //get the state update
    _command_t = command_t->_command;
    
    if (!State_SendtoGst(_command_t){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Pipeline state has been send to Gstreamer."));}

    pthread_cond_signal(&pthread_cond_t isaccepted);
    pthread_mutex_unlock(&command_t->lock);
  }

}





void *Gstreamer_Pipeline(int *margc, char **margv[]){
  
  GstElement *pipeline, *video_source, *tee;
  GstElement *video_queue, *vpu_enc, *rtp_h264, *udp_sink; 
  GstElement *snapshot_queue, *pic_format, *pic_file_sink;
  
  GstBus *bus;
  GstMessage *msg;
  GstPadTemplate *tee_src_pad_template;
  GstPad *tee_video_pad, *tee_snapshot_pad;
  GstPad *queue_video_pad, *queue_snapshot_pad;
   

  /* Initialize GStreamer */
  gst_init (&margc, &margv);
   
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
      case  PIPELINE_STATE_NULL     :  break; 

      case  PIPELINE_STATE_VIDEO    :  
            if ( gst_pad_link (tee_video_pad, video_video_pad) != GST_PAD_LINK_OK){
                g_printerr ("Video pipeline could not be linked.\n");
                gst_object_unref (pipeline);
                pthread_exit(NULL);
           };
           break;

      case  PIPELINE_STATE_SNAPSHOT :  
            g_object_set (video_source, "num-buffers", 1, NULL);
            if (  gst_pad_link (tee_snap_pad, snapshot_queue_pad) != GST_PAD_LINK_OK){
                  g_printerr ("Snapshot pipeline could not be linked.\n");
                  gst_object_unref (pipeline);
                  pthread_exit(NULL);
            };
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


