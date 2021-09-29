#include <glib.h>
#include <NetworkManager.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

NMConnection *get_client_nmconnection(const char *connection_id, const char *uuid, GString *ssid, const char *password)
{
    NMConnection *connection = NULL;
    NMSettingConnection *s_con;
    NMSettingWireless *s_wireless;
    NMSettingIP4Config *s_ip4;
    NMSettingWirelessSecurity *s_secure;

    connection = nm_simple_connection_new();
    s_wireless = (NMSettingWireless *)nm_setting_wireless_new();
    s_secure = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
    s_con = (NMSettingConnection *)nm_setting_connection_new();

    g_object_set(G_OBJECT(s_con),
                 NM_SETTING_CONNECTION_UUID,
                 uuid,
                 NM_SETTING_CONNECTION_ID,
                 connection_id,
                 NM_SETTING_CONNECTION_TYPE,
                 "802-11-wireless",
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_con));

    s_wireless = (NMSettingWireless *)nm_setting_wireless_new();

    g_object_set(G_OBJECT(s_wireless),
                 NM_SETTING_WIRELESS_SSID,
                 ssid,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_wireless));

    s_secure = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
    g_object_set(G_OBJECT(s_secure),
                 NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,
                 "wpa-psk",
                 NM_SETTING_WIRELESS_SECURITY_PSK,
                 password,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_secure));

    s_ip4 = (NMSettingIP4Config *)nm_setting_ip4_config_new();
    g_object_set(G_OBJECT(s_ip4),
                 NM_SETTING_IP_CONFIG_METHOD,
                 NM_SETTING_IP4_CONFIG_METHOD_AUTO,
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip4));

    return connection;
}
