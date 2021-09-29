#include "get_connection.hpp"

void get_ip_address(NMClient *client)
{
    GError *error = NULL;
    NMRemoteConnection *rem_con = NULL;
    NMSettingIPConfig *s_ip4;
    NMConnection *new_connection;
    NMIPAddress *ip_address;
    const char *address;
    const char *gateway;
    const char *method;
    int prefix;

    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    new_connection = nm_simple_connection_new_clone(NM_CONNECTION(rem_con));
    s_ip4 = nm_connection_get_setting_ip4_config(new_connection);
    method = nm_setting_ip_config_get_method(s_ip4);

    g_print("method: %s\n", method);

    if (!strcmp(method, "manual")) {
        ip_address = nm_setting_ip_config_get_address(s_ip4, 0);
        prefix = nm_ip_address_get_prefix(ip_address);
        address = nm_ip_address_get_address(ip_address);
        gateway = nm_setting_ip_config_get_gateway(s_ip4);
        g_print("ip_address: %s/%d\n", address, prefix);
        g_print("gateway: %s\n", gateway);
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

    get_ip_address(client);

    // Clean up
    g_object_unref(client);

    return 0;
}
