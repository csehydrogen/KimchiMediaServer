#include "RTSPRequest.h"
#include <cstdlib>
#include <cstring>

RTSPRequest::RTSPRequest() {
    startNptTime = -1.0;
}

RTSPRequest::~RTSPRequest() { }

void RTSPRequest::setMethod(char *ptr) {
    method = std::string(ptr);
}

void RTSPRequest::setURL(char *ptr) {
    int count;

    url = std::string(ptr);

    std::string::iterator iter_st, iter_ed;
    count = 0;
    iter_ed = url.end();
    for (auto it = url.begin() ; it != url.end() ; ++it) {
        if (*it == '/') {
            count++;
            if (count==3) {
                iter_st = next(it);
            }
            if (count==4) {
                iter_ed = it;
            }
        }
    }
    if(count<3) filepath="";
    else filepath = std::string(iter_st, iter_ed);

    count=0;
    std::string::iterator iter1, iter2 = url.end();
    for(auto iter = url.begin() ; iter != url.end() ; iter++) {
        if(*iter == '/') {
            count++;
            if(count==2) {
                iter1 = next(iter);
                continue;
            }
        }
        if((*iter == ':' || *iter == '/') && count==2) {
            iter2 = iter;
            break;
        }
    }
    srcIPAddr = std::string(iter1, iter2);
}

void RTSPRequest::setVersion(char *ptr) {
    version = std::string(ptr);
}

void RTSPRequest::addHeaders(char *key, char *val) {
    headers[std::string(key)] = std::string(val);
    
    if(method == "SETUP" && strcmp(key,"Transport") == 0) {
        int n = strlen(val);
        int loc0=0, loc1=0, loc2=n;
        for(int i=0 ; i<n ; i++) {
            if(val[i]=='=') {
                loc0 = i+1;
                break;
            }
        }
        
        for(int i=loc0 ; i<n ; i++) {
            if(val[i]=='-') {
                loc1 = i;
                break;
            }
        }
        
        for(int i=loc1 ; i<n ; i++) {
            if(val[i]==';') {
                loc2 = i;
                break;
            }
        }
        
        std::string rtpPort(val+loc0, val+loc1);
        std::string rtcpPort(val+loc1+1, val+loc2);
        
        clientRTPPort = (unsigned short)atoi(rtpPort.c_str());
        clientRTCPPort = (unsigned short)atoi(rtcpPort.c_str());
    }
    else if(method == "PLAY" && strcmp(key,"Range") == 0) {
        int n =strlen(val);
        int loc0=0, loc1=n;
        for(int i=0 ; i<n ; i++) {
            if(val[i] == '=') {
                loc0 = i+1;
                break;
            }
        }
        for(int i=loc0 ; i<n ; i++) {
            if(val[i]=='-') {
                loc1 = i;
                break;
            }
        }
        
        std::string npt(val+loc0, val+loc1);
        startNptTime = atof(npt.c_str());
    }
}

std::string RTSPRequest::getMethod() { 
    return method;
}

std::string RTSPRequest::getUrl() {
    return url;
}

std::string RTSPRequest::getFilepath() {
    return filepath;
}

std::string RTSPRequest::getSrcIPAddr() {
    return srcIPAddr;
}

std::string RTSPRequest::getVersion() {
    return version;
}

std::map<std::string, std::string> RTSPRequest::getHeadersAll() {
    return headers;
}

std::string RTSPRequest::getHeader(const char *key) {
    return headers[std::string(key)];
}

unsigned short RTSPRequest::getClientRTPPort() {
    return clientRTPPort;
}

unsigned short RTSPRequest::getClientRTCPPort() {
    return clientRTCPPort;
}

double RTSPRequest::getStartNptTime() {
    return startNptTime;
}

void RTSPRequest::printLog() {
    printf(
      "Method : %s\n"
      "Request-URI : %s\n"
      "RTSP-Version : %s\n"
      "filepath : %s\n"
      "source IPAddress : %s\n",
      method.c_str(), url.c_str(), version.c_str(), filepath.c_str(), srcIPAddr.c_str());
  
      printf("Headers info are below;\n");
      auto iter = headers.begin();
      for(iter=headers.begin() ; iter != headers.end() ; iter++) {
          auto pss = *iter;
          printf("%s : %s\n", pss.first.c_str(), pss.second.c_str());
      }
}
