#include "Cameradae.h" 


extern GDBusNodeInfo *g_mIntrospectionData;
extern GDBusInterfaceVTable g_mIfaceVTable;
extern guint g_mNameRequestId;
extern guint g_mRegistrationId;

extern GQueue *DbustoCommand_queue;

//DLT_IMPORT_CONTEXT(Camera_Daemon);
void CAMERACORE_gst_method_call( GDBusConnection            *connection,
                                      const gchar           *p_sender,
                                      const gchar           *p_object_path,
                                      const gchar           *p_interface_name,
                                      const gchar           *p_method_name,
                                      GVariant              *p_parameters,
                                      GDBusMethodInvocation *p_invocation,
                                      gpointer               p_user_data)
{
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("start."),DLT_STRING("p_sender = "),DLT_STRING(p_sender),DLT_STRING("p_object_path = "),DLT_STRING(p_object_path),DLT_STRING("p_interface_name = "),DLT_STRING(p_interface_name));
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("p_method_name = "),DLT_STRING(p_method_name));
  syslog(LOG_INFO,"CAMERACORE_gst_method_call START!/n",argv[0]);

  g_dbus_method_invocation_return_value(p_invocation, NULL);
  CAMERACORE_command_change(p_parameters);
  syslog(LOG_INFO,"CAMERACORE_gst_method_call END!/n",argv[0]);
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("end."));
  
  return ;
}

void CAMERACORE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("start. p_name = "),DLT_STRING(p_name));
  syslog(LOG_INFO,"CAMERACORE_acquired_bus_cb RUN!/n",argv[0]);
  memset(&g_mIfaceVTable, 0, sizeof(g_mIfaceVTable));
  g_mIfaceVTable.method_call = CAMERACORE_gst_method_call;
  
  g_mIntrospectionData = g_dbus_node_info_new_for_xml(CAMERADAE_INTERFACE_XML, NULL);
  if(g_mIntrospectionData == NULL){
//    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_node_info_new_for_xml]"));
  syslog(LOG_INFO,"CAMERACORE_acquired_bus_cb: fail g_dbus_node_info_new_for_xml!/n",argv[0]);
    return;
  }

  GError *p_error = NULL;
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("CAMERADAE_OBJ_PATH = "),DLT_STRING(CAMERADAE_OBJ_PATH));
  
  g_mRegistrationId = g_dbus_connection_register_object( g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                                                         CAMERADAE_OBJ_PATH,
                                                         g_mIntrospectionData->interfaces[0],
                                                         &g_mIfaceVTable,
                                                         NULL,
                                                         NULL, //GDestroyNotify
                                                         &p_error);
    if(p_error){
//    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_connection_register_object]"),DLT_STRING(p_error->message));
    syslog(LOG_ERR,"CAMERACORE_acquired_bus_cb: fail g_dbus_connection_register_object!/n",argv[0]);
    g_error_free(p_error);
    return;
  }

//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("end."));
  syslog(LOG_INFO,"CAMERACORE_acquired_bus_cb: END/n",argv[0]);
  return ;
}

void CAMERACORE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  syslog(LOG_INFO,"CAMERACORE_acquired_name_cb: /n",argv[0]);
  return ;
}

void CAMERACORE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_lost_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  syslog(LOG_INFO,"CAMERACORE_lost_name_cb: !/n",argv[0]);
  return ;
} 


void CAMERACORE_command_change(GVariant *parameters){
    
    int _command_ = COMMAND_VIDEO_STOP;
    
//  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" D-bus_api  start."));
    syslog(LOG_INFO," CAMERACORE_command_change : start./n",argv[0]);
    if(!strcmp(p_method_name,"OpenCamera")){ 
      g_variant_get(parameters, "(i)", &_command_);
      g_queue_push_tail(DbustoCommand_queue, _command_);

      if (_command_ == COMMAND_VIDEO_START){
//        DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_VIDEO_START into DbustoCommand_queue"));
          syslog(LOG_INFO," CAMERACORE_command_change: push COMMAND_VIDEO_START into DbustoCommand_queue/n",argv[0]);
      }
        else if(_command_ == COMMAND_VIDEO_STOP){
//          DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_VIDEO_STOP into DbustoCommand_queue"));
          syslog(LOG_INFO," CAMERACORE_command_change: push COMMAND_VIDEO_STOP into DbustoCommand_queue/n",argv[0]);
        }
    }
    
    if(!strcmp(p_method_name,"CapturePicture")){ 
      g_queue_push_tail(DbustoCommand_queue, COMMAND_CAPTUREPICTURE);
//      DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_CAPTUREPICTURE into DbustoCommand_queue"));
        syslog(LOG_INFO," CAMERACORE_command_change: push COMMAND_CAPTUREPICTURE into DbustoCommand_queue/n",argv[0]);
    }
//    DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("D-bus_api end. "));
}