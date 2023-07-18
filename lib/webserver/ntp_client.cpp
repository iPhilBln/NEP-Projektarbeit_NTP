#include "ntp_client.hpp"

std::vector<NTPClient*> NTPClient::masters;
std::vector<NTPClient*> NTPClient::slaves;

NTPClient::NTPClient(const String& serverName, const String& serverAddress, TYPE type, uint16_t serverPort)
    : _serverName(serverName), _serverAddress(serverAddress), _type(type), _serverPort(serverPort), _sockfd(-1) {
        Serial.println("Initialisiere :" + _serverName);
        if (setTimestamp()) {
            _isClient = true;
            _init     = false;
        }
    }

NTPClient::~NTPClient() {
    closeSocket();
}

/*      PRIVATE METHODS      */
bool NTPClient::init(void) {         
    // Socket erstellen
    if (_sockfd > -1) return true;

    Serial.println("Socket wird erstellt für: " + _serverName); 

    _sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sockfd < 0) {
        Serial.println("\tFehler beim Erstellen des Sockets.");
        return false;
    }

    // Server-Adresse und Port konfigurieren
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_serverPort);
    
    if (inet_pton(AF_INET, _serverAddress.c_str(), &(serverAddr.sin_addr)) <= 0) {
        Serial.println("\tUngültige Server-Adresse.");
        closeSocket();
        return false;
    }
    if(_init) Serial.println("\tIP-Adresse: " + String(_serverAddress));
    
    // Socket mit Server verbinden (normalerweise nicht notwendig für UDP, bei NTP scheinbar schon)
    if (connect(_sockfd, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        Serial.println("\tVerbindung zum Server fehlgeschlagen.");
        closeSocket();
        return false;
    }
    if(_init) Serial.println("\tSocketnr.: " + String(_sockfd));
    return true;
}

bool NTPClient::sendNTPRequest(void) {
    const uint32_t NTP_PACKET_SIZE = 48;
    uint8_t packetBuffer[NTP_PACKET_SIZE]{};

    // NTP-Paket vorbereiten
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;            // Stratum
    packetBuffer[2] = 6;            // Polling Interval
    packetBuffer[3] = 0xEC;         // Precision
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    // NTP-Paket senden
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_serverPort);

    if (sendto(_sockfd, packetBuffer, NTP_PACKET_SIZE, 0, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        Serial.println("\tFehler beim Senden des NTP-Pakets.");
        closeSocket();
        return false;
    }
    else {
        if(_init) Serial.println("\tAnfrage an " + String(_serverName) + " gesendet.");
        return true;
    }    
}

bool NTPClient::receiveNTPResponse(void) {
    const uint32_t NTP_PACKET_SIZE = 48;
    uint8_t packetBuffer[NTP_PACKET_SIZE]{};

    // Timeout-Wert in Sekunden (500 Millisekunden)
    int timeoutUSeconds = 500 * 1000;

    // Socket-Option für das Timeout festlegen
    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeoutUSeconds;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        Serial.println("\tFehler beim Festlegen des Sende-Timeouts.");
        closeSocket();
        return false;
    }

    // NTP-Antwort empfangen
    sockaddr_in serverAddr{};
    socklen_t serverAddrLen = sizeof(serverAddr);

    if (recvfrom(_sockfd, packetBuffer, NTP_PACKET_SIZE, 0, reinterpret_cast<sockaddr*>(&serverAddr), &serverAddrLen) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            Serial.println("\tTimeout beim Empfangen der NTP-Antwort.");
        } else {
            Serial.println("\tFehler beim Empfangen der NTP-Antwort.");
        }
        closeSocket();
        return false;
    }
    else {
        if(_init) Serial.println("\tAntwort erhalten.");
        // Auswertung der NTP-Antwort
        extractTimestamp(packetBuffer);
        return true;
    }
}

void NTPClient::extractTimestamp(const uint8_t* packetBuffer, int offset) {
    uint64_t timestamp = 0;
    // 64 Bit des Zeitstempels aus der Antwort extrahieren
    for (int i = 0; i < 8; ++i) {
        timestamp = (timestamp << 8) | packetBuffer[offset + i];
    }

    _timestamp = timestamp - NTP_TIMESTAMP_DELTA;
    
    if(_init) Serial.println("\tSpeicheraddresse: " + String((uintptr_t)&_timestamp, HEX));

    if(_init) Serial.print("\tTimestamp: "); if(_init) Serial.println(_timestamp);

    uint32_t seconds_helper = (_timestamp >> 32) & 0xFFFFFFFF;
    uint32_t fraction = _timestamp & 0xFFFFFFFF;
    if(_init) Serial.println("\tSekunden: " + String(seconds_helper));
    if(_init) Serial.println("\tBruchteil: " + String(fraction));

    if(_init) Serial.println("\tTimestamp[f]: " + timeToString(_timestamp));
}

/*      GETTER      */
bool NTPClient::getIsClient(void) {
    return _isClient;
}

uint8_t NTPClient::getClientsCount(void) {
    return masters.size() + slaves.size();
}

/*      SETTER      */
bool NTPClient::setTimestamp(void) {
    bool complete = true;
    if (complete) complete = init();
    if (complete) complete = sendNTPRequest();
    if (complete) complete = receiveNTPResponse();
    //closeSocket();
    return complete;
}

