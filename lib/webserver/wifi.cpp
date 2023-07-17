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
const char html_cloudflare_unchecked[] PROGMEM = R"(<label for="cloudflare_ip"><input type="checkbox" id="cloudflare_ip" name="cloudflare_ip" value="162.159.200.123"> <span class="checkbox-label">Cloudflare</span></label><br>)";
const char html_cloudflare_checked[] PROGMEM = R"(<label for="cloudflare_ip"><input type="checkbox" id="cloudflare_ip" name="cloudflare_ip" value="162.159.200.123" checked> <span class="checkbox-label">Cloudflare</span></label><br>)";
const char html_t_online_unchecked[] PROGMEM = R"(<label for="t_online_ip"><input type="checkbox" id="t_online_ip" name="t_online_ip" value="194.25.134.196"> <span class="checkbox-label">T-Online</span></label><br>)";
const char html_t_online_checked[] PROGMEM = R"(<label for="t_online_ip"><input type="checkbox" id="t_online_ip" name="t_online_ip" value="194.25.134.196" checked> <span class="checkbox-label">T-Online</span></label><br>)";

const char html_custom[] PROGMEM = R"(
    <br>
    <label for="custom_ntp" style="color: white;">NTP Server -> IP Adresse:Port :</label>
    <input type="text" id="custom_ntp_name" name="custom_ntp_name" placeholder="NTP Server Name">
    <input type="text" id="custom_ntp_ip" name="custom_ntp_ip" maxlength="15" placeholder="000.000.000.000" pattern="^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$">
    <input type="text" id="custom_ntp_port" name="custom_ntp_port" maxlength="5" placeholder="123" pattern="[0-9]+" value="123" required>
    <br>
)";

//const char html_client_settings_param_saved[] PROGMEM = "<div class='msg S'>Saved<br/></div>";
const char html_client_settings_param_saved[] PROGMEM = "<div class='msg S' style='background-color: #5cb85c;'>Saved<br/></div>";


const char html_clients[] PROGMEM = R"(
    <body>
        <h2>Master</h2>
        <table id="master-table">
            <colgroup>
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
            </colgroup>
            <tr>
                <th>Server</th>
                <th>Zeitstempel</th>
                <th>RTT</th>
                <th></th>
            </tr>
            <tr id="master-data-row">
                <td id="master-name-cell"></td>
                <td id="master-time-cell"></td>
                <td id="master-rtt-cell"></td>
                <td id="master-cell-4"></td>
            </tr>
        </table>

        <h2>Slave</h2>
        <table id="slave-table">
            <colgroup>
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
            </colgroup>
            <tr>
                <th>Server</th>
                <th>Zeitstempel</th>
                <th>RTT</th>
                <th>Zeitdifferenz</th>
            </tr>
        </table>

        <style>
            table {
                width: 100%;
                border-collapse: collapse;
            }
            th, td {
                padding: 8px;
                text-align: center;
                word-wrap: break-word;
            }
            th {
                background-color: #000000;
                color: #ffffff;  // Textfarbe auf Weiß ändern
            }
        </style>

        <script>
            function updateTable() {
                fetch('/getntpjson')
                    .then(response => response.json())
                    .then(data => {
                        let masterData = data[0].master[0];
                        let slaveData = data[1].slave;

                        // Aktualisieren Sie die Tabelle für den Master
                        let masterNameCell = document.getElementById('master-name-cell');
                        let masterTimeCell = document.getElementById('master-time-cell');
                        let masterRttCell = document.getElementById('master-rtt-cell');
                        let masterCell4 = document.getElementById('master-cell-4');
                        masterNameCell.innerHTML = masterData.name;
                        masterTimeCell.innerHTML = masterData.time;
                        masterRttCell.innerHTML = masterData.t_rtt;
                        masterCell4.innerHTML = ''; 

                        // Aktualisieren Sie die Tabelle für die Slaves
                        let slaveTable = document.getElementById('slave-table');
                        slaveTable.innerHTML = '';  // Tabelle leeren

                        slaveData.forEach((slave, index) => {
                            let slaveRow = slaveTable.insertRow(index + 1);
                            let slaveNameCell = slaveRow.insertCell();
                            let slaveTimeCell = slaveRow.insertCell();
                            let slaveRttCell = slaveRow.insertCell();
                            let slaveDiffCell = slaveRow.insertCell();
                            slaveNameCell.innerHTML = slave.name;
                            slaveTimeCell.innerHTML = slave.time;
                            slaveRttCell.innerHTML = slave.t_rtt;
                            slaveDiffCell.innerHTML = slave.t_dif;
                        });

                    })
                    .catch(error => console.error('Error:', error));
            }

            setInterval(updateTable, 1000);  // Aktualisierung alle 1 Sekunde
        </script>

    </body>
</html>
)";


