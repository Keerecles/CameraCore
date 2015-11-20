#include "Cameradae.h" 


extern GDBusNodeInfo *g_mIntrospectionData;
extern GDBusInterfaceVTable g_mIfaceVTable;
extern guint g_mNameRequestId;
extern guint g_mRegistrationId;

extern GQueue *DbustoCommand_queue;
extern FILE *fp;

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
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("start."),DLT_STRING("p_sender = "),DLT_STRING(p_sender),DLT_STRING("p_object_path = "),DLT_STRING(p_object_path),DLT_STRING("p_interface_name = "),DLT_STRING(p_interface_name));
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("p_method_name = "),DLT_STRING(p_method_name));
  CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_gst_method_call [START]\n");
  CAMERACORE_log(fp,"[CAMERACORE_log]:In CAMERACORE_gst_method_call [START command_change()]\n");
  g_dbus_method_invocation_return_value(p_invocation, NULL);
  command_change(p_method_name,p_parameters);
  CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_gst_method_call [END]\n");
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_gst_method_call]"),DLT_STRING("end."));
  
  return ;
}

void CAMERACORE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("start. p_name = "),DLT_STRING(p_name));

  CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_acquired_bus_cb [START]\n");
  memset(&g_mIfaceVTable, 0, sizeof(g_mIfaceVTable));
  g_mIfaceVTable.method_call = CAMERACORE_gst_method_call;
  
  g_mIntrospectionData = g_dbus_node_info_new_for_xml(CAMERADAE_INTERFACE_XML, NULL);
  if(g_mIntrospectionData == NULL){
    //DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_node_info_new_for_xml]"));
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_acquired_bus_cb [Fail g_dbus_node_info_new_for_xml]\n");
    return;
  }

  GError *p_error = NULL;
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("CAMERADAE_OBJ_PATH = "),DLT_STRING(CAMERADAE_OBJ_PATH));
  
  g_mRegistrationId = g_dbus_connection_register_object( g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                                                         CAMERADAE_OBJ_PATH,
                                                         g_mIntrospectionData->interfaces[0],
                                                         &g_mIfaceVTable,
                                                         NULL,
                                                         NULL, //GDestroyNotify
                                                         &p_error);
    if(p_error){
    //DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_connection_register_object]"),DLT_STRING(p_error->message));
    CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_acquired_bus_cb [Fail g_dbus_connection_register_object]\n");
    CAMERACORE_log(fp,p_error->message);
    g_error_free(p_error);
    return;
  }

  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_bus_cb]"),DLT_STRING("end."));
  CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_acquired_bus_cb [END]\n");
  return ;
}

void CAMERACORE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_acquired_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_acquired_name_cb [called. p_name =]");
  CAMERACORE_log(fp,p_name);
  CAMERACORE_log(fp,"\n");
  return ;
}

void CAMERACORE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  CAMERACORE_log(fp,"[CAMERACORE_log]:CAMERACORE_lost_name_cb [called. p_name =]");
  CAMERACORE_log(fp,p_name);
  CAMERACORE_log(fp,"\n");
  //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:CAMERACORE_lost_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  return ;
} 




void command_change(const gchar  *p_method_name, GVariant *parameters){
    
    int _command_ = COMMAND_VIDEO_STOP;
    CAMERACORE_log(fp,"[CAMERACORE_log]:command_change [START]\n");
    //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" D-bus_api  start."));
    
    if(!strcmp(p_method_name,"OpenCamera")){ 
     
      g_variant_get(parameters, "(i)", &_command_);
      g_queue_push_tail(DbustoCommand_queue, _command_);

      if (_command_ == COMMAND_VIDEO_START){
        CAMERACORE_log(fp,"[CAMERACORE_log]:command_change [Push COMMAND_VIDEO_START into DbustoCommand_queue]\n");
        //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_VIDEO_START into DbustoCommand_queue"));
      }
        else if(_command_ == COMMAND_VIDEO_STOP){
        CAMERACORE_log(fp,"[CAMERACORE_log]:command_change [Push COMMAND_VIDEO_STOP into DbustoCommand_queue]\n");
        //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_VIDEO_STOP into DbustoCommand_queue"));
        }
    }
    
    if(!strcmp(p_method_name,"CapturePicture")){ 
      g_queue_push_tail(DbustoCommand_queue, COMMAND_CAPTUREPICTURE);
      CAMERACORE_log(fp,"[CAMERACORE_log]:command_change [Push COMMAND_CAPTUREPICTURE into DbustoCommand_queue]\n");
      //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING(" push COMMAND_CAPTUREPICTURE into DbustoCommand_queue"));
    }
      CAMERACORE_log(fp,"[CAMERACORE_log]:command_change [END]\n");
    //DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("D-bus_api end. "));
}