bool NTPClient::setTimestampRtt(void) {
    if (! setTimestamp()) return false;
    uint64_t timestampStart = _timestamp;
    if (! setTimestamp()) return false;
    _timestampRtt = _timestamp - timestampStart;
    return true;
}

bool NTPClient::setTimestampDifSlave(void) {
    if (masters.empty()) return false;
    
    NTPClient* master = masters.front();
    
    if (!master->setTimestampRtt()) return false;
    if (!setTimestampRtt()) return false;

    uint64_t rttMaster_05 = master->_timestampRtt >> 1; // rtt halbieren
    uint64_t rttSlave_05  = _timestampRtt >> 1;         // rtt halbieren

    uint64_t timestampMaster  = master->_timestamp; 
             timestampMaster -= rttMaster_05;
    uint64_t timestampSlave   = _timestamp;
             timestampSlave  -= 3 * rttSlave_05;
             timestampSlave  -= rttMaster_05;

    _timestampDif = static_cast<int64_t>(timestampSlave - timestampMaster); 

    return true;
}


/*      PUBLIC METHODS      */

void NTPClient::closeSocket() {
    close(_sockfd);
    _sockfd = -1;
}

void NTPClient::addMember(void) {
    _type == TYPE::MASTER ? masters.push_back(this) : slaves.push_back(this);
}

void NTPClient::removeMaster(void) {
    masters.clear();
}

void NTPClient::removeSlave(String serverName) {
    slaves.erase(std::remove_if(slaves.begin(), slaves.end(),
        [&](NTPClient* client) {
            return client->_serverName == serverName;
        }),
        slaves.end()
    );
}


String NTPClient::timeToString(uint64_t time) {
    uint32_t seconds_helper = (time >> 32) & 0xFFFFFFFF;
    uint32_t fraction = time & 0xFFFFFFFF;

    uint32_t hours = (seconds_helper / 3600) % 24;  // Stunden im 24-Stunden-Format
    uint32_t minutes = (seconds_helper / 60) % 60;
    uint32_t seconds = seconds_helper % 60;

    double fractionInSeconds = static_cast<double>(fraction) / 0xFFFFFFFF;

    double milliseconds = fractionInSeconds * 1000.0;
    double microseconds = fractionInSeconds * 1000000.0;
    double picoseconds = fractionInSeconds * 1000000000.0;

    uint32_t millisecondsUint = static_cast<uint32_t>(milliseconds);
    uint32_t microsecondsUint = static_cast<uint32_t>(microseconds) % 1000;
    uint32_t picosecondsUint = static_cast<uint32_t>(picoseconds) % 1000;

    char buffer[100];
    std::sprintf(buffer, "%02d:%02d:%02d.%03d:%03d:%03d", hours, minutes, seconds, millisecondsUint, microsecondsUint, picosecondsUint);
    return String(buffer);
}

String NTPClient::timeToString(int64_t time) {
    bool negativ;
    time < 0 ? negativ = true : negativ = false;
    if (negativ) time *= -1;

    uint32_t seconds_helper = (time >> 32) & 0xFFFFFFFF;
    uint32_t fraction = time & 0xFFFFFFFF;

    uint32_t hours = (seconds_helper / 3600) % 24;  // Stunden im 24-Stunden-Format
    uint32_t minutes = (seconds_helper / 60) % 60;
    uint32_t seconds = seconds_helper % 60;

    double fractionInSeconds = static_cast<double>(fraction) / 0xFFFFFFFF;

    double milliseconds = fractionInSeconds * 1000.0;
    double microseconds = fractionInSeconds * 1000000.0;
    double picoseconds = fractionInSeconds * 1000000000.0;

    uint32_t millisecondsUint = static_cast<uint32_t>(milliseconds);
    uint32_t microsecondsUint = static_cast<uint32_t>(microseconds) % 1000;
    uint32_t picosecondsUint = static_cast<uint32_t>(picoseconds) % 1000;


    char buffer[100];
    std::sprintf(buffer, "%02d:%02d:%02d.%03d:%03d:%03d", hours, minutes, seconds, millisecondsUint, microsecondsUint, picosecondsUint);
    return negativ ? "-" + String(buffer) : String(buffer);
}



String NTPClient::toJSON(void) {
    String json = "[";

    json += "{\"master\":[";

    int i = 0;
    for(NTPClient* master : masters) {
        json += "{\"name\":\"" + master->_serverName + "\",";
        json +=  "\"time\":\"" + master->timeToString(master->_timestamp) + "\",";
        json +=  "\"t_rtt\":\"" + master->timeToString(master->_timestampRtt) + "\"}";
        if (i < masters.size() - 1) {
            json += ",";
        }
        i++;
    }
    json += "]},";

    json += "{\"slave\":[";
    i = 0;
    for(NTPClient* slave : slaves) {
        json += "{\"name\":\"" + slave->_serverName + "\",";
        json +=  "\"time\":\"" + slave->timeToString(slave->_timestamp) + "\",";
        json +=  "\"t_rtt\":\"" + slave->timeToString(slave->_timestampRtt) + "\",";
        json +=  "\"t_dif\":\"" + slave->timeToString(slave->_timestampDif) + "\"}";
        if (i < slaves.size() - 1) {
            json += ",";
        }
        i++;
    }
    json += "]}";

    json += "]";
    return json;
}

