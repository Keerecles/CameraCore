void SERVICE_handle_method_call( GDBusConnection            *connection,
                                      const gchar           *p_sender,
                                      const gchar           *p_object_path,
                                      const gchar           *p_interface_name,
                                      const gchar           *p_method_name,
                                      GVariant              *p_parameters,
                                      GDBusMethodInvocation *p_invocation,
                                      gpointer               p_user_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_handle_method_call]"),DLT_STRING("start."),DLT_STRING("p_sender = "),DLT_STRING(p_sender),DLT_STRING("p_object_path = "),DLT_STRING(p_object_path),DLT_STRING("p_interface_name = "),DLT_STRING(p_interface_name));
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_handle_method_call]"),DLT_STRING("p_method_name = "),DLT_STRING(p_method_name));
  

  command_change();
  
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_handle_method_call]"),DLT_STRING("end."));
  
  return ;
}

void SERVICE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("start. p_name = "),DLT_STRING(p_name));

  memset(&g_mIfaceVTable, 0, sizeof(g_mIfaceVTable));
  g_mIfaceVTable.method_call = SERVICE_handle_method_call;
  
  g_mIntrospectionData = g_dbus_node_info_new_for_xml(CAMERADAE_INTERFACE_XML, NULL);
  if(g_mIntrospectionData == NULL){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_node_info_new_for_xml]"));
    return;
  }

  GError *p_error = NULL;
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("CAMERADAE_OBJ_PATH = "),DLT_STRING(CAMERADAE_OBJ_PATH));
  g_mRegistrationId = g_dbus_connection_register_object( g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL),
                                                         CAMERADAE_OBJ_PATH,
                                                         g_mIntrospectionData->interfaces[0],
                                                         &g_mIfaceVTable,
                                                         NULL,
                                                         NULL, //GDestroyNotify
                                                         &p_error);
    if(p_error){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("[fail g_dbus_connection_register_object]"),DLT_STRING(p_error->message));
    g_error_free(p_error);
        return;
  }

  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_bus_cb]"),DLT_STRING("end."));
  
  return ;
}

void SERVICE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_acquired_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  
  return ;
}

void SERVICE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data)
{
  DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("[loc:SERVICE_lost_name_cb]"),DLT_STRING("called. p_name = "),DLT_STRING(p_name));
  
  return ;
} 


void command_change(){
  void *retval;
  pthread_t commandchange_id;
  if( 0 != pthread_create(&commandchange_id, NULL, commandchange, NULL)){
    DLT_LOG(Camera_Daemon,DLT_LOG_ERROR,DLT_STRING("Failed to ctreate the thread : commandchange"));
    return ;
  }
  pthread_join(commandchange_id, &retval);

}


void commandchange(*void){
   
  int _command_ = COMMAND_0; 
  if(!strcmp(p_method_name,"VIDEO")){
    g_variant_get(p_parameters, "(y)", &_command_);
    //update the command_dbus
    pthread_mutex_lock(&command_dbus->lock);
    
    while(pthread_cond _wait(&command_dbus->isaccepted, &command_dbus->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("dbus block to wiat the command accepted."));}
    command_dbus->_command =(_command_ == COMMAND_0)?COMMAND_0:COMMAND_1

    pthread_cond_signal(&command_dbus->isupdated);
    pthread_mutex_unlock(&command_dbus->lock);

    g_dbus_method_invocation_return_value(p_invocation, NULL);
  
  }
  
  if(!strcmp(p_method_name,"SNAP")){

    pthread_mutex_lock(&command_dbus->lock);
    
    while(pthread_cond _wait(&command_dbus->isaccepted, &command_dbus->lock)){DLT_LOG(Camera_Daemon,DLT_LOG_INFO,DLT_STRING("dbus block to wiat the command accepted."));}
    
    command_dbus->_command = COMMAND_2;

    pthread_cond_signal(&command_dbus->isupdated);
    pthread_mutex_unlock(&command_dbus->lock);

    g_dbus_method_invocation_return_value(p_invocation, NULL);
  }

}