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
	std::string getSETUP();
	std::string getPLAY();
	std::string getPAUSE();
	std::string getGET_PARAMETER();
	std::string getTEARDOWN();
   
    std::string getResponse();
    std::string getCreationTimeString();
    std::string getSDPDescription(double maxRange);

    void printLog();
};
