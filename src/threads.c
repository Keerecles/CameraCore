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
#include <gio/gio.h>
#include <glib.h>
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(Camera_Daemon);

#define COMMAND_0               0
#define COMMAND_1               1
#define COMMAND_2               2

#define PIPELINE_STATE_NULL     0
#define PIPELINE_STATE_VIDEO    1
#define PIPELINE_STATE_SNAPSHOT 2

extern GDBusNodeInfo *g_mIntrospectionData;
extern GDBusInterfaceVTable g_mIfaceVTable;
extern guint g_mNameRequestId;
extern guint g_mRegistrationId;


//Structure of the command
struct command_Dbus
{  
    int _command;                 // keep the command coming from Command_Adopter
    //boolean        isnewcommand;
    pthread_mutex_t lock;         // the mutex lock 
    pthread_cond_t isupdated;     // Is the _command updated
    pthread_cond_t isaccepted;
};

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


static struct command_Dbus command_dbus;
static struct command command_t;
static struct pipeline_state pipeline_state_t;




void SERVICE_handle_method_call( GDBusConnection            *connection,
                                      const gchar           *p_sender,
                                      const gchar           *p_object_path,
                                      const gchar           *p_interface_name,
                                      const gchar           *p_method_name,
                                      GVariant              *p_parameters,
                                      GDBusMethodInvocation *p_invocation,
                                      gpointer               p_user_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_handle_method_call]"),DLT_STRING("start."),DLT_STRING("p_sender = "),DLT_STRING(p_sender),DLT_STRING("p_object_path = "),DLT_STRING(p_object_path),DLT_STRING("p_interface_name = "),DLT_STRING(p_interface_name));
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_handle_method_call]"),DLT_STRING("p_method_name = "),DLT_STRING(p_method_name));
  

  command_change();
  
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_handle_method_call]"),DLT_STRING("end."));
  
  return ;
}

void SERVICE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("start. p_name = "),DLT_STRING(p_name));

  memset(&g_mIfaceVTable, 0, sizeof(g_mIfaceVTable));
  g_mIfaceVTable.method_call = SERVICE_handle_method_call;
  
  g_mIntrospectionData = g_dbus_node_info_new_for_xml(CAMERADAE_INTERFACE_XML, NULL);
  if(g_mIntrospectionData == NULL){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_node_info_new_for_xml]"));
    return;
  }

  GError *p_error = NULL;
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("CAMERADAE_OBJ_PATH = "),DLT_STRING(CAMERADAE_OBJ_PATH));
  g_mRegistrationId = g_dbus_connection_register_object( g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                                                         CAMERADAE_OBJ_PATH,
                                                         g_mIntrospectionData->interfaces[0],
                                                         &g_mIfaceVTable,
                                                         NULL,
                                                         NULL, //GDestroyNotify
                                                         &p_error);
    if(p_error){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_connection_register_object]"),DLT_STRING(p_error->message));
    g_error_free(p_error);
        return;
  }

  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("end."));
  
  return ;
}

void SERVICE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  
  return ;
}

void SERVICE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_lost_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  
  return ;
} 


void command_change(){
  void *retval;
  pthread_t commandchange_id;
  if( 0 != pthread_create(&commandchange_id, NULL, commandchange, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : commandchange"));
    return ;
  }
  pthread_join(commandchange_id, &retval);

}


void commandchange(*void){
   
  int _command_ = COMMAND_0; 
  if(!strcmp(p_method_name,"VIDEO")){
    g_variant_get(p_parameters, "(y)", &_command_);
    //update the command_dbus
    pthread_mutex_lock(&command_dbus->lock);
    
    while(pthread_cond _wait(&command_dbus->isaccepted, &command_dbus->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("dbus block to wiat the command accepted."));}
    command_dbus->_command =(_command_ == COMMAND_0)?COMMAND_0:COMMAND_1

    pthread_cond_signal(&command_dbus->isupdated);
    pthread_mutex_unlock(&command_dbus->lock);

    g_dbus_method_invocation_return_value(p_invocation, NULL);
  
  }
  
  if(!strcmp(p_method_name,"SNAP")){

    pthread_mutex_lock(&command_dbus->lock);
    
    while(pthread_cond _wait(&command_dbus->isaccepted, &command_dbus->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("dbus block to wiat the command accepted."));}
    
    command_dbus->_command = COMMAND_2;

    pthread_cond_signal(&command_dbus->isupdated);
    pthread_mutex_unlock(&command_dbus->lock);

    g_dbus_method_invocation_return_value(p_invocation, NULL);
  }

}


int Command_SendtoFSM(int _command_){
  pthread_mutex_lock(&command_t->lock);
    
  while(pthread_cond _wait(&command_t->isaccepted, &command_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("Command_Adopter block to wiat the pipeline_state accepted."));}
    
  //get the command updated
  command_t->_command = _command_;

  pthread_cond_signal(&command_t->isupdated);
  pthread_mutex_unlock(&command_t->lock);
  return 0;
}


int State_SendtoGst(int _command_){  

  pthread_mutex_lock(&pipeline_state_t->lock);
    
  while(pthread_cond _wait(&pipeline_state_t->isaccepted, &pipeline_state_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("FSM block to wiat the pipeline_state accepted."));}
    
  //get the state updated
  pipeline_state_t->_state = _command_;
  

  pthread_cond_signal(&pipeline_state_t->isupdated);
  pthread_mutex_unlock(&pipeline_state_t->lock);
  return 0;
}

int State_Accept(){
    
    int _pipeline_tate_t;
    pthread_mutex_lock(&pipeline_state_t->lock);
    
    while(pthread_cond _wait(&pipeline_state_t->isupdated, &pipeline_state_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("Gstreamer block to wiat the state updated.");}
    
    //get the state update
    _pipeline_tate_t = pipeline_state_t->_state;

    pthread_cond_signal(&pipeline_state_t->isaccepted);
    pthread_mutex_unlock(&pipeline_state_t->lock);
    return _pipeline_tate_t;
}




void *Command_Adopter(void*){
  int _command_t = COMMAND_0;  
  DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Command_Adopter start!"));
  
  for(;;){
    
    //Command_Accept() is D-bus communication
    pthread_mutex_lock(&command_dbus->lock);
    while(pthread_cond _wait(&command_dbus->isupdated, &command_dbus->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("Camera_Daemon block to wiat the command accepted."));}
      
    _command_t = command_dbus->command;

    if (!Command_SendtoFSM(_command_t){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Command has been send to FSM."));}

    //    command_dbus->isnewcommand =false;
    
    pthread_cond_signal(&command_dbus->isaccepted);
    
    pthread_mutex_unlock(&command_dbus->lock);
  }

  }
  DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Command_Adopter end!"));
}




int *FSM(void*){
  
  int _command_t = COMMAND_0;  

  for(;;){

    pthread_mutex_lock(&command_t->lock);
    
    while(pthread_cond _wait(&command_t->isupdated, &command_t->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("FSM block to wiat the command updated."));}
    
    //get the state update
    _command_t = command_t->_command;
    
    if (!State_SendtoGst(_command_t){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Pipeline state has been send to Gstreamer."));}

    pthread_cond_signal(&command_t->isaccepted);
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



