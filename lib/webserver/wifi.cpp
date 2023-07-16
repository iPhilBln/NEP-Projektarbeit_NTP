#include "wifi.hpp"

const char html_header[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
    <title>NTP Client Settings</title>
    <style>
        .c,body{text-align:center;font-family:verdana}
        div,input,select{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}
        input,button,select,.msg{border-radius:.3rem;width: 100%}
        input[type=radio],input[type=checkbox]{width:auto}
        button,input[type='button'],input[type='submit']{cursor:pointer;border:0;background-color:#4CAF50;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}
        input[type='file']{border:1px solid #1fa3ec}
        .wrap {text-align:left;display:inline-block;min-width:260px;max-width:500px}
        a{color:#fff;font-weight:700;text-decoration:none}
        a:hover{color:#1fa3ec;text-decoration:underline}
        .q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}
        .q.q-0:after{background-position-x:0}
        .q.q-1:after{background-position-x:-16px}
        .q.q-2:after{background-position-x:-32px}
        .q.q-3:after{background-position-x:-48px}
        .q.q-4:after{background-position-x:-64px}
        .q.l:before{background-position-x:-80px;padding-right:5px}
        .ql .q{float:left}
        .q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position:16px 0;background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=')}
        @media (-webkit-min-device-pixel-ratio: 2),(min-resolution: 192dpi){.q:before,.q:after {background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');background-size:95px 16px}}
        .msg{padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777}
        .msg h4{margin-top:0;margin-bottom:5px}
        .msg.P{border-left-color:#1fa3ec}
        .msg.P h4{color:#1fa3ec}
        .msg.D{border-left-color:#dc3630}
        .msg.D h4{color:#dc3630}
        .msg.S{border-left-color:#5cb85c}
        .msg.S h4{color:#5cb85c}
        dt{font-weight:bold}
        dd{margin:0;padding:0 .5em .5em 0;min-height:12px}
        td{vertical-align:top}
        .h{display:none}
        button{transition:0s opacity;transition-delay:3s;transition-duration:0s;cursor:pointer}
        button.D{
            background-color:#4CAF50;
            border-radius: 10px;
        }
        button:active{opacity:50% !important;cursor:wait;transition-delay:0s}
        body{background-color:#000;color:#fff}
        input[type=checkbox]{
            display:inline-block;
            margin-right: 5px;
        }
        span.checkbox-label {
            color: #fff;
        }
    </style>
</head>
)";
const char html_client_settings_body_start[] PROGMEM = R"(
<body>
    <div class="wrap">
        <h1>NTP Client Settings</h1>
        <form action="/saventpsettings" method="post">
)";
const char html_client_settings_body_end[] PROGMEM = R"(
            <button class="D" type="submit">Save</button>
        </form>
    </div>
</body>
</html>
)";

const char html_tu_berlin_unchecked[] PROGMEM = R"(<label for="tu_berlin_ip"><input type="checkbox" id="tu_berlin_ip" name="tu_berlin_ip" value="130.149.7.7"> <span class="checkbox-label">TU Berlin</span></label><br>)";
const char html_tu_berlin_checked[] PROGMEM = R"(<label for="tu_berlin_ip"><input type="checkbox" id="tu_berlin_ip" name="tu_berlin_ip" value="130.149.7.7" checked> <span class="checkbox-label">TU Berlin</span></label><br>)";
const char html_tu_dresden_unchecked[] PROGMEM = R"(<label for="tu_dresden_ip"><input type="checkbox" id="tu_dresden_ip" name="tu_dresden_ip" value="141.76.32.160"> <span class="checkbox-label">TU Dresden</span></label><br>)";
const char html_tu_dresden_checked[] PROGMEM = R"(<label for="tu_dresden_ip"><input type="checkbox" id="tu_dresden_ip" name="tu_dresden_ip" value="141.76.32.160" checked> <span class="checkbox-label">TU Dresden</span></label><br>)";
const char html_t_online_unchecked[] PROGMEM = R"(<label for="t_online_ip"><input type="checkbox" id="t_online_ip" name="t_online_ip" value="194.25.134.196"> <span class="checkbox-label">T-Online</span></label><br>)";
const char html_t_online_checked[] PROGMEM = R"(<label for="t_online_ip"><input type="checkbox" id="t_online_ip" name="t_online_ip" value="194.25.134.196" checked> <span class="checkbox-label">T-Online</span></label><br>)";

const char html_custom[] PROGMEM = R"(
    <br>
    <label for="ntp_ip" style="color: white;">NTP Server -> IP Adresse:Port :</label>
    <input type="text" id="custom_ntp_name" name="custom_ntp_name" placeholder="NTP Server Name">
    <input type="text" id="custom_ntp_ip" name="custom_ntp_ip" maxlength="15" placeholder="000.000.000.000" pattern="^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
    <input type="text" id="custom_ntp_port" name="custom_ntp_port" maxlength="5" placeholder="123" pattern="^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
    <br>
)";

const char html_client_settings_param_saved[] PROGMEM = "<div class='msg S'>Saved<br/></div>";


WiFiManager wm;
WiFiManagerParameter title; // global param ( for non blocking w params )
WiFiManagerParameter lineBreak; // global param ( for non blocking w params )
WiFiManagerParameter tu_berlin_ntp_server; // global param ( for non blocking w params )
WiFiManagerParameter tu_dresden_ntp_server; // global param ( for non blocking w params )
WiFiManagerParameter t_online_ntp_server; // global param ( for non blocking w params )
WiFiManagerParameter custom_0_ntp_server; // global param ( for non blocking w params )

const char* checkbox_unchecked = "type=\"checkbox\"";
const char* checkbox_checked   = "type=\"checkbox\" checked";

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = true; // change to true to use non blocking

char* getParam(const char* name) {
    if (wm.server->hasArg(name)) {
        String value = wm.server->arg(name);
        if (value.length() > 0) {
            char* param = new char[value.length() + 1];
            strcpy(param, value.c_str());
            return param;
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

void saveParamCallback() {
    static char lost_ips[3][16] = { "", "", "" };
    uint8_t lost_cnt = 0;

    static bool preSave = true;
    preSave ? Serial.println("[CALLBACK] savePreParamCallback fired") : Serial.println("[CALLBACK] saveParamCallback fired");

    for (uint8_t i = 0; i < wm.getParametersCount(); i++) {
        const char* name = wm.getParameters()[i]->getID();
        if (name) {
            const char* param = getParam(name);
            if (param) {
                Serial.println(String(i) + " was checked");
                Serial.print("PARAM[" + String(name) + "]: ");
                Serial.println(param);

                // Hier NTP-Client-Objekte erstellen
            } else if (i != 5) {
                if (preSave) {
                    const char* value = wm.getParameters()[i]->getValue();
                    int length = wm.getParameters()[i]->getValueLength();
                    Serial.println(String(i) + " was nullptr");
                    Serial.println("val: " + String(value));
                    Serial.println("len: " + String(length));

                    strncpy(lost_ips[lost_cnt], value, sizeof(lost_ips[lost_cnt]) - 1);

                    lost_cnt < 3 ? lost_cnt++ : lost_cnt = lost_cnt;
                } else {
                    Serial.println(i);
                    const char* value = lost_ips[lost_cnt];
                    wm.getParameters()[i]->setValue(value, strlen(value));

                    lost_ips[lost_cnt][0] = '\0';

                    lost_cnt < 3 ? lost_cnt++ : lost_cnt = lost_cnt;
                }
            }
        } else {
            Serial.println(String(i) + " has no id");
        }
        Serial.println();
    }

    Serial.println("Array Inhalt:");
    for (uint8_t i = 0; i < 3; i++) {
        Serial.println(lost_ips[i]);
    }
    Serial.println();

    preSave = !preSave;
}


const char *logHTMLTags = R"(
    <html>
        <body>%s</body>
        <script>
            function updateLog() {
                location.reload();
                setTimeout(updateLog, 1000);
            }
           updateLog();
        </script>
    </html>
)";

bool tu_berlin = false;
bool tu_dresden = false;
bool t_online = false;

void handleNtpSettingsRoute(void){
    Serial.println("[HTTP] handle NTP settings route");

    wm.server->sendHeader("Content-Type", "text/html");
    wm.server->sendContent_P(html_header);
    wm.server->sendContent_P(html_client_settings_body_start);

    tu_berlin ? wm.server->sendContent_P(html_tu_berlin_unchecked) : wm.server->sendContent_P(html_tu_berlin_checked);
    tu_dresden ? wm.server->sendContent_P(html_tu_dresden_unchecked) : wm.server->sendContent_P(html_tu_dresden_checked);
    t_online ? wm.server->sendContent_P(html_t_online_unchecked) : wm.server->sendContent_P(html_t_online_checked);
    
    wm.server->sendContent_P(html_client_settings_body_end);

    wm.server->sendContent_P(html_client_settings_body_end);
}

void handleNtpClientsRoute(void){
    Serial.println("[HTTP] handle NTP clients route");
    wm.server->send(200, "text/html", logHTMLTags);
}

void handleSaveNtpSettings(void) {
    tu_berlin = false;
    tu_dresden = false;
    tu_berlin = false;


    Serial.println("[HTTP] handle NTP clients save settings: start");
    wm.server->sendHeader("Content-Type", "text/html");
    wm.server->sendContent_P(html_header);
    wm.server->sendContent_P(html_client_settings_param_saved);

    if (wm.server->hasArg("tu_berlin_ip")) {
        Serial.println("\tTU Berlin was checked");
        tu_berlin = true;
        NTPClient("TU Berlin", wm.server->arg("tu_berlin_ip").c_str(), TYPE::MASTER);
    } 
    if (wm.server->hasArg("tu_dresden_ip")) {
        Serial.println("\tTU Dresden was checked");
        tu_dresden = true;
        NTPClient("TU Dresden", wm.server->arg("tu_dresden_ip").c_str(), TYPE::MASTER);
    } 
    if (wm.server->hasArg("t_online_ip")) {
        Serial.println("\tT-Online was checked");
        t_online = true;
        NTPClient("T-Online", wm.server->arg("t_online_ip").c_str(), TYPE::MASTER);
    } 
    if (wm.server->hasArg("custom_ntp_ip")) {
        if (wm.server->arg("custom_ntp_ip").length() > 0) {
            Serial.println("\tCustom was checked");
            NTPClient(
                wm.server->arg("custom_ntp_name").c_str(), 
                wm.server->arg("custom_ntp_ip").c_str(), 
                TYPE::SLAVE, 
                wm.server->arg("custom_ntp_port").length() > 0 ? static_cast<uint16_t>(strtoul(wm.server->arg("custom_ntp_port").c_str(), nullptr, 10)) : 123);
        }
    }


    Serial.println("[HTTP] handle NTP clients save settings: end");
    Serial.println();
}

void bindServerCallback(){
    wm.server->on("/ntpsettings", handleNtpSettingsRoute);
    wm.server->on("/ntpclients", HTTP_GET, handleNtpClientsRoute);
    wm.server->on("/saventpsettings", handleSaveNtpSettings);
}

void wifiBegin(void) {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.println("\n Starting Wifi AP...");

    const char* title_str = "<br/><label style='font-weight: bold; font-size: 25px;'>NTP Server Auswahl</label><br/>";
    const char* lineBreak_str = "<br/><br />";
    const char* tu_berlin_ip    = "130.149.7.7";
    const char* tu_dresden_ip   = "141.76.32.160";
    const char* t_online_ip     = "194.25.134.196";

    new (&title) WiFiManagerParameter (title_str);
    new (&tu_berlin_ntp_server) WiFiManagerParameter("tu_berlin_ntp_server", "TU Berlin", tu_berlin_ip, strlen(tu_berlin_ip), checkbox_unchecked, WFM_LABEL_AFTER);      // custom html input for tu berlin ntp server
    new (&tu_dresden_ntp_server) WiFiManagerParameter("tu_dresden_ntp_server", "TU Dresden", tu_dresden_ip, strlen(tu_dresden_ip), checkbox_unchecked, WFM_LABEL_AFTER);    // custom html input for tu dresden ntp server
    new (&t_online_ntp_server) WiFiManagerParameter("t_online_ntp_server", "T-Online", t_online_ip, strlen(t_online_ip),checkbox_unchecked, WFM_LABEL_AFTER);        // custom html input for t-online ntp server
    new (&lineBreak) WiFiManagerParameter (lineBreak_str);
    new (&custom_0_ntp_server) WiFiManagerParameter("custom_0_ntp_server", "NTP Server IP:", "", 15, "pattern='\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}'");
    
    wm.addParameter(&title);
    wm.addParameter(&tu_berlin_ntp_server);    
    wm.addParameter(&tu_dresden_ntp_server);
    wm.addParameter(&t_online_ntp_server);
    wm.addParameter(&lineBreak);
    wm.addParameter(&custom_0_ntp_server);

    //wm.setPreSaveParamsCallback(saveParamCallback);
    //wm.setSaveParamsCallback(saveParamCallback);

    wm.setWebServerCallback(bindServerCallback);
    // set custom html menu content , inside menu item "custom", see setMenu()
    const char* menuhtml = "\
        <form action='/ntpsettings' method='get'><button>NTP Settings</button></form><br/>\n\
        <form action='/ntpclients' method='get'><button>NTP Clients</button></form><br/>\n\
    ";
    wm.setCustomMenuHTML(menuhtml);

    // custom menu via array or vector
    //std::vector<const char *> menu = {"wifi","info","param","sep","restart","erase","exit"};
    std::vector<const char *> menu = {"wifi","info","param","sep","custom", "sep","restart","erase","exit"};
    wm.setMenu(menu);

    wm.setTitle("NTP Client - Settings");
    wm.setHostname("NTPclient");

    // set country to germany
    //wm.setCountry("DE"); -> doesn't work for ESP32

    // set dark theme
    wm.setClass("invert");

    wm.setConfigPortalBlocking(true);
    wm.setConfigPortalTimeout(6000);                                 // auto close configportal after n seconds
    
    const char* apName = "NTPclient";
    const char* apPass = "clientNtpSettings";
    bool res = wm.autoConnect(apName,apPass);      // start password protected accesspoint

  if(!res) {
    Serial.println("Failed to connect or hit timeout!");
    delay(500);
    ESP.restart();
  } 
  else {
    Serial.println("Connected to Wifi.");
  }
    wm.setWebPortalClientCheck(true);
    wm.startWebPortal();
    Serial.println("Webportal is running...");
}

void wifiProcess(void) {
    //if (wm_nonblocking) wm.process();
    wm.process();
}

void wifiCleanup(bool eraseSPIFF) {
    Serial.println("Erasing Config and restarting...");
    delay(1000);
    if ( eraseSPIFF) {
        //clean FS, for testing
        SPIFFS.format();
    }

    wm.resetSettings();
    ESP.restart();
}

int wifiGetParamCounter(void) {
    return wm.getParametersCount();
}

const char* wifiGetParamValue(uint8_t paramNum) {
    return wm.getParametersCount() > 0 ? wm.getParameters()[paramNum]->getValue() : nullptr;
}