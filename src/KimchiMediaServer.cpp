#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>

int main(int argc, char **argv) {
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);

    RTSPServer *rtspServer = RTSPServer::createNew(*env, 8554);
    if(rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    ServerMediaSession *sms = ServerMediaSession::createNew(
        *env,
        "surfing.ts", // streamName
        "surfing.ts", // info
        "Session streamed by \"KimchiMediaServer\"" // description
    );
    sms->addSubsession(
        MPEG2TransportFileServerMediaSubsession::createNew(
            *env,
            "surfing.ts",
            "surfing.tsx",
            false // reuseFirstSource
        )
    );
    rtspServer->addServerMediaSession(sms);

    char *url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;

    env->taskScheduler().doEventLoop();

    return 0;
}
