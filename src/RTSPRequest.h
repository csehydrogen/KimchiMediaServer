#pragma once

#include <string>
#include <map>

class RTSPRequest {
    std::string method, url, filepath, srcIPAddr, version;
    std::map<std::string, std::string> headers;
    unsigned short clientRTPPort, clientRTCPPort;
    double startNptTime;

public:
    RTSPRequest();
    ~RTSPRequest();

    void setMethod(char *ptr);
    void setURL(char *ptr);
    void setVersion(char *ptr);
    void addHeaders(char *key, char *val);

    std::string getMethod();
    std::string getUrl();
    std::string getFilepath();
    std::string getSrcIPAddr();
    std::string getVersion();
    std::map<std::string, std::string> getHeadersAll();
    std::string getHeader(const char *key);
    
    unsigned short getClientRTPPort();
    unsigned short getClientRTCPPort();
    double getStartNptTime();

    void printLog();
};
