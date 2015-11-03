#include "Cameradae.h"
#include "thread.h"

extern GQueue *CommandtoFSM_queue;
extern GQueue *FSMtoGst_queue ;

DLT_IMPORT_CONTEXT(Camera_Daemon);


int *FSM(void*){
  
  int _command_t = COMMAND_VIDEO_STOP;  

  while(1){
    
    /*Get the command from the command-adapter*/
    while(g_queue_is_empty(CommandtoFSM_queue)){

      DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("CommandtoFSM_queue is empty! FSM block here!"));
    
    }

    _command_t = g_queue_pop_head(CommandtoFSM_queue);
    
    if(_command_t==NULL){
      DLT_LOG(Camera_Daemon, DLT_LOG_ERROR, DLT_STRING("Pop the empty CommandtoFSM_queue ! "));
      pthread_exit(retval);
    }
    else{
      g_queue_push_tail(FSMtoGst_queue, _command_t);
      DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Command has been send to FSM."));}
    }
  }
  DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("FSM end!"));
}