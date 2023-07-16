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
    const char* _serverName;
    const char* _serverAddress;
    TYPE        _type;
    uint16_t    _serverPort;
    int         _sockfd;
    uint64_t    _timestamp;
    uint64_t    _timestampDif[3];
    bool        _isClient = false; 

    const   uint8_t     _clientsCountMax = 20;
    static  uint16_t    _clientsCount;


    bool init(void);
    void sendNTPRequest(void);
    void receiveNTPResponse(void);
    
    //bool    setIsClient(bool isClient);
public:
    static  std::vector<NTPClient> masters;
    static  std::vector<NTPClient> slaves;
    
    NTPClient(const char* serverName, const char* serverAddress, TYPE type, uint16_t serverPort = 123);
    ~NTPClient();

    bool    getIsClient(void);
    uint8_t getClientsCount(TYPE type);
    void    getTimestamp(void);

    void    setTimestamp(const uint8_t* packetBuffer, int offset = 40);
    
    void    closeSocket(void);
    String  timeToString(void);
    
    static String toJSONmaster(void);
    static String toJSONslave(void);


};

#endif
