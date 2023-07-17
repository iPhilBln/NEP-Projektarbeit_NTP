#include "wifi.hpp"
#include "html.h"

WiFiManager wm;

bool tu_berlin = false;
bool cloudflare = false;
bool t_online = false;

void handleNtpSettingsRoute(void){
    Serial.println("[HTTP] handle NTP settings route");

    wm.server->sendHeader("Content-Type", "text/html");
    wm.server->sendContent_P(html_header);
    wm.server->sendContent_P(html_client_settings_body_start);

    tu_berlin ? wm.server->sendContent_P(html_tu_berlin_checked) : wm.server->sendContent_P(html_tu_berlin_unchecked);
    cloudflare ? wm.server->sendContent_P(html_cloudflare_checked) : wm.server->sendContent_P(html_cloudflare_unchecked);
    t_online ? wm.server->sendContent_P(html_t_online_checked) : wm.server->sendContent_P(html_t_online_unchecked);
    
    wm.server->sendContent_P(html_custom);
    wm.server->sendContent_P(html_client_settings_body_end);
}


void handleSaveNtpSettings(void) {
    Serial.println("[HTTP] handle NTP clients save settings: start");
    wm.server->sendHeader("Content-Type", "text/html");
    wm.server->sendContent_P(html_header);
    wm.server->sendContent_P(html_client_settings_param_saved);

    if (wm.server->hasArg("tu_berlin_ip") && (! tu_berlin)) {
        NTPClient* ntpClient = new NTPClient("TU Berlin", wm.server->arg("tu_berlin_ip").c_str(), TYPE::SLAVE);
        if (ntpClient->getIsClient()) {
            ntpClient->addMember();
            tu_berlin = true;
        }
    }
    else if (! wm.server->hasArg("tu_berlin_ip") && tu_berlin) {
        NTPClient::removeSlave("TU Berlin");
        tu_berlin = false;
    }  
    if (wm.server->hasArg("cloudflare_ip") && (! cloudflare)) {
        Serial.println("[HTTP] Cloudflare was checked");
        NTPClient* ntpClient = new NTPClient("Cloudflare", wm.server->arg("cloudflare_ip").c_str(), TYPE::SLAVE);
        if (ntpClient->getIsClient()) {
            ntpClient->addMember();
            cloudflare = true;
        }
    }
    else if (! wm.server->hasArg("cloudflare_ip") && cloudflare) {
        NTPClient::removeSlave("Cloudflare");
        cloudflare = false;
    } 
    if (wm.server->hasArg("t_online_ip") && (! t_online)) {
        Serial.println("[HTTP] T-Online was checked");
        NTPClient* ntpClient = new NTPClient("T-Online", wm.server->arg("t_online_ip").c_str(), TYPE::SLAVE);
        if (ntpClient->getIsClient()) {
            ntpClient->addMember();
            t_online = true;
        }
    }
    else if (! wm.server->hasArg("t_online_ip") && t_online) {
        NTPClient::removeSlave("T-Online");
        t_online = false;
    } 
    if (wm.server->hasArg("custom_ntp_ip")) {
        if (wm.server->arg("custom_ntp_ip").length() > 0) {
            Serial.println("[HTTP] Custom was checked");
            String     name = wm.server->arg("custom_ntp_name");
            String     ip   = wm.server->arg("custom_ntp_ip");
            String port_str = wm.server->arg("custom_ntp_port");
            uint16_t   port = port_str.length() > 0 ? static_cast<uint16_t>(port_str.toDouble()) : 123;
            NTPClient* ntpClient = new NTPClient(name, ip, TYPE::MASTER, port);
            if (ntpClient->getIsClient()) {
                NTPClient::removeMaster();
                ntpClient->addMember();
            }
        }
    }

    Serial.println("[HTTP] handle NTP clients save settings: end");
    Serial.println();
}

void handleNtpClientsRoute(void) {
    Serial.println("[HTTP] handle NTP clients route");

    String html = String(html_header) + String(html_clients);
    wm.server->send(200, "text/html", html);
}

void handleGetNtpJson() {
    String json = NTPClient::toJSON();
    wm.server->send(200, "application/json", json);
}


void bindServerCallback(){
    wm.server->on("/ntpsettings", handleNtpSettingsRoute);
    wm.server->on("/saventpsettings", handleSaveNtpSettings);
    wm.server->on("/ntpclients", HTTP_GET, handleNtpClientsRoute);
    wm.server->on("/getntpjson", HTTP_GET, handleGetNtpJson);
}

void wifiBegin(void) {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.println("\n Starting Wifi AP...");

    wm.setWebServerCallback(bindServerCallback);

    // set custom html menu content , inside menu item "custom", see setMenu()
    const char* menuhtml = "\
        <form action='/ntpsettings' method='get'><button>NTP Settings</button></form><br/>\n\
        <form action='/ntpclients' method='get'><button>NTP Clients</button></form><br/>\n\
    ";
    wm.setCustomMenuHTML(menuhtml);

    // custom menu via array or vector
    std::vector<const char *> menu = {"wifi","info","sep","custom", "sep","restart","erase","exit"};
    wm.setMenu(menu);

    wm.setTitle("NTP Client - Settings");
    wm.setHostname("NTPclient");

    // set dark theme
    wm.setClass("invert");

    wm.setConfigPortalBlocking(true);
    wm.setConfigPortalTimeout(6000);               // auto close configportal after n seconds
    
    const char* apName = "NTPclient";
    const char* apPass = "clientNtpSettings";
    bool res = wm.autoConnect(apName,apPass);      // start password protected accesspoint

  if(!res) {
    Serial.println("[VERBINDUNG] Es konnte keine Verbindung vergestellt werden!");
    delay(500);
    ESP.restart();
  } 
  else {
    Serial.println("[VERBINDUNG] Mit dem Netzwerk verbungen.");
  }
    wm.setWebPortalClientCheck(true);
    wm.startWebPortal();
    Serial.println("Webportal läuft...");
}

void wifiProcess(void) {
    wm.process();
}

void wifiCleanup(bool eraseSPIFF) {
    Serial.println("Lösche Konfiguration und starte neu...");
    delay(1000);
    if ( eraseSPIFF) {
        //clean FS, for testing
        SPIFFS.format();
    }

    wm.resetSettings();
    ESP.restart();
}