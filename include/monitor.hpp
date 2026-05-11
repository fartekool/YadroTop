#pragma once
#include <pwd.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>

struct ProcessInfo {
    int pid;
    std::string name;
    double mem;
    char status;
    int threads;
    double cpuUsage;
    std::string user;
};

struct Memory {
    double total_ = 0.0;
    double free_ = 0.0;
    double available_ = 0.0;

    void updateMemoryData();
};

struct Cpu {
    int64_t idle_ = 0;
    int64_t total_ = 0;

    void updateCpuData();
};

class Monitor {
    Memory mem_;
    Cpu prevCpu_;
    double cpuUsage_ = 0.0;
    std::vector<ProcessInfo> processes_;
    std::map<int, int64_t> prevProcessesTicks_;
    std::map<uid_t, std::string> userCache_;

    void updateProcesses(int64_t totalDelta);
public:
    void update();
    double getCpuUsage() const {return cpuUsage_;}
    const Memory& getMemory() const {return mem_;}
    const std::vector<ProcessInfo>& getProcesses() const { return processes_; }
};