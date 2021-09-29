#include "get_connection.hpp"

void get_device_type(NMClient *client)
{
    NMActiveConnection *active_con;
    NMDevice *device;
    const GPtrArray *devices;
    const char *name, *con_id, *type;

    devices = nm_client_get_devices(client);

    for (int i = 0; i < devices->len; i++) {
        device = (NMDevice *)devices->pdata[i];
        name = nm_device_get_iface(device);

        switch (nm_device_get_device_type(device)) {
            case NM_DEVICE_TYPE_ETHERNET:
                type = "ethernet";
                break;

            case NM_DEVICE_TYPE_WIFI:
                type = "wifi";
                break;
            default:
                continue;
        }
        g_print("network interface [%s], device type is: %s\n", name, type);
    }
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

    get_device_type(client);

    // Clean up
    g_object_unref(client);

    return 0;
}
