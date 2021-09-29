#include "get_connection.hpp"

void get_active(NMClient *client, const char *device_name)
{
    NMDevice *device;
    NMActiveConnection *active_con;
    NMSettingWireless *s_wireless;
    NMConnection *new_connection;
    GBytes *active_ssid;
    const char *name;
    const char *ssid;

    device = nm_client_get_device_by_iface(client, device_name);
    active_con = nm_device_get_active_connection(device);
    new_connection = nm_simple_connection_new_clone(NM_CONNECTION(nm_active_connection_get_connection(active_con)));
    name = nm_connection_get_id(new_connection);
    s_wireless = nm_connection_get_setting_wireless(new_connection);
    active_ssid = nm_setting_wireless_get_ssid(s_wireless);
    ssid = nm_utils_ssid_to_utf8((guint8 *)g_bytes_get_data(active_ssid, NULL),
                                 g_bytes_get_size(active_ssid));
    g_print("Current active connection of device [%s]: %s \n", device_name, name);
    g_print("ssid of connection [%s]: %s", name, ssid);
}

int main(int argc, char *argv[])
{
    NMClient *client;
    GError *error = NULL;

    // Connect to NetworkManager
    client = nm_client_new(NULL, &error);
    if (!client) {
        g_message("Error: Could not connect to NetworkManager: %s.", error->message);
        g_error_free(error);
        return 1;
    }

    get_active(client, "wlx08beac0c85d9");

    // Clean up
    g_object_unref(client);

    return 0;
}
