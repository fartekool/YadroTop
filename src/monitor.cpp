#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <filesystem>
#include <algorithm>
#include <unistd.h>

#include "monitor.hpp"



void Memory::updateMemoryData() {

    std::ifstream meminfo("/proc/meminfo");
    std::string line;

    while (std::getline(meminfo, line)) {

        std::istringstream iss(line);
        std::string key;
        int64_t value;

        if (!(iss >> key >> value)) {
            continue;
        }
        if (key == "MemTotal:") {
            total_ = static_cast<double>(value) / 1024; 
        } else if (key == "MemFree:") {
            free_ = static_cast<double>(value) / 1024; 
        } else if (key == "MemAvailable:") {
            available_ = static_cast<double>(value) / 1024;
        }
    }
}

void Cpu::updateCpuData() {
    std::ifstream stat("/proc/stat");
    std::string line;
    std::getline(stat, line);

    std::istringstream iss(line);

    std::string cpuLabel;
    iss >> cpuLabel;

    std::vector<int64_t> vectorJiffies;
    int64_t jiffie;
    while (iss >> jiffie) {
        vectorJiffies.push_back(jiffie);
    }
    int64_t idle = vectorJiffies[3];
    int64_t total = std::accumulate(vectorJiffies.begin(), vectorJiffies.end(), static_cast<int64_t>(0));

    idle_ = idle;
    total_ = total;
}


void Monitor::update() {
    mem_.updateMemoryData();

    Cpu current;
    current.updateCpuData();

    int64_t diffIdle = current.idle_ - prevCpu_.idle_;
    int64_t diffTotal = current.total_ - prevCpu_.total_;

    if (diffTotal == 0) {
        cpuUsage_ = 0.0;
    } else {
        cpuUsage_ = (1 - static_cast<double>(diffIdle) / static_cast<double>(diffTotal)) * 100.0;
    }
    prevCpu_ = current;

    updateProcesses(diffTotal);
}

void Monitor::updateProcesses(int64_t totalDelta) {

    //processes_.clear();
    std::vector<ProcessInfo> nextProcesses;
    std::map<int, int64_t> currProcessesTicks;

    static int64_t pageSize = sysconf(_SC_PAGESIZE);
    static int numCores = sysconf(_SC_NPROCESSORS_ONLN);
    for (const auto& entry : std::filesystem::directory_iterator("/proc")) {
        if (!entry.is_directory()) continue;

        std::string dirname = entry.path().filename().string();
        
        if (!std::all_of(dirname.begin(), dirname.end(), ::isdigit)) continue;

        int pid = std::stoi(dirname);
        
        std::string commPath = "/proc/" + dirname + "/comm";
        std::ifstream commFile(commPath);
        std::string name;
        if (commFile.is_open()) {
            std::getline(commFile, name);
        } else {
            continue;
        }

        std::string statmPath = "/proc/" + dirname + "/statm";
        std::ifstream statmFile(statmPath);
        double memMb = 0.0;
        if (statmFile.is_open()) {
            int64_t rssPages;
            std::string dummy; 
            // В statm: VSIZE RSS SHARED TEXT LIB DATA DIRTY
            if (statmFile >> dummy >> rssPages) {
                memMb = (rssPages * pageSize) / (1024.0 * 1024.0);
            }
        } else {
            continue;
        }

        std::string statPath = "/proc/" + dirname + "/stat";
        std::ifstream statFile(statPath);
        char status = '?';
        int threads = 1;
        double processCpuUsage = 0.0;
        if (statFile.is_open()) {
            std::string line;
            if (std::getline(statFile, line)) {
                size_t lastBracket = line.find_last_of(')');
                if (lastBracket != std::string::npos && lastBracket + 2 < line.size()) {
                    status = line[lastBracket + 2];
                    std::stringstream ss(line.substr(lastBracket + 4));
                    std::string dummy;
                    
                    int64_t uTime = 0;
                    int64_t sTime = 0;
                    for (int i = 0; i < 10; ++i) {
                        if (!(ss >> dummy)) break; 
                    }

                    if (ss >> uTime >> sTime) {
                        int64_t totalTicks = uTime + sTime;
                        currProcessesTicks[pid] = totalTicks;
                        if (totalDelta > 0 && prevProcessesTicks_.count(pid)) {
                            int64_t deltaTicks = totalTicks - prevProcessesTicks_[pid];
                            processCpuUsage = (static_cast<double>(deltaTicks) / static_cast<double>(totalDelta)) * 100.0 * numCores;
                        }
                    }

                    for (int i = 0; i < 4; ++i) {
                        if (!(ss >> dummy)) break;
                    }

                    ss >> threads;
                }
            }
        } else {
            continue;
        }
        nextProcesses.push_back({pid, name, memMb, status, threads, processCpuUsage});
    }
    prevProcessesTicks_ = std::move(currProcessesTicks);
    std::sort(nextProcesses.begin(), nextProcesses.end(), [](const ProcessInfo& a, const ProcessInfo& b){return a.cpuUsage > b.cpuUsage;});
    processes_ = std::move(nextProcesses);
}