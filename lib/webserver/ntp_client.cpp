#include "ntp_client.hpp"

uint16_t NTPClient::_clientsCount = 0;
std::vector<NTPClient> NTPClient::masters;
std::vector<NTPClient> NTPClient::slaves;

NTPClient::NTPClient(const char* serverName, const char* serverAddress, TYPE type, uint16_t serverPort)
    : _serverName(serverName), _serverAddress(serverAddress), _type(type), _serverPort(serverPort), _sockfd(-1) {
        Serial.println("Initialize :" + String(serverName));
        if (_clientsCount < _clientsCountMax) {
            if (init()) {
                _type == TYPE::MASTER ? masters.push_back(*this) : slaves.push_back(*this);
                _clientsCount++;
                _isClient = true;
                getTimestamp();
            }
        }
        getIsClient();
    }


NTPClient::~NTPClient() {
    closeSocket();
}

/*      GETTER      */
bool NTPClient::getIsClient(void) {
    return _isClient;
}

void NTPClient::getTimestamp(void) {
    sendNTPRequest();
    receiveNTPResponse();
    Serial.println("Timestamp[f]: " + timeToString());
}

uint8_t NTPClient::getClientsCount(TYPE type) {
    return type == TYPE::MASTER ? masters.size() : slaves.size();
}

/*      SETTER      */
void NTPClient::setTimestamp(const uint8_t* packetBuffer, int offset) {
    uint64_t timestamp = 0;
    for (int i = 0; i < 8; ++i) {
        timestamp = (timestamp << 8) | packetBuffer[offset + i];
    }

    _timestamp = timestamp - NTP_TIMESTAMP_DELTA;
}

/*      PRIVATE METHODS      */
bool NTPClient::init(void) {   
    // Socket erstellen
    _sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sockfd < 0) {
        Serial.println("Fehler beim Erstellen des Sockets.");
        return false;
    }

    // Server-Adresse und Port konfigurieren
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(_serverPort);
    if (inet_pton(AF_INET, _serverAddress, &(serverAddr.sin_addr)) <= 0) {
        Serial.println("Ungültige Server-Adresse.");
        close(_sockfd);
        return false;
    }

    // Socket mit Server verbinden (nicht notwendig für UDP)
    //if (connect(_sockfd, reinterpret_cast<const sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
    //    Serial.println("Verbindung zum Server fehlgeschlagen.");
    //    close(_sockfd);
    //    return false;
    //}
    Serial.println(_sockfd);
    return true;
}

void NTPClient::sendNTPRequest(void) {
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
        Serial.println("Fehler beim Senden des NTP-Pakets.");
    }
    else {
        Serial.println("Anfrage gesendet.");
    }    
}

void NTPClient::receiveNTPResponse(void) {
    const uint32_t NTP_PACKET_SIZE = 48;
    uint8_t packetBuffer[NTP_PACKET_SIZE]{};

    // Timeout-Wert in Sekunden (5 Sekunden)
    int timeoutSeconds = 5;

    // Socket-Option für das Timeout festlegen
    timeval timeout;
    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = 0;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        Serial.println("Fehler beim Festlegen des Sende-Timeouts.");
        return;
    }

    // NTP-Antwort empfangen
    sockaddr_in serverAddr{};
    socklen_t serverAddrLen = sizeof(serverAddr);

    if (recvfrom(_sockfd, packetBuffer, NTP_PACKET_SIZE, 0, reinterpret_cast<sockaddr*>(&serverAddr), &serverAddrLen) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            Serial.println("Timeout beim Empfangen der NTP-Antwort.");
        } else {
            Serial.println("Fehler beim Empfangen der NTP-Antwort.");
        }
        return;
    }

    Serial.println("Antwort erhalten.");
    // Auswertung der NTP-Antwort
    setTimestamp(packetBuffer);
}


/*      PUBLIC METHODS      */


void NTPClient::closeSocket() {
    close(_sockfd);
}

String NTPClient::timeToString(void) {
    Serial.print("Timestamp: "); Serial.println(_timestamp);

    uint32_t seconds_helper = (_timestamp >> 32) & 0xFFFFFFFF;
    uint32_t fraction = _timestamp & 0xFFFFFFFF;

    uint32_t hours = (seconds_helper / 3600) % 24;  // Stunden im 24-Stunden-Format
    uint32_t minutes = (seconds_helper / 60) % 60;
    uint32_t seconds = seconds_helper % 60;

    uint32_t milliseconds = (fraction * 1000) / 0xFFFFFFFF;
    uint32_t microseconds = (fraction * 1000000) / 0xFFFFFFFF;
    uint32_t picoseconds = (fraction * 1000000000) / 0xFFFFFFFF;

    char buffer[100];
    std::sprintf(buffer, "%02d:%02d:%02d.%03d:%03d:%03d", hours, minutes, seconds, milliseconds, microseconds, picoseconds);
    return String(buffer);
}

String NTPClient::toJSONmaster(void) {
    String json = "[";

    //for(const NTPClient& client : NTPClient::clients) {
    int i = 0;
    for(NTPClient& master : masters) {
        json += "{  \"name\":\"" + String(master._serverName) + "\",";
        json +=    "\"time\":\"" + master.timeToString() + "}";
        if (i < masters.size() - 1) {
            json += ",";
        }
        i++;
    }
    json += "]";
    return json;
}

String NTPClient::toJSONslave(void) {
    String json = "[";

    //for(const NTPClient& client : NTPClient::clients) {
    int i = 0;
    for(NTPClient& slave : slaves) {
        json += "{  \"name\":\"" + String(slave._serverName) + "\",";
        json +=    "\"time\":\"" + slave.timeToString() + "}";
        if (i < slaves.size() - 1) {
            json += ",";
        }
        i++;
    }
    json += "]";
    return json;
}
