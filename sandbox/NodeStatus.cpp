#include "NodeStatus.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

NodeStatus::NodeStatus() {
    updateCPUStatus();
    updateMemStatus();
}

NodeStatus::~NodeStatus() {
  
}

void NodeStatus::updateCPUStatus() {
    FILE *fp = fopen("/proc/stat", "r");
    if(fp == NULL) {
        perror("/proc/stat NOT Found");
        idle=1, total=2;
        return;
    }

    char buf[512];
    fgets(buf,512,fp);
  
    long long _total=0, _idle, count=0;
    char *ptr, *saveptr;
    ptr = strtok_r(buf, " \n", &saveptr);
    ptr = strtok_r(NULL, " \n", &saveptr);

    while(ptr != NULL) {
        count++;
        long long val = atol(ptr);
        _total += val;
        if(count == 4)
            _idle = val;

        ptr = strtok_r(NULL, " \n", &saveptr);
    }

    total = _total;
    idle = _idle;
    fclose(fp);
}

double NodeStatus::getCPUIdleRatio() {
    return (double)idle/total;
}

double NodeStatus::getCPUUsageRatio() {
    return 1.0-(double)idle/total;
}
    
void NodeStatus::updateMemStatus() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if(fp == NULL) {
        perror("/proc/meminfo NOT Found");
        mem["MemAvailable"] = 1, mem["MemTotal"] = 2;
        return;
    }
  
    char buf[512];
    char *ptr, *saveptr;

    while(fgets(buf,512,fp) != NULL) {
        ptr = strtok_r(buf, " :\n", &saveptr);
        std::string key(ptr);
        ptr = strtok_r(NULL, " :\n", &saveptr);
        long long value = atol(ptr);
    
        mem[key] = value;
    }

    fclose(fp);
}

long long NodeStatus::getTotalMem() {
    return mem["MemTotal"];
}

long long NodeStatus::getAvailableMem() {
    return mem["MemAvailable"];
}

double NodeStatus::getAvailableMemRatio() {
    return (double)mem["MemAvailable"]/mem["MemTotal"];
}

double NodeStatus::getUsageMemRatio() {
    return 1.0-(double)mem["MemAvailable"]/mem["MemTotal"];
}

long long NodeStatus::getInfoMem(const char *buf) {
    return mem[std::string(buf)];
}

void NodeStatus::printLog() {
    printf("CPU Idle Ratio : %lf\n", getCPUIdleRatio());
    printf("\nMemTotal : %lld\n"
      "MemAvailable : %lld\n"
      "AvailableRatio : %lf\n", 
      getTotalMem(), getAvailableMem(), getAvailableMemRatio());
}