const char html_clients_neu[] PROGMEM = R"(
    <body>
        <h2>Master</h2>
        <table id="master-table">
            <colgroup>
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
            </colgroup>
            <tr>
                <th>Server</th>
                <th>Zeitstempel</th>
                <th>RTT</th>
                <th></th>
            </tr>
            <tr id="master-data-row">
                <td id="master-name-cell"></td>
                <td id="master-time-cell"></td>
                <td id="master-rtt-cell"></td>
                <td id="master-cell-4"></td>
            </tr>
        </table>

        <h2>Slave</h2>
        <table id="slave-table">
            <colgroup>
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
                <col style="width: 25%;">
            </colgroup>
            <tr>
                <th>Server</th>
                <th>Zeitstempel</th>
                <th>RTT</th>
                <th>Zeitdifferenz</th>
            </tr>
            <tr id="slave-data-row-1">
                <td id="slave-name-cell-1"></td>
                <td id="slave-time-cell-1"></td>
                <td id="slave-rtt-cell-1"></td>
                <td id="slave-diff-cell-1"></td>
            </tr>
            <tr id="slave-data-row-2">
                <td id="slave-name-cell-2"></td>
                <td id="slave-time-cell-2"></td>
                <td id="slave-rtt-cell-2"></td>
                <td id="slave-diff-cell-2"></td>
            </tr>
            <tr id="slave-data-row-3">
                <td id="slave-name-cell-3"></td>
                <td id="slave-time-cell-3"></td>
                <td id="slave-rtt-cell-3"></td>
                <td id="slave-diff-cell-3"></td>
            </tr>
            <tr id="slave-data-row-4">
                <td id="slave-name-cell-4"></td>
                <td id="slave-time-cell-4"></td>
                <td id="slave-rtt-cell-4"></td>
                <td id="slave-diff-cell-4"></td>
            </tr>
        </table>

        <style>
            table {
                width: 100%;
                border-collapse: collapse;
            }
            th, td {
                padding: 8px;
                text-align: center;
                word-wrap: break-word;
            }
            th {
                background-color: #000000;
                color: #ffffff;  // Textfarbe auf Weiß ändern
            }
        </style>

        <script>
            function updateTable() {
                fetch('/getntpjson')
                    .then(response => response.json())
                    .then(data => {
                        let masterData = data[0].master[0];
                        let slaveData = data[1].slave;

                        // Aktualisieren Sie die Tabelle für den Master
                        let masterNameCell = document.getElementById('master-name-cell');
                        let masterTimeCell = document.getElementById('master-time-cell');
                        let masterRttCell = document.getElementById('master-rtt-cell');
                        let masterCell4 = document.getElementById('master-cell-4');
                        masterNameCell.innerHTML = masterData.name;
                        masterTimeCell.innerHTML = masterData.time;
                        masterRttCell.innerHTML = masterData.t_rtt;
                        masterCell4.innerHTML = ''; // Inhalt für 4. Spalte einfügen

                        // Aktualisieren Sie die Tabelle für die Slaves
                        let slaveNameCell1 = document.getElementById('slave-name-cell-1');
                        let slaveTimeCell1 = document.getElementById('slave-time-cell-1');
                        let slaveRttCell1 = document.getElementById('slave-rtt-cell-1');
                        let slaveDiffCell1 = document.getElementById('slave-diff-cell-1');
                        slaveNameCell1.innerHTML = slaveData[0].name;
                        slaveTimeCell1.innerHTML = slaveData[0].time;
                        slaveRttCell1.innerHTML = slaveData[0].t_rtt;
                        slaveDiffCell1.innerHTML = slaveData[0].t_dif;

                        let slaveNameCell2 = document.getElementById('slave-name-cell-2');
                        let slaveTimeCell2 = document.getElementById('slave-time-cell-2');
                        let slaveRttCell2 = document.getElementById('slave-rtt-cell-2');
                        let slaveDiffCell2 = document.getElementById('slave-diff-cell-2');
                        slaveNameCell2.innerHTML = slaveData[1].name;
                        slaveTimeCell2.innerHTML = slaveData[1].time;
                        slaveRttCell2.innerHTML = slaveData[1].t_rtt;
                        slaveDiffCell2.innerHTML = slaveData[1].t_dif;

                        let slaveNameCell3 = document.getElementById('slave-name-cell-3');
                        let slaveTimeCell3 = document.getElementById('slave-time-cell-3');
                        let slaveRttCell3 = document.getElementById('slave-rtt-cell-3');
                        let slaveDiffCell3 = document.getElementById('slave-diff-cell-3');
                        slaveNameCell3.innerHTML = slaveData[2].name;
                        slaveTimeCell3.innerHTML = slaveData[2].time;
                        slaveRttCell3.innerHTML = slaveData[2].t_rtt;
                        slaveDiffCell3.innerHTML = slaveData[2].t_dif;

                        let slaveNameCell4 = document.getElementById('slave-name-cell-4');
                        let slaveTimeCell4 = document.getElementById('slave-time-cell-4');
                        let slaveRttCell4 = document.getElementById('slave-rtt-cell-4');
                        let slaveDiffCell4 = document.getElementById('slave-diff-cell-4');
                        slaveNameCell4.innerHTML = slaveData[3].name;
                        slaveTimeCell4.innerHTML = slaveData[3].time;
                        slaveRttCell4.innerHTML = slaveData[3].t_rtt;
                        slaveDiffCell4.innerHTML = slaveData[3].t_dif;

                    })
                    .catch(error => console.error('Error:', error));
            }

            setInterval(updateTable, 1000);  // Aktualisierung alle 1 Sekunde
        </script>

    </body>
</html>
)";



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

    String html = String(html_header) + String(html_clients_neu);
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
    //std::vector<const char *> menu = {"wifi","info","param","sep","restart","erase","exit"};
    std::vector<const char *> menu = {"wifi","info","sep","custom", "sep","restart","erase","exit"};
    wm.setMenu(menu);

    wm.setTitle("NTP Client - Settings");
    wm.setHostname("NTPclient");

    // set dark theme
    wm.setClass("invert");

    wm.setConfigPortalBlocking(true);
    wm.setConfigPortalTimeout(6000);                                 // auto close configportal after n seconds
    
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