#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

const int BUF_MAX = 11111;
char buf[BUF_MAX];

struct RTSPRequest {
  string method, request_URI, RTSP_version; // Request-Line
  string filename, srcIPAddress;
  map<string,string> headers; // headers

  string session, timeout; // replace this line with session class
  string dstIPAddress, serverPort; // for SETUP Response

  void add_headers(char *buf);
  string getResponse();

  string getCreationTimeString();
  string getSDPDescription();

  void setDstIPAddress(string _dstIPAddress, string _serverPort);
  void setSession(string _session);
  void setTimeout(string _timeout);

private:
  void setFilename();
  void setSrcIPAddress();
};

void RTSPRequest::add_headers(char *bufline) {
  bool isRequestLine = false;
  int n = strlen(bufline);
  if(n<=2) return;

  char *pch = strtok(bufline, " :\r\n");
  if( strcmp(pch,"OPTIONS") == 0 || strcmp(pch,"DESCRIBE") == 0 
    || strcmp(pch,"SETUP") == 0 || strcmp(pch,"TEARDOWN") == 0
    || strcmp(pch,"PLAY") == 0 || strcmp(pch,"PAUSE") == 0
    || strcmp(pch,"GET_PARAMETER") == 0 || strcmp(pch,"SET_PARAMETER") == 0
  ) isRequestLine = true;
  
  for(int j=0 ; pch != NULL ; j++) {
    if(isRequestLine) {
      if(j==0) method = string(pch);
      else if(j==1) {
        request_URI = string(pch);
        setFilename();
        setSrcIPAddress();
      }
      else RTSP_version = string(pch);
      pch = strtok(NULL, " \r\n");
    }
    else {
      string s1(pch);
      pch = strtok(NULL, ":\r\n");
      string s2(pch+1);
      pch = strtok(NULL, ":\r\n");
      headers[s1] = s2;
    }
  }
}

void RTSPRequest::setFilename() {
  string::iterator iter_st, iter_ed;
  int count=0;

  iter_ed = request_URI.end();
  for(auto iter = request_URI.begin() ; iter != request_URI.end() ; iter++) {
    if(*iter == '/') {
      count++;
      if(count==3) {
        iter_st = std::next(iter);
      }
      if(count==4) {
        iter_ed = iter;
      }
    }
  }    

  filename = string(iter_st, iter_ed);
}

string RTSPRequest::getCreationTimeString() {
  string timeString;
  timeval ctime;
  gettimeofday(&ctime,NULL);
  
  char buf[256];
  sprintf(buf, "%ld%06ld", ctime.tv_sec, ctime.tv_usec);
  timeString = string(buf);

  return timeString;
}

void RTSPRequest::setSrcIPAddress() {
  int count=0;
  string::iterator iter1, iter2;
  for(auto iter = request_URI.begin() ; iter != request_URI.end() ; iter++) {
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

  srcIPAddress = string(iter1, iter2);
}

string RTSPRequest::getSDPDescription() {

  string sdpd("");

  sdpd += "v=0\r\n";
  sdpd += "o=- " + getCreationTimeString() + " 1 IN IP4 " + srcIPAddress + "\r\n";
  sdpd += "s=Session streamed by \"KimchiMediaServer\"\r\n";
  sdpd += "i=" + filename + "\r\n";
  sdpd += "t=0 0\r\n";
  sdpd += "a=tool:Kimchi Streaming Media\r\n";
  sdpd += "a=type:broadcast\r\n";
  sdpd += "a=control:*\r\n";
  sdpd += "a=range:npt=0-x.xxx (TODO)\r\n"; // TODO : get maxRange from video file
  // sdpd += "a=x-qt-text-nam:Session streamed by \"KimchiMediaServer\"\r\n";
  // sdpd += "a=x-qt-text-inf:" + filename + "\r\n";
  sdpd += "m=video 0 RTP/AVP 33\r\n"; // payload type of TS files = 33
  sdpd += "c=IN IP4 0.0.0.0\r\n";
  // sdpd += "b = XX:XXX\r\n"; // estimates bandwidth from video file
  sdpd += "a=control:track1\r\n";

  return sdpd;
}

string RTSPRequest::getResponse() {
  string ret("");
  
  char timebuf[200];
  time_t tt = time(NULL);
  strftime(timebuf, sizeof(timebuf), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));  

  ret += RTSP_version + " 200 OK\r\n";
  ret += "CSeq: " + headers["CSeq"] + "\r\n";
  ret += string(timebuf);

  if(method == "OPTIONS") {
    ret += string("Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n"); 
    ret += "\r\n";
  }
  else if(method == "DESCRIBE") {
    string sdps = getSDPDescription();

    ret += "Content-Base: " + request_URI + "/\r\n";
    ret += "Content-type: application/sdp\r\n";
    ret += "Content-length: " + to_string(sdps.size()) + "\r\n\r\n";
    ret += getSDPDescription();  
  }
  else if(method == "SETUP") {
    string transPort, preTransport, clientPort;
    string::iterator iter1, iter2;
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
    preTransport = string(transPort.begin(), iter1);
    clientPort = string(iter2, transPort.end());
    
    ret += "Transport: " + preTransport;
    ret += ";" + dstIPAddress + ";" + srcIPAddress + ";" + clientPort + ";" + serverPort + "\r\n";
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
    "filename : %s\n"
    "source IPAddress : %s\n",
    r.method.c_str(), r.request_URI.c_str(), r.RTSP_version.c_str(), r.filename.c_str(), r.srcIPAddress.c_str());
  
  map<string,string>::iterator iter;
  for(iter=r.headers.begin() ; iter != r.headers.end() ; iter++) {
    pair<string,string> pss = *iter;
    printf("%s : %s\n", pss.first.c_str(), pss.second.c_str());
  }


  printf("\n=========================\n\n");

  string response = r.getResponse();
  printf("%s", response.c_str());

/*
  for(int i=0 ; i<response.length() ; i++) {
    printf("%02x", response[i]);
  }
  printf("\n");
*/
  return 0;
}
