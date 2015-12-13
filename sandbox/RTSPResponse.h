#pragma once

#include <string>
#include "RTSPRequest.h"

class RTSPResponse {
	RTSPRequest* rtspRequest;
	char timebuf[128];
	std::string common;

public:
	RTSPResponse(RTSPRequest* _rtspRequest);
	~RTSPResponse();
	
	std::string getOPTIONS();
	std::string getDESCRIBE(double maxRange);
	std::string getSETUP(char const *clientIPAddr, unsigned short serverRTPPort, unsigned short serverRTCPPort, char const *sessionKey);
	std::string getPLAY(double startNpt, unsigned short seq, unsigned int rtptime);
	std::string getPAUSE();
	std::string getGET_PARAMETER();
	std::string getTEARDOWN();
   
    std::string getResponse();
    std::string getCreationTimeString();
    std::string getSDPDescription(double maxRange);

    void printLog();
};
