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
#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(Camera_Daemon);


void *Gstreamer_Pipeline(int *margc, char **margv[]);
int  *Command_Adopter(void*);
int  *FSM(void*);


GDBusNodeInfo *g_mIntrospectionData=NULL;
GDBusInterfaceVTable g_mIfaceVTable;
guint g_mNameRequestId;
guint g_mRegistrationId;

extern static struct command command_dbus;
extern static struct command command_t;
extern static struct pipeline_state pipeline_state_t;

GMainLoop *p_loop = NULL;

int main(int argc, char *argv[]) {

  /* register application */
  DLT_REGISTER_APP("Camera_Daemon","system camera daemon");

  /* register all contexts */
  DLT_REGISTER_CONTEXT(Camera_Daemon,"CON1","Context 1 for Logging");

  /* Start the nignx server*/
  system("service nginx start");

  /* Start the janus webrtc gateway*/
  system("/opt/janus/bin/janus -F /opt/janus/etc/janus/");

  command_dbus->isnewcommand= false;

  pthread_t gstPipeline,commandAdopter,fsm;
  void *retval; 

  
  gs_su_info.e_status = E_IVOS_SERVICE_SU_NULL;
  g_mNameRequestId = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                    CAMERADAE_SERVICE,
                                    G_BUS_NAME_OWNER_FLAGS_NONE,
                                    SERVICE_acquired_bus_cb,
                                    SERVICE_acquired_name_cb,
                                    SERVICE_lost_name_cb,
                                    NULL,
                                    NULL); //GDestroyNotify

  pthread_mutex_init(&command_dbus->lock,NULL);
  pthread_cond_init (&command_dbus->isupdated,NULL);
  pthread_cond_init (&command_dbus->isaccepted,NULL);
  
  pthread_mutex_init(&command_t->lock,NULL);
  pthread_cond_init (&command_t->isupdated,NULL);
  pthread_cond_init (&command_t->isaccepted,NULL);

  pthread_mutex_init(&pipeline_state_t->lock,NULL);
  pthread_cond_init (&pipeline_state_t->isupdated,NULL);
  pthread_cond_init (&pipeline_state_t->isaccepted,NULL);

  //Create the threads
  if( 0 != pthread_create(&gstPipeline, NULL, Gstreamer_Pipeline, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Gstreamer_Pipeline"));
    return -1;
  }
  if( 0 != pthread_create(&commandAdopter, NULL, Command_Adopter, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Command_Adopter"));
    return -1;
  }
  if( 0 != pthread_create(&fsm, NULL, FSM, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : FSM"));
    return -1;
  }

  
  /*wait the end of the threads */
  pthread_join(gstPipeline, &retval);
  pthread_join(commandAdopter, &retval);
  pthread_join(fsm, &retval);


  p_loop = g_main_loop_new(NULL, TRUE);
  if(!p_loop) {
    DLT_LOG(OTACON1,DLT_LOG_ERROR,DLT_STRING("IVOS_SU Failed to create GMainLoop"));
        return -2;
  }
  
  g_main_loop_run(p_loop);
  g_main_loop_unref(p_loop);


  /* unregister your contexts */
  DLT_UNREGISTER_CONTEXT(OTACON1);

  /* unregister your application */
  DLT_UNREGISTER_APP();
  return 0;  
}