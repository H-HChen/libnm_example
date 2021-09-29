#include "get_connection.hpp"

typedef struct {
    NMClient  *client;
    GMainLoop *loop;
}WifiModifyData;

void reapply_cb(GObject *     device,
                GAsyncResult *result,
                gpointer      user_data)
{
    NMDevice *wifi = NM_DEVICE(device);
    GError *error = NULL;

    nm_device_reapply_finish(wifi, result, &error);
    if (error) {
        g_print("Error reapply connection: %s", error->message);
        g_error_free(error);
    } else {
        g_print(("Connection successfully reapplied.\n"));
    }

    g_main_loop_quit((GMainLoop *)user_data);
}

void connection_reapply(NMClient *client, GMainLoop *loop)
{
    const char *name;
    NMDevice *device;
    NMConnection *new_connection;
    NMActiveConnection *active_con;

    device = nm_client_get_device_by_iface(client, "wlp1s0");
    active_con = nm_device_get_active_connection(device);
    if (active_con) {
        new_connection = nm_simple_connection_new_clone(NM_CONNECTION(nm_active_connection_get_connection(active_con)));
        name = nm_connection_get_id(new_connection);
        if (!strcmp(name, "RMTClient")) {
            nm_device_reapply_async(device, new_connection, 0, 0, NULL, reapply_cb, loop);
        } else {
            g_main_loop_quit(loop);
        }
    }
}

void modify_connection_cb(GObject *     connection,
                          GAsyncResult *result,
                          gpointer      user_data)
{
    GError *error = NULL;
    WifiModifyData *data = (WifiModifyData*)user_data;

    if (!nm_remote_connection_commit_changes_finish(NM_REMOTE_CONNECTION(connection),
                                                    result, &error)) {
        printf(("Error: Failed to modify connection '%s': %s"),
               nm_connection_get_id(NM_CONNECTION(connection)),
               error->message);
        g_main_loop_quit(data->loop);
    } else {
        printf(("Connection '%s' (%s) successfully modified.\n"),
               nm_connection_get_id(NM_CONNECTION(connection)),
               nm_connection_get_uuid(NM_CONNECTION(connection)));
        connection_reapply(data->client, data->loop);
    }
}

void update_ip4(NMClient *client, GMainLoop *loop, const char *method, bool gateway)
{
    NMRemoteConnection *rem_con = NULL;
    gboolean temporary = FALSE;
    NMSettingIPConfig *s_ip4;
    NMConnection *new_connection;
    NMIPAddress *ip_address;
    WifiModifyData *wifi_modify_data;

    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    new_connection = nm_simple_connection_new_clone(NM_CONNECTION(rem_con));
    s_ip4 = (NMSettingIPConfig *)nm_setting_ip4_config_new();
    wifi_modify_data = g_slice_new(WifiModifyData);
    *wifi_modify_data = (WifiModifyData) {
        .client = client,
        .loop = loop,
    };

    nm_connection_remove_setting(new_connection, NM_TYPE_SETTING_IP4_CONFIG);

    if (!strcmp(method, "auto")) {
        g_object_set(s_ip4,
                     NM_SETTING_IP_CONFIG_METHOD,
                     NM_SETTING_IP4_CONFIG_METHOD_AUTO,
                     NULL);
    } else if (!strcmp(method, "manual")) {
        ip_address = nm_ip_address_new(AF_INET, "192.168.50.26", 24, NULL);
        g_object_set(s_ip4,
                     NM_SETTING_IP_CONFIG_METHOD,
                     NM_SETTING_IP4_CONFIG_METHOD_MANUAL,
                     NULL);

        if (gateway) {
            g_object_set(s_ip4,
                         NM_SETTING_IP_CONFIG_GATEWAY,
                         "192.168.50.1",
                         NULL);
        }
        nm_setting_ip_config_add_address(s_ip4, ip_address);
        nm_ip_address_unref(ip_address);
    } else {
        g_print("IP setting method not found");
        g_main_loop_quit((GMainLoop *)loop);

        return;
    }

    nm_connection_add_setting(new_connection, NM_SETTING(s_ip4));

    if (!nm_connection_verify(new_connection, NULL)) {
        printf("Error: invalid property of connection, abort action.\n");
        g_main_loop_quit(loop);

        return;
    }

    nm_connection_replace_settings_from_connection(NM_CONNECTION(rem_con),
                                                   new_connection);
    nm_remote_connection_commit_changes_async(rem_con,
                                              !temporary,
                                              NULL,
                                              modify_connection_cb,
                                              wifi_modify_data);
    g_object_unref(new_connection);
}

int main(int argc, char *argv[])
{
    NMClient *client;
    GMainLoop *loop;
    GError *error = NULL;

    loop = g_main_loop_new(NULL, FALSE);

    // Connect to NetworkManager
    client = nm_client_new(NULL, &error);
    if (!client) {
        g_message("Error: Could not connect to NetworkManager: %s.", error->message);
        g_error_free(error);
        return 1;
    }

    update_ip4(client, loop, "manual", true);

    // Wait for the connection to be added
    g_main_loop_run(loop);

    // Clean up
    g_object_unref(client);

    return 0;
}
