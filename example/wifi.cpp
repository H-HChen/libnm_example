#include <glib.h>
#include <NetworkManager.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

NMConnection *  get_client_nmconnection(const char* connection_id, const char* uuid, GString* ssid, const char * password){
    NMConnection *connection = NULL;
    NMSettingConnection *s_con;
    NMSettingWireless *     s_wireless;
    NMSettingIP4Config * s_ip4;
    NMSettingWirelessSecurity * s_secure;

    connection = nm_simple_connection_new();
    s_wireless = (NMSettingWireless *) nm_setting_wireless_new();
    s_secure = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
    s_con = (NMSettingConnection *) nm_setting_connection_new();

    g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_UUID,
                 uuid,
                 NM_SETTING_CONNECTION_ID,
                 connection_id,
                 NM_SETTING_CONNECTION_TYPE,
                 "802-11-wireless",
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_con));

    s_wireless = (NMSettingWireless *) nm_setting_wireless_new();

    g_object_set(G_OBJECT(s_wireless),
                 NM_SETTING_WIRELESS_SSID,
                 ssid,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_wireless));
    
    s_secure = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
    g_object_set(G_OBJECT(s_secure),
                 NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                 "wpa-psk",
                 NM_SETTING_WIRELESS_SECURITY_PSK,
                 password,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_secure));
    
    s_ip4 = (NMSettingIP4Config *) nm_setting_ip4_config_new();
    g_object_set(G_OBJECT(s_ip4),
                 NM_SETTING_IP_CONFIG_METHOD,
                 NM_SETTING_IP4_CONFIG_METHOD_AUTO,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip4));

    return connection;
}

static void
added_cb(GObject *client, GAsyncResult *result, gpointer user_data)
{
    
    NMRemoteConnection *remote;
    GError *            error = NULL;

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
    g_main_loop_quit((GMainLoop*)user_data);
}

static void
add_connection(NMClient *client, GMainLoop *loop, const char *con_name)
{
    NMConnection *       connection;
    NMSettingConnection *s_con;
    NMSettingWired *     s_wired;
    NMSettingIP4Config * s_ip4;
    char *               uuid;

    /* Create a new connection object */
    connection = nm_simple_connection_new();

    /* Build up the 'connection' Setting */
    s_con = (NMSettingConnection *) nm_setting_connection_new();
    uuid  = nm_utils_uuid_generate();
    g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_UUID,
                 uuid,
                 NM_SETTING_CONNECTION_ID,
                 con_name,
                 NM_SETTING_CONNECTION_TYPE,
                 "802-3-ethernet",
                 NULL);
    g_free(uuid);
    nm_connection_add_setting(connection, NM_SETTING(s_con));

    /* Build up the 'wired' Setting */
    s_wired = (NMSettingWired *) nm_setting_wired_new();
    nm_connection_add_setting(connection, NM_SETTING(s_wired));

    /* Build up the 'ipv4' Setting */
    s_ip4 = (NMSettingIP4Config *) nm_setting_ip4_config_new();
    g_object_set(G_OBJECT(s_ip4),
                 NM_SETTING_IP_CONFIG_METHOD,
                 NM_SETTING_IP4_CONFIG_METHOD_AUTO,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip4));

    /* Ask the settings service to add the new connection; we'll quit the
     * mainloop and exit when the callback is called.
     */
    nm_client_add_connection_async(client, connection, TRUE, NULL, added_cb, loop);
    g_object_unref(connection);
}

static void
add_wifi(NMClient *client, GMainLoop *loop, const char *con_name)
{
    NMConnection *       connection;

    const char *   uuid;
    const char *   password;

    /* Create a new connection object */
    uuid  = nm_utils_uuid_generate();
    GString* ssid = g_string_new("RMTserver");
    password = "adlinkros";
    connection = get_client_nmconnection("RMTClient", uuid, ssid, password);

    /* Ask the settings service to add the new connection; we'll quit the
     * mainloop and exit when the callback is called.
     */
    nm_client_add_connection_async(client, connection, TRUE, NULL, added_cb, loop);
    g_object_unref(connection);
}

