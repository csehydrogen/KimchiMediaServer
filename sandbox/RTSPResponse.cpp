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
    
    ret += "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n\r\n";
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

std::string RTSPResponse::getSETUP() {
    std::string ret(common);
    
    // TODO
    // Transport
    // Session
    
    return ret;    
}

std::string RTSPResponse::getPLAY() {
    std::string ret(common);
    
    // TODO
    // range
    // session
    // RTP-INFO: url=~~~;seq=~~~;rtptime=~~~\r\n
    // \r\n
    
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