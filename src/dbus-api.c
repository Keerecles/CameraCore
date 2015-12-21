#include "Cameradae.h" 

extern GDBusNodeInfo *g_mIntrospectionData;
extern GDBusInterfaceVTable g_mIfaceVTable;
extern guint g_mRegistrationId;

extern FILE *fpp;
void CAMERACORE_gst_method_call( GDBusConnection            *connection,
                                      const gchar           *p_sender,
                                      const gchar           *p_object_path,
                                      const gchar           *p_interface_name,
                                      const gchar           *p_method_name,
                                      GVariant              *p_parameters,
                                      GDBusMethodInvocation *p_invocation,
                                      gpointer               p_user_data)
{
  char *p_log[300];

  FILE *fp = fopen("/log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
    }
    print_log(fpp,"LOOOOOOOOOG\n");

    memset(p_log, '\0', 300);
    sprintf(p_log, "START the CAMERACORE_gst_method_call\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);

  g_dbus_method_invocation_return_value(p_invocation, NULL);
  command_change(p_method_name,p_parameters);
  
   memset(p_log, '\0', 300);
    sprintf(p_log, "END the CAMERACORE_gst_method_call\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
  
  return ;
}

void CAMERACORE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
    char *p_log[300];
    FILE *fp = fopen("/log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
    }
    memset(p_log, '\0', 300);
    sprintf(p_log, "START the CAMERACORE_acquired_bus_cb\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
    print_log(fpp,"LOOOOOOOOOG\n");

  memset(&g_mIfaceVTable, 0, sizeof(g_mIfaceVTable));
  g_mIfaceVTable.method_call = CAMERACORE_gst_method_call;
  
  g_mIntrospectionData = g_dbus_node_info_new_for_xml(CAMERADAE_INTERFACE_XML, NULL);
  if(g_mIntrospectionData == NULL){
    
    memset(p_log, '\0', 300);
    sprintf(p_log, "IN CAMERACORE_acquired_bus_cb: Fail g_dbus_node_info_new_for_xml\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
    return;
  }

  GError *p_error = NULL;
  g_mRegistrationId = g_dbus_connection_register_object( g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                                                         CAMERADAE_OBJ_PATH,
                                                         g_mIntrospectionData->interfaces[0],
                                                         &g_mIfaceVTable,
                                                         NULL,
                                                         NULL, //GDestroyNotify
                                                         &p_error);
    if(p_error){
    memset(p_log, '\0', 300);
    sprintf(p_log, "IN CAMERACORE_acquired_bus_cb: Fail g_dbus_connection_register_object\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
    g_error_free(p_error);
    return;
  }

  memset(p_log, '\0', 300);
    sprintf(p_log, "END the CAMERACORE_acquired_bus_cb\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
  
  return ;
}

void CAMERACORE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
    char *p_log[300];
    FILE *fp = fopen("/log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
    }
    memset(p_log, '\0', 300);
    sprintf(p_log, "START CAMERACORE_acquired_name_cb\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
        print_log(fpp,"LOOOOOOOOOG\n");
  return ;
}

void CAMERACORE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  
  char *p_log[300];
    FILE *fp = fopen("/log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
    }
    memset(p_log, '\0', 300);
    sprintf(p_log, "START CAMERACORE_lost_name_cb\n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
        print_log(fpp,"LOOOOOOOOOG\n");
  return ;
} 

void command_change(const gchar *p_method_name, GVariant *parameters){
    
    int _command_ = COMMAND_VIDEO_STOP;
    GVariant *status = NULL;
    char *p_log[300];
    FILE *fp = fopen("/log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fp){
    return 1;
    }
    memset(p_log, '\0', 300);
    sprintf(p_log, "START command_change \n");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);

    
    // if(!strcmp(p_method_name,"OpenCamera")){ 

    // g_variant_get(parameters, "(v)", &status);
    // _command_ = g_variant_get_int32(status);

    //   if (_command_ == COMMAND_VIDEO_START){
       
    //     g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
    //               TIZEN_PREFIX,   /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
    //               CAMERADAE_OBJ_PATH,
    //               CAMERADAE_SERVICE,
    //               "CameraStatus",
    //               SIGNAL_CAMERASTATUS_START,
    //               NULL);
    //     memset(p_log, '\0', 300);
    // sprintf(p_log, "In command_change :Send Signal OpenCamera COMMAND_CAMERASTATUS_START \n");
    // fwrite(p_log, strlen(p_log), 1, fp);
    // fflush(fp);
    //   }
    //     else if(_command_ == COMMAND_VIDEO_STOP){ 
    //     g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
    //               TIZEN_PREFIX,      /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
    //               CAMERADAE_OBJ_PATH,
    //               CAMERADAE_SERVICE,
    //               "CameraStatus",
    //               SIGNAL_CAMERASTATUS_STOP,
    //               NULL);
    //     memset(p_log, '\0', 300);
    // sprintf(p_log, "In command_change :Send Signal OpenCamera COMMAND_CAMERASTATUS_STOP \n");
    // fwrite(p_log, strlen(p_log), 1, fp);
    // fflush(fp);
    //     }
    // }
    
    if(!strcmp(p_method_name,"CapturePicture")){ 

    memset(p_log, '\0', 300);
    sprintf(p_log, "method_call name = CapturePicture \n ");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);


    memset(p_log, '\0', 300);
    sprintf(p_log, " START send signal : g_dbus_connection_emit_signal \n ");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);


      g_dbus_connection_emit_signal(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                  NULL,      /* the unique bus name for the destination for the signal or NULL to emit to all listeners */
                  CAMERADAE_OBJ_PATH,
                  CAMERADAE_IFACE,
                  "SnapResult",
                  g_variant_new("(i)", 1),
                  NULL);

    memset(p_log, '\0', 300);
    sprintf(p_log, "In command_change :Send Signal CapturePicture SIGNAL_CAPTUREPICTURE_SUCCESSFUL\n ");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
     memset(p_log, '\0', 300);
    sprintf(p_log, "In command_change :system gst \n ");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
    system("gst-launch-1.0 videotestsrc ! autovideosink");
    }

    memset(p_log, '\0', 300);
    sprintf(p_log, "D-bus_api end ");
    fwrite(p_log, strlen(p_log), 1, fp);
    fflush(fp);
}