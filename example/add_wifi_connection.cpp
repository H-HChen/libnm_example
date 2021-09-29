#include "get_connection.hpp"

void added_cb(GObject *client, GAsyncResult *result, gpointer user_data)
{
    NMRemoteConnection *remote;
    GError *error = NULL;

    /* NM responded to our request; either handle the resulting error or
     * print out the object path of the connection we just added.
     */
    remote = nm_client_add_connection_finish(NM_CLIENT(client), result, &error);

    if (error) {
        g_print("Error adding connection: %s", error->message);
        g_error_free(error);
    } else {
        g_print("Added: %s\n", nm_connection_get_path(NM_CONNECTION(remote)));
        g_object_unref(remote);
    }

    /* Tell the mainloop we're done and we can quit now */
    g_main_loop_quit((GMainLoop *)user_data);
}

void add_wifi(NMClient *client, GMainLoop *loop)
{
    NMConnection *connection;

    const char *uuid;
    const char *password;

    /* Create a new connection object */
    uuid = nm_utils_uuid_generate();
    GString *ssid = g_string_new("RMTserver");
    password = "adlinkros";
    connection = get_client_nmconnection("RMTClient", uuid, ssid, password);

    /* Ask the settings service to add the new connection; we'll quit the
     * mainloop and exit when the callback is called.
     */
    nm_client_add_connection_async(client, connection, TRUE, NULL, added_cb, loop);
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

    add_wifi(client, loop);

    // Wait for the connection to be added
    g_main_loop_run(loop);

    // Clean up
    g_object_unref(client);

    return 0;
}
