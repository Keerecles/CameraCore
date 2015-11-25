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
//#include <dlt/dlt.h>
#include "Cameradae.h" 


//DLT_DECLARE_CONTEXT(Camera_Daemon);


void  *Gstreamer_Pipeline(void* v);
void  *Command_Adapter(void* v);
void  *FSM(void* v);
void  *Send_Signal(void* v);


GDBusNodeInfo *g_mIntrospectionData=NULL;
GDBusInterfaceVTable g_mIfaceVTable;
guint g_mNameRequestId;
guint g_mRegistrationId;

GQueue *DbustoCommand_queue = NULL;
GQueue *CommandtoFSM_queue = NULL;
GQueue *FSMtoGst_queue = NULL;
GQueue *SingaltoDbus_queue = NULL;

GMainLoop *p_loop = NULL;
void *retval;

FILE *fp;
void CAMERACORE_log(FILE *file, char *log_){
    
    char *p_log[300];
    memset(p_log, '\0', 300);
    sprintf(p_log, log_);
    fwrite(p_log, strlen(p_log), 1, file);
    fflush(file);
}


int main(int argc, char *argv[]) {

  fp = fopen("/CAMERACORE_log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
  }

  /* register application */
//  DLT_REGISTER_APP("Camera_Daemon","system camera daemon");

  /* register all contexts */
//  DLT_REGISTER_CONTEXT(Camera_Daemon,"CON1","Context 1 for Logging");

  /* Start the nignx server*/
//  system("service nginx start");

  /* Start the janus webrtc gateway*/
//  system("/opt/janus/bin/janus -F /opt/janus/etc/janus/");


  CAMERACORE_log(fp,"[CAMERACORE_log]: main()  [START]\n");

  /*Create the queue to store the command*/
  DbustoCommand_queue = g_queue_new();
  CommandtoFSM_queue  = g_queue_new();
  FSMtoGst_queue      = g_queue_new();
  SingaltoDbus_queue  = g_queue_new();


  pthread_t gstPipeline,commandAdapter,fsm,sendsignal;
   
  
 
  g_mNameRequestId = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                    CAMERADAE_SERVICE,
                                    G_BUS_NAME_OWNER_FLAGS_NONE,
                                    CAMERACORE_acquired_bus_cb,
                                    CAMERACORE_acquired_name_cb,
                                    CAMERACORE_lost_name_cb,
                                    NULL,
                                    NULL); //GDestroyNotify

 

  //Create the threads
  if( 0 != pthread_create(&gstPipeline, NULL, Gstreamer_Pipeline, NULL)){
    CAMERACORE_log(fp,"[CAMERACORE_log]: main() [Failed to ctreate the thread : Gstreamer_Pipeline]\n");
    //DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Gstreamer_Pipeline"));
    return -1;
  }
  if( 0 != pthread_create(&commandAdapter, NULL, Command_Adapter, NULL)){
    CAMERACORE_log(fp,"[CAMERACORE_log]: main() [Failed to ctreate the thread : Command_Adapter]\n");
    //DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Command_Adapter"));
    return -1;
  }
  if( 0 != pthread_create(&fsm, NULL, FSM, NULL)){
    CAMERACORE_log(fp,"[CAMERACORE_log]: main() [Failed to ctreate the thread : FSM]\n");
    //DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : FSM"));
    return -1;
  }
  if( 0 != pthread_create(&sendsignal, NULL, Send_Signal, NULL)){
    CAMERACORE_log(fp,"[CAMERACORE_log]: main() [Failed to ctreate the thread : Send_Signal]\n");
    //DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Send_Signal"));
    return -1;
  }




  p_loop = g_main_loop_new(NULL, TRUE);
  if(!p_loop) {
    CAMERACORE_log(fp,"[CAMERACORE_log]: main() [Failed to create GMainLoop]\n");
    
    //DLT_LOG(OTACON1,DLT_LOG_ERROR,DLT_STRING("IVOS_CAMERACORE Failed to create GMainLoop"));
        return -2;
  }
  
  g_main_loop_run(p_loop);
  g_main_loop_unref(p_loop);

  g_queue_free (DbustoCommand_queue);
  g_queue_free (CommandtoFSM_queue);
  g_queue_free (FSMtoGst_queue);
  g_queue_free (SingaltoDbus_queue);
  
  /* unregister your contexts */
  //DLT_UNREGISTER_CONTEXT(Camera_Daemon);

  /* unregister your application */
  //DLT_UNREGISTER_APP();
  return 0;  
}
