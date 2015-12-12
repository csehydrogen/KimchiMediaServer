#pragma once

#include <string>
#include <map>

class RTSPRequest {
    std::string method, url, filepath, srcIPAddr, version;
    std::map<std::string, std::string> headers;

    std::string session, timeout; // replace this line with session class
    std::string dstIPAddress, serverPort; // for SETUP Response
    std::string seq, rtptime; // for PLAY Response
public:
    void setMethod(char *ptr);
    void setURL(char *ptr);
    void setVersion(char *ptr);
    void addHeaders(char *key, char *val);

    std::string getResponse();

    std::string getCreationTimestring();
    std::string getSDPDescription();

    // the methods that would be modified are below; 
    void setDstIPAddress(std::string _dstIPAddress, std::string _serverPort);
    void setSession(std::string _session);
    void setTimeout(std::string _timeout);

    void printLog();
};