static void
modify_connection_cb (GObject *connection,
                      GAsyncResult *result,
                      gpointer user_data)
{
	GError *error = NULL;
	if (!nm_remote_connection_commit_changes_finish (NM_REMOTE_CONNECTION (connection),
	                                                 result, &error)) {
		printf(("Error: Failed to modify connection '%s': %s"),
		                 nm_connection_get_id (NM_CONNECTION (connection)),
		                 error->message);
	} 
    else {
			printf(("Connection '%s' (%s) successfully modified.\n"),
			         nm_connection_get_id (NM_CONNECTION (connection)),
			         nm_connection_get_uuid (NM_CONNECTION (connection)));
	}
    g_main_loop_quit((GMainLoop*)user_data);

}

static void
modify_wifi2(NMClient *client, GMainLoop *loop)
{
    NMRemoteConnection *rem_con = NULL;
    NMConnection *connection = NULL;
    gboolean temporary = FALSE;
    NMSettingWirelessSecurity * s_secure;
    s_secure = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
    const char *   uuid;
    const char *   password;
    const char *   ssid_char;
    ssid_char = "RMTtest";
    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    nm_connection_remove_setting(NM_CONNECTION (rem_con), NM_TYPE_SETTING_WIRELESS_SECURITY);
    s_secure = (NMSettingWirelessSecurity *) nm_setting_wireless_security_new();
    g_object_set(G_OBJECT(s_secure),
                 NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                 "wpa-psk",
                 NM_SETTING_WIRELESS_SECURITY_PSK,
                 "adlinktest",
                 NULL);
    nm_connection_add_setting(NM_CONNECTION (rem_con), NM_SETTING(s_secure));
    GString* ssid = g_string_new(ssid_char);
    uuid  = nm_setting_connection_get_uuid(nm_connection_get_setting_connection(NM_CONNECTION (rem_con)));
    password = "adlinktest";
    nm_remote_connection_commit_changes_async(rem_con,
	                                           !temporary,
	                                           NULL,
	                                           modify_connection_cb,
	                                           loop);    
}

static void
modify_wifi(NMClient *client, GMainLoop *loop)
{
    NMRemoteConnection *rem_con = NULL;
    NMConnection *connection = NULL;
    gboolean temporary = FALSE;
    const char *   uuid;
    const char *   password;

    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    connection = nm_simple_connection_new();
    GString* ssid = g_string_new("RMTtest");
    uuid  = nm_setting_connection_get_uuid(nm_connection_get_setting_connection(NM_CONNECTION (rem_con)));
    password = "adlinktest";

    connection = get_client_nmconnection("RMTClient", uuid, ssid, password);

    nm_connection_replace_settings_from_connection (NM_CONNECTION (rem_con),
					                                               connection);
    nm_remote_connection_commit_changes_async(rem_con,
	                                           !temporary,
	                                           NULL,
	                                           modify_connection_cb,
	                                           loop);
    g_object_unref(connection);
}

static void
modify_current_wifi(NMClient *client, GMainLoop *loop)
{
    NMRemoteConnection *rem_con = NULL;
    NMConnection *connection = NULL;
    gboolean temporary = FALSE;
    const char *   uuid;
    const char *   password;
    NMDevice *device;
    NMActiveConnection *active_con;
    const char *   name;

    device = nm_client_get_device_by_iface(client, "wlx08beac0c85d9");
    active_con = nm_device_get_active_connection(device);
    rem_con = nm_active_connection_get_connection(active_con);
    name = nm_connection_get_id(NM_CONNECTION (rem_con));
    GString* ssid = g_string_new("RMTtest");
    uuid  = nm_setting_connection_get_uuid(nm_connection_get_setting_connection(NM_CONNECTION (rem_con)));
    password = "adlinktest";

    connection = get_client_nmconnection(name, uuid, ssid, password);

    nm_connection_replace_settings_from_connection (NM_CONNECTION (rem_con),
					                                               connection);
    nm_remote_connection_commit_changes_async(rem_con,
	                                           !temporary,
	                                           NULL,
	                                           modify_connection_cb,
	                                           loop);
    g_object_unref(connection);
}

