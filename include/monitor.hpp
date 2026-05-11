#pragma once
#include <string>
#include <vector>


struct ProcessInfo {
    int pid;
    std::string name;
    double mem;
    char status;
    int threads;
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

    void updateProcesses();
public:
    void update();
    double getCpuUsage() const {return cpuUsage_;}
    const Memory& getMemory() const {return mem_;}
    const std::vector<ProcessInfo>& getProcesses() const { return processes_; }
};