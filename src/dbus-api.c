#include "Cameradae.h" 


extern GDBusNodeInfo *g_mIntrospectionData;
extern GDBusInterfaceVTable g_mIfaceVTable;
extern guint g_mNameRequestId;
extern guint g_mRegistrationId;

extern GQueue *DbustoCommand_queue;

DLT_IMPORT_CONTEXT(Camera_Daemon);
void CAMERACORE_gst_method_call( GDBusConnection            *connection,
                                      const gchar           *p_sender,
                                      const gchar           *p_object_path,
                                      const gchar           *p_interface_name,
                                      const gchar           *p_method_name,
                                      GVariant              *p_parameters,
                                      GDBusMethodInvocation *p_invocation,
                                      gpointer               p_user_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("start."),DLT_STRING("p_sender = "),DLT_STRING(p_sender),DLT_STRING("p_object_path = "),DLT_STRING(p_object_path),DLT_STRING("p_interface_name = "),DLT_STRING(p_interface_name));
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("p_method_name = "),DLT_STRING(p_method_name));
  
  g_dbus_method_invocation_return_value(p_invocation, NULL);
  command_change();
  
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("end."));
  
  return ;
}

void CAMERACORE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("start. p_name = "),DLT_STRING(p_name));

  memset(&g_mIfaceVTable, 0, sizeof(g_mIfaceVTable));
  g_mIfaceVTable.method_call = CAMERACORE_gst_method_call;
  
  g_mIntrospectionData = g_dbus_node_info_new_for_xml(CAMERADAE_INTERFACE_XML, NULL);
  if(g_mIntrospectionData == NULL){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_node_info_new_for_xml]"));
    return;
  }

  GError *p_error = NULL;
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("CAMERADAE_OBJ_PATH = "),DLT_STRING(CAMERADAE_OBJ_PATH));
  g_mRegistrationId = g_dbus_connection_register_object( g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                                                         CAMERADAE_OBJ_PATH,
                                                         g_mIntrospectionData->interfaces[0],
                                                         &g_mIfaceVTable,
                                                         NULL,
                                                         NULL, //GDestroyNotify
                                                         &p_error);
    if(p_error){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_connection_register_object]"),DLT_STRING(p_error->message));
    g_error_free(p_error);
        return;
  }

  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("end."));
  
  return ;
}

void CAMERACORE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  
  return ;
}

void CAMERACORE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_lost_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  
  return ;
} 




void command_change(){
    
    int _command_ = 0;
    
    DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" D-bus_api  start."));
    g_variant_get(p_parameters, "(i)", &_command_);
    if (_command_ == 1){
      g_queue_push_tail(DbustoCommand_queue, COMMAND_VIDEO_START);
      DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_VIDEO_START into DbustoCommand_queue"));
    }
      else if(_command_ == 0){
        g_queue_push_tail(DbustoCommand_queue, COMMAND_VIDEO_STOP );
        DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_VIDEO_STOP into DbustoCommand_queue"));
      }
  } 
  if(!strcmp(p_method_name,"CapturePicture")){ g_mIntrospectionData 
    g_queue_push_tail(DbustoCommand_queue, COMMAND_CAPTUREPICTURE);
    DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_CAPTUREPICTURE into DbustoCommand_queue"));
  }
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("D-bus_api end. "));
}