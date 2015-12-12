#include "RTSPRequest.h"

/*
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <ctime>
*/


#include <sys/time.h>
#include <unistd.h>

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
    filepath = std::string(iter_st, iter_ed);

    count=0;
    std::string::iterator iter1, iter2;
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
}

std::string RTSPRequest::getResponse() {
  std::string ret;

  char timebuf[128];
  time_t tt = time(NULL);
  strftime(timebuf, sizeof(timebuf), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));  

  ret += version + " 200 OK\r\n";
  ret += "CSeq: " + headers["CSeq"] + "\r\n";
  ret += std::string(timebuf);

  if(method == "OPTIONS") {
    ret += std::string("Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n"); 
    ret += "\r\n";
  }
  else if(method == "DESCRIBE") {
    std::string sdps = getSDPDescription();

    ret += "Content-Base: " + url + "/\r\n";
    ret += "Content-type: application/sdp\r\n";
    ret += "Content-length: " + std::to_string(sdps.size()) + "\r\n\r\n";
    ret += getSDPDescription();  
  }
  else if(method == "SETUP") {
    std::string transPort, preTransport, clientPort;
    std::string::iterator iter1, iter2;
    transPort = headers["Transport"];

    int count=0;
    for(auto iter = transPort.begin() ; iter != transPort.end() ; iter++) {
      if(*iter == ';') count++;
      if(count==2) {
        iter1 = iter;
        iter2 = next(iter);
        break;
      }
    }
    preTransport = std::string(transPort.begin(), iter1);
    clientPort = std::string(iter2, transPort.end());
    
    ret += "Transport: " + preTransport;
    ret += ";" + dstIPAddress + ";" + srcIPAddr + ";" + clientPort + ";" + serverPort + "\r\n";
    ret += "Session: " + session + ";timeout=" + timeout + "\r\n";
    ret += "\r\n";
  }
  else if(method == "TEARDOWN") {
    ret += "\r\n";
  }
  else if(method == "PLAY") {
    ret += "Range: " + headers["Range"] + "\r\n";
    ret += "Session: " + session + "\r\n";
    // TODO : RTP-Info
    ret += "\r\n";
  }
  else if(method == "PAUSE") {
    ret += "Session: " + headers["Session"] + "\r\n";
    ret += "\r\n";
  }
  else if(method == "GET_PARAMETER") {
    ret += session + "\r\n";
    // TODO : Content-length \r\n\r\n Content
    ret += "\r\n";
  }
  else if(method == "SET_PARAMETER") {
    // this is not necessary
  }
  else {
    // does not support other method
  }
  
  return ret;
}

std::string RTSPRequest::getCreationTimestring() {
  std::string timestring;
  timeval ctime;
  gettimeofday(&ctime,NULL);
  
  char buf[256];
  sprintf(buf, "%ld%06d", ctime.tv_sec, ctime.tv_usec);
  timestring = std::string(buf);

  return timestring;
}


std::string RTSPRequest::getSDPDescription() {

  std::string sdpd("");

  sdpd += "v=0\r\n";
  sdpd += "o=- " + getCreationTimestring() + " 1 IN IP4 " + srcIPAddr + "\r\n";
  sdpd += "s=Session streamed by \"KimchiMediaServer\"\r\n";
  sdpd += "i=" + filepath + "\r\n";
  sdpd += "t=0 0\r\n";
  sdpd += "a=tool:Kimchi Streaming Media\r\n";
  sdpd += "a=type:broadcast\r\n";
  sdpd += "a=control:*\r\n";
  sdpd += "a=range:npt=0-x.xxx (TODO)\r\n"; // TODO : get maxRange from video file
  // sdpd += "a=x-qt-text-nam:Session streamed by \"KimchiMediaServer\"\r\n";
  // sdpd += "a=x-qt-text-inf:" + filepath + "\r\n";
  sdpd += "m=video 0 RTP/AVP 33\r\n"; // payload type of TS files = 33
  sdpd += "c=IN IP4 0.0.0.0\r\n";
  // sdpd += "b = XX:XXX\r\n"; // estimates bandwidth from video file
  sdpd += "a=control:track1\r\n";

  return sdpd;
}

/*

int main() {
 
  RTSPRequest r;

  for(int line=0 ; ; line++) 
  {
    fgets(buf,BUF_MAX-1,stdin);
    int n = strlen(buf);
    if(n<=2) break;

    r.add_headers(buf);
  }
  printf(
    "Method : %s\n"
    "Request-URI : %s\n"
    "RTSP-Version : %s\n"
    "filepath : %s\n"
    "source IPAddress : %s\n",
    r.method.c_str(), r.request_URI.c_str(), r.RTSP_version.c_str(), r.filepath.c_str(), r.srcIPAddress.c_str());
  
    std::map<std::string,std::string>::iterator iter;
  for(iter=r.headers.begin() ; iter != r.headers.end() ; iter++) {
    pair<std::string,std::string> pss = *iter;
    printf("%s : %s\n", pss.first.c_str(), pss.second.c_str());
  }


  printf("\n=========================\n\n");

  std::string response = r.getResponse();
  printf("%s", response.c_str());

  for(int i=0 ; i<response.length() ; i++) {
    printf("%02x", response[i]);
  }
  printf("\n");
  return 0;
}
*/
