#include "Cameradae.h" 


extern GQueue *DbustoCommand_queue ;
extern GQueue *CommandtoFSM_queue  ;


DLT_IMPORT_CONTEXT(Camera_Daemon);

void *Command_Adapter(void*){
  int _command_t = COMMAND_VIDEO_STOP;  
  DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Command_Adapter start!"));
  
  while(1){
    
    /*Get the command from the D-bus*/
    while(g_queue_is_empty(DbustoCommand_queue)){}

    _command_t = g_queue_pop_head(DbustoCommand_queue);
  
    g_queue_push_tail(CommandtoFSM_queue, _command_t);
    
    DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" Command has been send to FSM."));
    
  }
  DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Command_Adapter end!"));
}
