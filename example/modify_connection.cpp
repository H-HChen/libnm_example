#include "get_connection.hpp"

void modify_connection_cb(GObject *     connection,
                          GAsyncResult *result,
                          gpointer      user_data)
{
    GError *error = NULL;

    if (!nm_remote_connection_commit_changes_finish(NM_REMOTE_CONNECTION(connection),
                                                    result, &error)) {
        printf(("Error: Failed to modify connection '%s': %s"),
               nm_connection_get_id(NM_CONNECTION(connection)),
               error->message);
    } else {
        printf(("Connection '%s' (%s) successfully modified.\n"),
               nm_connection_get_id(NM_CONNECTION(connection)),
               nm_connection_get_uuid(NM_CONNECTION(connection)));
    }
    g_main_loop_quit((GMainLoop *)user_data);
}

void modify_wifi(NMClient *client, GMainLoop *loop)
{
    NMRemoteConnection *rem_con = NULL;
    NMConnection *connection = NULL;
    gboolean temporary = FALSE;
    const char *uuid;
    const char *password;

    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    GString *ssid = g_string_new("RMTtest");
    uuid = nm_setting_connection_get_uuid(nm_connection_get_setting_connection(NM_CONNECTION(rem_con)));
    password = "adlinktest";

    connection = get_client_nmconnection("RMTClient", uuid, ssid, password);

    nm_connection_replace_settings_from_connection(NM_CONNECTION(rem_con),
                                                   connection);
    nm_remote_connection_commit_changes_async(rem_con,
                                              !temporary,
                                              NULL,
                                              modify_connection_cb,
                                              loop);
    g_object_unref(connection);
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

    modify_wifi(client, loop);

    // Wait for the connection to be added
    g_main_loop_run(loop);

    // Clean up
    g_object_unref(client);

    return 0;
}