typedef struct {
	GMainLoop *loop;
	NMConnection *local;
	const char *setting_name;
} GetSecretsData;

static void
got_secrets (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
	NMRemoteConnection *remote = NM_REMOTE_CONNECTION (source_object);
	GetSecretsData *data = (GetSecretsData*)user_data;
	GVariant *secrets;
	GError *error = NULL;

	secrets = nm_remote_connection_get_secrets_finish (remote, res, NULL);
	if (secrets) {
		if (!nm_connection_update_secrets (data->local, NULL, secrets, &error) && error) {
			g_print("Error updating secrets for %s: %s\n",
			            data->setting_name, error->message);
			g_clear_error (&error);
		}
		g_variant_unref (secrets);
	}

	g_main_loop_quit (data->loop);
}

static void
get_password(NMClient *client)
{
    NMRemoteConnection *rem_con = NULL;
    NMConnection *new_connection;
    NMSettingWirelessSecurity * s_secure;
    const char *   password;
    const char *   name;
    GetSecretsData data = { 0, };

    rem_con = nm_client_get_connection_by_id(client, "RMTClient");
    new_connection = nm_simple_connection_new_clone(NM_CONNECTION(rem_con));
    data.loop = g_main_loop_new (NULL, FALSE);
    data.local = new_connection;
    data.setting_name = "802-11-wireless-security";

    nm_remote_connection_get_secrets_async (rem_con,
		                                    "802-11-wireless-security",
		                                    NULL,
		                                    got_secrets,
		                                    &data);
	g_main_loop_run (data.loop);

	g_main_loop_unref(data.loop);

    s_secure = (NMSettingWirelessSecurity *) nm_connection_get_setting_wireless_security(new_connection);
    password = nm_setting_wireless_security_get_psk(s_secure);
    g_print("%s",password);
}

static void
get_active(NMClient *client)
{
    NMDevice *device;
    NMActiveConnection *active_con;
    NMSettingWireless  *s_wireless;
    NMConnection *new_connection;
    GBytes *         active_ssid;
    const char *   name;
    const char *   ssid;

    device = nm_client_get_device_by_iface(client, "wlx08beac0c85d9");
    active_con = nm_device_get_active_connection(device);
    new_connection = nm_simple_connection_new_clone(NM_CONNECTION(nm_active_connection_get_connection(active_con)));
    name = nm_connection_get_id(new_connection);
    s_wireless = nm_connection_get_setting_wireless(new_connection);
    active_ssid = nm_setting_wireless_get_ssid(s_wireless);
    ssid = nm_utils_ssid_to_utf8((guint8* )g_bytes_get_data(active_ssid, NULL),
                                 g_bytes_get_size(active_ssid));
    g_print("%s\n",name);
    g_print("%s",ssid);
}

int
main(int argc, char *argv[])
{
    
    NMClient * client;
    GMainLoop *loop;
    GError *   error = NULL;
    
    loop = g_main_loop_new(NULL, FALSE);

    // Connect to NetworkManager 
    client = nm_client_new(NULL, &error);
    if (!client) {
        g_message("Error: Could not connect to NetworkManager: %s.", error->message);
        g_error_free(error);
        return 1;
    }

    // Ask NM to add the new connection 

    add_wifi(client, loop, "RMTClient");
    //modify_wifi3(client, loop);
    //get_active(client);
    //get_password(client);
    // Wait for the connection to be added 
    g_main_loop_run(loop);

    // Clean up 

    g_object_unref(client);
    


    return 0;
}