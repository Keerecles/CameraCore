#include "Cameradae.h"


extern GQueue *SingaltoDbus_queue ;
extern FILE *fp ;
//DLT_IMPORT_CONTEXT(Camera_Daemon) ;

void  *Send_Signal(void* v){

  SignaltoDbus *signal_t ;
  //DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Send_Signal start "));
  CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Send_Signal  [START]\n");

  while(1){
    /*Get the signal from the Gst_pipeline*/
    while(g_queue_is_empty(SingaltoDbus_queue)){};

    signal_t = g_queue_pop_head(SingaltoDbus_queue);
  	/* 向dbus发送signal函数 */
  	CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Send_Signal  [Send_Signal: g_dbus_connection_emit_signal() start]\n");
    //DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Send_Signal g_dbus_connection_emit_signal start"));
  	g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                  NULL,   /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
                  CAMERADAE_OBJ_PATH,
                  CAMERADAE_IFACE,
                  signal_t->signalname,
                  g_variant_new("(v)", signal_t->result),
                  NULL);
  	CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Send_Signal  [Send_Signal: g_dbus_connection_emit_signal() end]\n");
    //DLT_LOG(Camera_Daemon, DLT_LOG_INFO, DLT_STRING("Send_Signal g_dbus_connection_emit_signal end"));
  }
  CAMERACORE_log(fp,"[CAMERACORE_log]: Thread Send_Signal  [END]\n");  
}
