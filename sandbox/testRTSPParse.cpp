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
  map<string,string> headers; // headers
  string filename;

  void add_headers(char *buf);
  void setFilename();
  string getResponse();

  string getCreationTimeString();
  string getIPAddress();
  string getSDPDescription();
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
  // TODO : this should be done when the session is created

  string timeString;
  timeval ctime;
  gettimeofday(&ctime,NULL);
  
  char buf[256];
  sprintf(buf, "%ld%06ld", ctime.tv_sec, ctime.tv_usec);
  timeString = string(buf);

  return timeString;
}

string RTSPRequest::getIPAddress() {
  // TODO : this sholud be modified

  int count=0;
  string::iterator iter1, iter2;
  for(auto iter = request_URI.begin() ; iter != request_URI.end() ; iter++) {
    if(*iter == '/') {
      count++;
      if(count==2) {
        iter1 = ++iter;
        --iter;
      }
    }
    if(*iter == ':' && count==2) {
      iter2 = iter;
      break;
    }
  }

  return string(iter1, iter2);
}

string RTSPRequest::getSDPDescription() {

  string sdpd("");

  sdpd += "v=0\r\n";
  sdpd += "o=- " + getCreationTimeString() + " 1 IN IP4 " + getIPAddress() + "\r\n";
  sdpd += "s=Session streamed by \"KimchiMediaServer\"\r\n";
  sdpd += "i=" + filename + "\r\n";
  sdpd += "t=0 0\r\n";
  sdpd += "a=tool:Kimchi Streaming Media\r\n";
  sdpd += "a=type:broadcast\r\n";
  sdpd += "a=control:*\r\n";
  sdpd += "a=range:npt=0-x.xxx (TODO)\r\n"; // TODO
  sdpd += "a=x-qt-text-nam:Session streamed by \"KimchiMediaServer\"\r\n";
  sdpd += "a=x-qt-text-inf:" + filename + "\r\n";
  sdpd += "H2 drives his hydrogen_bus in Project1 (TODO)\r\n";

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
    // TODO
  }
  else if(method == "TEARDOWN") {
    ret += "\r\n";
  }
  else if(method == "PLAY") {
    // TODO
  }
  else if(method == "PAUSE") {
    ret += "Session: " + headers["Session"];
    ret += "\r\n";
  }
  else if(method == "GET_PARAMETER") {
    // TODO
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
    "filename : %s\n", 
    r.method.c_str(), r.request_URI.c_str(), r.RTSP_version.c_str(), r.filename.c_str());
  
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
