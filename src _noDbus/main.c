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


void *Gstreamer_Pipeline(int *margc, char **margv[]);
int  *Command_Adopter(void*);
int  *FSM(void*);


extern static struct command command_t;
extern static struct pipeline_state pipeline_state_t;
   
int main(int argc, char *argv[]) {

  /* Start the nignx server*/
  system("service nginx start");

  /* Start the janus webrtc gateway*/
  system("/opt/janus/bin/janus -F /opt/janus/etc/janus/");


  pthread_t gstPipeline,commandAdopter,fsm;
  void *retval; 
  


  pthread_mutex_init(&command_t->lock,NULL);
  pthread_cond_init (&command_t->isupdated,NULL);
  pthread_cond_init (&command_t->isaccepted,NULL);

  pthread_mutex_init(&pipeline_state_t->lock,NULL);
  pthread_cond_init (&pipeline_state_t->isupdated,NULL);
  pthread_cond_init (&pipeline_state_t->isaccepted,NULL);

  //Create the threads
  if( 0 != pthread_create(&gstPipeline, NULL, Gstreamer_Pipeline, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Gstreamer_Pipeline"));
  }
  if( 0 != pthread_create(&commandAdopter, NULL, Command_Adopter, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : Command_Adopter"));
  }
  if( 0 != pthread_create(&fsm, NULL, FSM, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : FSM"));
  }

  
  //wait the end of the threads
  pthread_join(gstPipeline, &retval);
  pthread_join(commandAdopter, &retval);
  pthread_join(fsm, &retval);

  return 0;  

}