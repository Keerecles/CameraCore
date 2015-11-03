#include <gio/gio.h>
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
#include <gst/gst.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <dlt/dlt.h>



#define TIZEN_PREFIX               "org.tizen"
#define CAMERADAE_SERVICE          TIZEN_PREFIX ".cameradaemon"
#define CAMERADAE_IFACE            CAMERADAE_SERVICE ".Cameradaemon"
#define CAMERADAE_OBJ_PATH         "/"

#define BUF_LEN (10*(sizeof(struct inotify_event)+NAME_MAX+1))



#define CAMERADAE_INTERFACE_XML                                    \
    "<node>"                                                    \
    "  <interface name='" CAMERADAE_IFACE "'>"                     \
    "    <method name='OpenCamera'>"                                \
    "      <arg type='i' name='Status' direction='out'/>"
    "    </method>"                                             \
    "    <method name='CapturePicture'>"                               \
    "    </method>"                                             \
    "  </interface>"                                            \
    "</node>"
        

#define COMMAND_VIDEO_STOP          0
#define COMMAND_VIDEO_START         1
#define COMMAND_CAPTUREPICTURE      2

#define PIPELINE_STATE_NULL         0
#define PIPELINE_STATE_VIDEO        1
#define PIPELINE_STATE_SNAPSHOT     2



void CAMERACORE_gst_method_call( GDBusConnection    *connection,
                              const gchar           *p_sender,
                              const gchar           *p_object_path,
                              const gchar           *p_interface_name,
                              const gchar           *p_method_name,
                              GVariant              *p_parameters,
                              GDBusMethodInvocation *p_invocation,
                              gpointer               p_user_data);

void CAMERACORE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data) ;


void CAMERACORE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data);

void CAMERACORE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data);

int  CAMERACORE_send_camerastatus_signal(int signal_to_client);

int  CAMERACORE_send_snapstatus_signal(int signal_to_client);