#include "RTSPResponse.h"
#include <sys/time.h>
#include <unistd.h>

RTSPResponse::RTSPResponse(RTSPRequest* _rtspRequest) : rtspRequest(_rtspRequest) { 
    time_t tt = time(NULL);
    strftime(timebuf, sizeof(timebuf), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));

    common = rtspRequest->getVersion() + " 200 OK\r\n"
            + "CSeq: " + rtspRequest->getHeader("CSeq") + "\r\n"
            + std::string(timebuf);
}
RTSPResponse::~RTSPResponse() { }

std::string RTSPResponse::getOPTIONS() {
    std::string ret(common);
    
    ret += "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER, REDIRECT\r\n\r\n";
    return ret;
}

std::string RTSPResponse::getDESCRIBE(double maxRange = -1.0) {
    std::string ret(common);
    std::string sdps = getSDPDescription(maxRange);
    
    ret += "Content-Base: " + rtspRequest->getUrl() + "/\r\n"
            + "Content-type: application/sdp\r\n"
            + "Content-length: " + std::to_string(sdps.size()) + "\r\n\r\n"
            + sdps;
    
    return ret;   
}

std::string RTSPResponse::getSETUP(char const *clientIPAddr, unsigned short serverRTPPort, unsigned short serverRTCPPort, char const *sessionKey) {
    std::string ret(common);
    std::string transport, preTransport;
    
    transport = rtspRequest->getHeader("Transport");
    int count=0;
    for(auto iter = transport.begin() ; iter != transport.end() ; iter++) {
        if(*iter == ';') count++;
        if(count==2) {
            preTransport = std::string(transport.begin(), iter);
            break;
        }
    }
    
    ret += "Transport: " + preTransport
            + ";destination=" + clientIPAddr
            + ";source=" + rtspRequest->getSrcIPAddr()
            + ";client_port=" + std::to_string(rtspRequest->getClientRTPPort()) + "-" + std::to_string(rtspRequest->getClientRTCPPort())
            + ";server_port=" + std::to_string(serverRTPPort) + "-" + std::to_string(serverRTCPPort) + "\r\n"
            + "Session: " + sessionKey + "\r\n\r\n";  
            
    return ret;    
}

std::string RTSPResponse::getPLAY(double startNpt, unsigned short seq, unsigned int rtptime) {
    std::string ret(common);
    
    ret += "Range: npt=" + std::to_string(startNpt) + "-\r\n"
            + "Session: " + rtspRequest->getHeader("Session") + "\r\n"
            + "RTP-INFO: url=" + rtspRequest->getUrl() + "track1;seq=" + std::to_string(seq) + ";rtptime=" + std::to_string(rtptime) + "\r\n\r\n";
    
    return ret;
}

std::string RTSPResponse::getPAUSE() {
    std::string ret(common);
    
    ret += "Session: " + rtspRequest->getHeader("Session") + "\r\n\r\n";
    
    return ret;
}

std::string RTSPResponse::getGET_PARAMETER() {
    std::string ret(common);
    
    ret += "Session: " + rtspRequest->getHeader("Session") + "\r\n"
            + "Content-length: 0\r\n\r\n";
    
    return ret;
}

std::string RTSPResponse::getTEARDOWN() {
    std::string ret(common);
    
    ret += "\r\n";
    
    return ret;
}

std::string RTSPResponse::getREDIRECT(const char *ip, unsigned short port, const char *filepath) {
    std::string ret;
    
    ret = "RTSP/1.0 301 Moved Permanently\r\n";
    ret += "Location: rtsp://" + std::string(ip) + ":" 
            + std::to_string(port) + "/" 
            + std::string(filepath) + "\r\n\r\n";
    
    return ret;
}

std::string RTSPResponse::getCreationTimeString() {
    std::string timeString;
    timeval ctime;
    gettimeofday(&ctime,NULL);
  
    char buf[256];
    sprintf(buf, "%ld%06ld", ctime.tv_sec, ctime.tv_usec);
    timeString = std::string(buf);

    return timeString;
}

std::string RTSPResponse::getSDPDescription(double maxRange) {
    std::string sdpd;
   
    sdpd += std::string("v=0\r\n")
            + "o=- " + getCreationTimeString() + " 1 IN IP4 " + (rtspRequest->getSrcIPAddr()) + "\r\n"
            + "s=Session streamed by \"KimchiMediaServer\"\r\n"
            + "i=" + (rtspRequest->getFilepath()) + "\r\n"
            + "t=0 0\r\n"
            + "a=tool:Kimchi Streaming Media\r\n"
            + "a=type:broadcast\r\n"
            + "a=control:*\r\n"
            + "a=range:npt=0-" + (maxRange<0?"":std::to_string(maxRange)) + "\r\n"
            + "m=video 0 RTP/AVP 33\r\n" // payload type of TS files = 33
            + "c=IN IP4 0.0.0.0\r\n"
            + "a=control:track1\r\n";   
    // sdpd += "a=x-qt-text-nam:Session streamed by \"KimchiMediaServer\"\r\n";
    // sdpd += "a=x-qt-text-inf:" + filepath + "\r\n";
    // sdpd += "b = XX:XXX\r\n"; // estimates bandwidth from video file
    
    return sdpd;
}
