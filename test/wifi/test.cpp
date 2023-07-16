#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define LOG_LINES 5

WiFiManager wm;
char webLogCircularBuf[LOG_LINES][100] = {0};
String webLog = "";

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

void addStringToLogger(const char *str) {
    static unsigned currLogLine = 0;
    static int circularCounter;
    strcpy(webLogCircularBuf[currLogLine++], str);
    webLog = "";
    for (unsigned k = 0; k < LOG_LINES; k++) {
        circularCounter = (circularCounter + 1) % LOG_LINES;
        webLog += "<p style='font-size:50px;'>" + String(webLogCircularBuf[circularCounter]) + "</p>";
    }
    circularCounter = (circularCounter + 1) % LOG_LINES;
    if (currLogLine == LOG_LINES)
        currLogLine = 0;
}

void bindServerCallback(){
    wm.server->on("/logger",handleLoggerRoute);
}

void handleLoggerRoute(){
    char buf[400];
    sprintf(buf, logHTMLTags, webLog.c_str());
    wm.server->send(200, "text/html", buf);
}

void setup() {
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    // put your setup code here, to run once:
    Serial.begin(115200);

    //reset settings - wipe credentials for testing
    //wm.resetSettings();

    wm.setWebServerCallback(bindServerCallback);
    // set custom html menu content , inside menu item "custom", see setMenu()
    const char* menuhtml = "<form action='/logger' method='get'><button>Log</button></form><br/>\n";
    wm.setCustomMenuHTML(menuhtml);

    std::vector<const char *> menu = {"wifi","info", "custom", "exit", "sep","update"};
    wm.setMenu(menu);

    wm.setConfigPortalBlocking(false);
    wm.setConfigPortalTimeout(60);
    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("AutoConnectAP")){
        Serial.println("connected...yeey :)");
    }
    else {
        Serial.println("Configportal running");
    }
}

void loop() {
    static int q = 0;
    static int dummyNumber = 0;
    
    wm.process();

    if (millis() - q > 1000) {
        q = millis();
        addStringToLogger(String(dummyNumber++).c_str());
    }
}