#include "Cameradae.h"


extern GQueue *CommandtoFSM_queue;
extern GQueue *FSMtoGst_queue ;

//DLT_IMPORT_CONTEXT(Camera_Daemon);


void *FSM(void*){
  
  int _command_t = COMMAND_VIDEO_STOP;  

  while(1){
    
    /*Get the command from the command-adapter*/
    while(g_queue_is_empty(CommandtoFSM_queue)){}

    _command_t = g_queue_pop_head(CommandtoFSM_queue);
    
    g_queue_push_tail(FSMtoGst_queue, _command_t);
  
   // DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Command has been send to FSM."));
    syslog(LOG_INFO,"Thread FSM : Command has been send to FSM./n",argv[0]);
  
  }
 // DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("FSM end!"));
  syslog(LOG_INFO,"Thread FSM : thread FSM END./n",argv[0]);
}