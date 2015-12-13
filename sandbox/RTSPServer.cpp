#include "RTSPServer.h"
#include "RTSPParser.h"
#include "RTSPRequest.h"
#include "RTSPResponse.h"
#include "MPEG2TS.h"
#include "RTSPSession.h"

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

RTSPServer::RTSPServer(int _port, int _backlog, int _nextRTPport)
    : port(_port), backlog(_backlog), listenfd(-1), nextRTPport(_nextRTPport) {}

RTSPServer::~RTSPServer() {
    close(listenfd);
}

void RTSPServer::setNextRTPport(int _nextRTPport) {
    nextRTPport = _nextRTPport;
}

int RTSPServer::getNextRTPport() {
    return nextRTPport;
}

bool RTSPServer::startListen() {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("ERROR opening socket");
        return false;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(listenfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on bind");
        close(listenfd);
        return false;
    }

    if (listen(listenfd, backlog)) {
        perror("ERROR on listen");
        close(listenfd);
        return false;
    }

    return true;
}

void RTSPServer::acceptLoop() {
    while (true) {
        RTSPParser *rtspParser = new RTSPParser(this);
        if (!rtspParser->acceptClient(listenfd))
            return;

        pthread_t thread;
        if (pthread_create(&thread, NULL, parseLoop, rtspParser)) {
            perror("ERROR on pthread_create");
            delete rtspParser;
            return;
        }
    }
}

void* RTSPServer::parseLoop(void *arg) {
    RTSPParser *rtspParser = (RTSPParser*)arg;

    if (pthread_detach(pthread_self())) {
        perror("ERROR on pthread_detach");
        delete rtspParser;
        return NULL;
    }

    RTSPServer *rtspServer = rtspParser->getServer();

    while(true) {
        RTSPRequest *rtspRequest = rtspParser->parse();
        RTSPResponse rtspResponse(rtspRequest);

        std::string method = rtspRequest->getMethod();
        if (method == "OPTIONS") {
            std::string res = rtspResponse.getOPTIONS();
            rtspParser->write(res.c_str(), res.length());
        } else if (method == "DESCRIBE") {
            MPEG2TS v;
            if (v.open(rtspRequest->getFilepath().c_str())) {
                v.parsetsx();
                std::string res = rtspResponse.getDESCRIBE(v.getDuration());
                rtspParser->write(res.c_str(), res.length());
            }
        } else if (method == "SETUP") {
            RTSPSession s;
            if (s.setup(rtspServer->getNextRTPport())) {
                s.generateKey();
                rtspServer->setNextRTPport(s.getPort() + 2);

                std::string res = rtspResponse.getSETUP(
                    rtspParser->getClientIP(),
                    s.getPort(),
                    s.getPort() + 1,
                    s.getKey()
                );
                rtspParser->write(res.c_str(), res.length());
            }
        }

        delete rtspRequest;
    }

    delete rtspParser;

    return NULL;
}
