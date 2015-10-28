#include <gio/gio.h>

#define TIZEN_PREFIX          "org.ivos"

#define CAMERADAE_SERVICE          TIZEN_PREFIX ".cameradaemon"
#define CAMERADAE_IFACE            CAMERADAE_SERVICE ".Cameradaemon"
#define CAMERADAE_OBJ_PATH         "/"

#define BUF_LEN (10*(sizeof(struct inotify_event)+NAME_MAX+1))



#define CAMERADAE_INTERFACE_XML                                    \
    "<node>"                                                    \
    "  <interface name='" CAMERADAE_IFACE "'>"                     \
    "    <method name='VIDEO'>"                                \
    "      <arg type='y' name='status' direction='out'/>"
    "    </method>"                                             \
    "    <method name='SNAP'>"                               \
    "    </method>"                                             \
    "  </interface>"                                            \
    "</node>"
        


void SERVICE_handle_method_call( GDBusConnection    *connection,
                              const gchar           *p_sender,
                              const gchar           *p_object_path,
                              const gchar           *p_interface_name,
                              const gchar           *p_method_name,
                              GVariant              *p_parameters,
                              GDBusMethodInvocation *p_invocation,
                              gpointer               p_user_data);

void SERVICE_acquired_bus_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data) ;


void SERVICE_acquired_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data);

void SERVICE_lost_name_cb(GDBusConnection *p_gdus, const gchar *p_name, gpointer p_data);


