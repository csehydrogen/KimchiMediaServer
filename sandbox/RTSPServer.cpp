#include "RTSPServer.h"
#include "RTSPParser.h"
#include "RTSPRequest.h"
#include "RTSPResponse.h"
#include "MPEG2TS.h"

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

void RTSPServer::addSession(char const *key, RTSPSession *s) {
    sessions[key] = s;
}

RTSPSession* RTSPServer::getSession(std::string key) {
    return sessions[key];
}

std::map<std::string, RTSPSession*>& RTSPServer::getSessions() {
    return sessions;
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
    pthread_t thread;
    if (pthread_create(&thread, NULL, sendLoop, this)) {
        perror("ERROR on pthread_create sendLoop");
        return;
    }
    while (true) {
        RTSPParser *rtspParser = new RTSPParser(this);
        if (!rtspParser->acceptClient(listenfd))
            return;

        if (pthread_create(&thread, NULL, parseLoop, rtspParser)) {
            perror("ERROR on pthread_create parseLoop");
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
        if (rtspRequest == NULL)
            break;
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
            RTSPSession *s = new RTSPSession();
            if (s->setup(rtspServer->getNextRTPport())) {
                rtspServer->setNextRTPport(s->getPort() + 2);
                s->generateKey();
                s->setTS(rtspRequest->getFilepath().c_str());
                s->setClientAddr(rtspParser->getClientIP(), rtspRequest->getClientRTPPort());
                rtspServer->addSession(s->getKey(), s);

                std::string res = rtspResponse.getSETUP(
                    rtspParser->getClientIP(),
                    s->getPort(),
                    s->getPort() + 1,
                    s->getKey()
                );
                rtspParser->write(res.c_str(), res.length());
            }
        } else if (method == "PLAY") {
            RTSPSession *s = rtspServer->getSession(rtspRequest->getHeader("Session"));
            double startNptTime = rtspRequest->getStartNptTime();
            s->seek(startNptTime);

            std::string res = rtspResponse.getPLAY(s->getNpt(), s->getSeqnum(), s->getTimestamp());
            rtspParser->write(res.c_str(), res.length());

            s->setPlay();
        } else if (method == "PAUSE") {
            RTSPSession *s = rtspServer->getSession(rtspRequest->getHeader("Session"));

            std::string res = rtspResponse.getPAUSE();
            rtspParser->write(res.c_str(), res.length());

            s->setPause();
        } else if (method == "TEARDOWN") {
            std::string res = rtspResponse.getTEARDOWN();
            rtspParser->write(res.c_str(), res.length());
        }

        delete rtspRequest;
    }

    delete rtspParser;

    return NULL;
}

void* RTSPServer::sendLoop(void *arg) {
    RTSPServer *server = (RTSPServer*)arg;
    auto &sessions = server->getSessions();
    while (true) {
        usleep(1000);
        for (auto it = sessions.begin(); it != sessions.end(); ++it) {
            it->second->play();
        }
    }
}
