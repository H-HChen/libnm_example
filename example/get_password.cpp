#include "get_connection.hpp"

typedef struct {
    GMainLoop *loop;
    NMConnection *local;
    const char *setting_name;
} GetSecretsData;

void got_secrets(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    NMRemoteConnection *remote = NM_REMOTE_CONNECTION(source_object);
    GetSecretsData *data = (GetSecretsData *)user_data;
    GVariant *secrets;
    GError *error = NULL;

    secrets = nm_remote_connection_get_secrets_finish(remote, res, NULL);
    if (secrets) {
        if (!nm_connection_update_secrets(data->local, NULL, secrets, &error) && error) {
            g_print("Error updating secrets for %s: %s\n",
                    data->setting_name, error->message);
            g_clear_error(&error);
        }
        g_variant_unref(secrets);
    }

    g_main_loop_quit(data->loop);
}

void get_password(NMClient *client)
{
    NMRemoteConnection *rem_con = NULL;
    NMConnection *new_connection;
    NMSettingWirelessSecurity *s_secure;
    const char *password;
    const char *name;
    GetSecretsData data = {
        0,
    };

    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    new_connection = nm_simple_connection_new_clone(NM_CONNECTION(rem_con));
    data.loop = g_main_loop_new(NULL, FALSE);
    data.local = new_connection;
    data.setting_name = "802-11-wireless-security";

    nm_remote_connection_get_secrets_async(rem_con,
                                           "802-11-wireless-security",
                                           NULL,
                                           got_secrets,
                                           &data);
    g_main_loop_run(data.loop);

    g_main_loop_unref(data.loop);

    s_secure = nm_connection_get_setting_wireless_security(new_connection);
    password = nm_setting_wireless_security_get_psk(s_secure);
    name = nm_connection_get_id(new_connection);
    g_print("password of connection [%s] is: %s\n", name, password);
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

    get_password(client);

    // Clean up
    g_object_unref(client);

    return 0;
}
