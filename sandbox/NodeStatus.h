#pragma once

#include <string>
#include <map>

class NodeStatus {
    long long total, idle;
    std::map<std::string,long long> mem;

public:
    NodeStatus();
    ~NodeStatus();

    void updateCPUStatus();
    double getCPUIdleRatio();
    double getCPUUsageRatio();

    void updateMemStatus();
    long long getTotalMem();
    long long getAvailableMem();
    double getAvailableMemRatio();
    double getUsageMemRatio();
    long long getInfoMem(const char* key);
    void printLog();
};