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
//#include <gst/gst.h>
#include <dbus/dbus.h>
//#include <dlt/dlt.h>
#include "Cameradae.h" 



GDBusNodeInfo *g_mIntrospectionData=NULL;
GDBusInterfaceVTable g_mIfaceVTable;
guint g_mNameRequestId;
guint g_mRegistrationId;

GMainLoop *p_loop = NULL;


FILE *fpp;
void print_log(FILE *file, const char *log_){
    
    char *p_log[300];
    memset(p_log, '\0', 300);
    sprintf(p_log, log_);
    fwrite(p_log, strlen(p_log), 1, file);
    fflush(file);
}

int main(int argc, char *argv[]) {


    fpp = fopen("/log.txt", "a+");                      /*创建文件用于输出log*/
    if(!fpp){
    return 1;
    }

    // memset(p_log, '\0', 300);
    // sprintf(p_log, "START the main\n");
    // fwrite(p_log, strlen(p_log), 1, fp);
    // fflush(fp);

   print_log(fpp,"START the main\n");
  g_mNameRequestId = g_bus_own_name(G_BUS_TYPE_SYSTEM,
                                    CAMERADAE_SERVICE,
                                    G_BUS_NAME_OWNER_FLAGS_NONE,
                                    CAMERACORE_acquired_bus_cb,
                                    CAMERACORE_acquired_name_cb,
                                    CAMERACORE_lost_name_cb,
                                    NULL,
                                    NULL); //GDestroyNotify


  p_loop = g_main_loop_new(NULL, TRUE);
  if(!p_loop) {
    // memset(p_log, '\0', 300);
    // sprintf(p_log, "IVOS_CAMERACORE Failed to create GMainLoop\n");
    // fwrite(p_log, strlen(p_log), 1, fp);
    // fflush(fp);
        return -2;
  }
  
  g_main_loop_run(p_loop);
  g_main_loop_unref(p_loop);

 
  return 0;  
}
