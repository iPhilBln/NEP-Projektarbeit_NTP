#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define NTP_TIMESTAMP_DELTA 2208988800ULL

enum class TYPE {
    MASTER,
    SLAVE
};

class NTPClient {
private:
    String      _serverName;
    String      _serverAddress;
    TYPE        _type;
    uint16_t    _serverPort;

    int         _sockfd;
    bool        _isClient = false; 
    bool        _init = true;
    uint64_t    _timestamp;
    int64_t     _timestampDif;
    uint64_t    _timestampRtt;

    bool init(void);
    bool sendNTPRequest(void);
    bool receiveNTPResponse(void);
    void extractTimestamp(const uint8_t* packetBuffer, int offset = 40);
    
    //bool    setIsClient(bool isClient);
public:
    NTPClient(const String& serverName, const String& serverAddress, TYPE type, uint16_t serverPort = 123);
    ~NTPClient();

    static  std::vector<NTPClient*> masters;
    static  std::vector<NTPClient*> slaves;

    /*      GETTER      */
    bool    getIsClient(void);
    uint8_t getClientsCount(void);
    
    /*      SETTER      */
    bool    setTimestamp(void);
    bool    setTimestampRtt(void);
    bool    setTimestampDifSlave(void);
    
    /*      PUBLIC METHODS      */
    void        closeSocket(void);
    void        addMember(void);
    static void removeMaster(void);
    static void removeSlave(String serverName);
    String      timeToString(uint64_t time);
    String      timeToString(int64_t time);
    
    static String toJSON(void);
};

#endif
