#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

double getCPU();
double getCPUProcess();
double getTotalPhysicalMemory();
double getUsedPhysicalMemory();
double getProcessPhysicalMemory();
double getTotalVirtualMemory();
double getUsedVirtualMemory();
double getProcessVirtualMemory();

#ifdef _WIN32

#include "windows.h"
#include "TCHAR.h"
#include "pdh.h"
#include "psapi.h"


static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

void init(){
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}


MEMORYSTATUSEX memInfo;
memInfo.dwLength = sizeof(MEMORYSTATUSEX);
GlobalMemoryStatusEx(&memInfo);

double getTotalVirtualMemory() {
    DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
    return (double)totalVirtualMem;
}

double getUsedVirtualMemory() {
    DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
    return (double)virtualMemUsed;
}

double getProcessVirtualMemory() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    return (double)virtualMemUsedByMe;
}

double getTotalPhysicalMemory() {
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    return (double)totalPhysMem;
}

double getUsedPhysicalMemory() {
    DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    return (double)physMemUsed;
}

double getProcessVirtualMemory() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
    return (double)physMemUsedByMe;
}


double getCPU(){
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
}

double getCPUProcess() {
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    double percent;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));
    percent = (sys.QuadPart - lastSysCPU.QuadPart) +
        (user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;
    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;

    return percent * 100;
}


#else

#include "sys/types.h"
#include "sys/sysinfo.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "sys/times.h"

struct sysinfo memInfo;
static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

static clock_t lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;

int parseLine(char* line){
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

void init() {
    FILE* file1 = fopen("/proc/stat", "r");
    if (!file1) return;

    if (fscanf(file1, "cpu %llu %llu %llu %llu",
               &lastTotalUser,
               &lastTotalUserLow,
               &lastTotalSys,
               &lastTotalIdle) != 4) {
        fclose(file1);
        return;
    }

    fclose(file1);

    FILE* file2;
    struct tms timeSample;
    char line[128];

    lastCPU = times(&timeSample);
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;

    file2 = fopen("/proc/cpuinfo", "r");
    numProcessors = 0;
    while(fgets(line, 128, file2) != NULL){
        if (strncmp(line, "processor", 9) == 0) numProcessors++;
    }
    fclose(file2);
}

double getTotalVirtualMemory() {
    sysinfo (&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

    return (double)totalVirtualMem;
}

double getUsedVirtualMemory() {
    sysinfo (&memInfo);
    long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;

    return (double)virtualMemUsed;
}

double getProcessVirtualMemory(){
    FILE* file = fopen("/proc/self/status", "r");
    int result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return (double)result;  
}

double getTotalPhysicalMemory() {
    sysinfo (&memInfo);
    long long totalPhysMem = memInfo.totalram;
    totalPhysMem *= memInfo.mem_unit;

    return (double)totalPhysMem;
}

double getUsedPhysicalMemory() {
    sysinfo (&memInfo);
    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    physMemUsed *= memInfo.mem_unit;

    return (double)physMemUsed;
}

double getProcessPhysicalMemory(){
    FILE* file = fopen("/proc/self/status", "r");
    double result = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            result = parseLine(line);
            break;
        }
    }
    fclose(file);
    return (double)result;  
}

double getCPU() {
    FILE* file = fopen("/proc/stat", "r");
    if (!file) return -1.0;

    unsigned long long totalUser, totalUserLow, totalSys, totalIdle;

    if (fscanf(file, "cpu %llu %llu %llu %llu",
               &totalUser,
               &totalUserLow,
               &totalSys,
               &totalIdle) != 4) {
        fclose(file);
        return -1.0;
    }

    fclose(file);

    double percent;
    unsigned long long total;

    if (totalUser < lastTotalUser ||
        totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys ||
        totalIdle < lastTotalIdle) {
        percent = -1.0;
    } else {
        total = (totalUser - lastTotalUser)
              + (totalUserLow - lastTotalUserLow)
              + (totalSys - lastTotalSys);

        percent = (double)total;
        total += (totalIdle - lastTotalIdle);
        percent = (percent / total) * 100.0;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return (double)percent;
}
int i = 1;
double getCPUProcess() {
    // std::cout << "Start getCPUProcess" << std::endl; // Comment
    // struct tms timeSample;
    // std::cout << "timeSample stime:" << timeSample.tms_stime << std::endl; // Comment
    // std::cout << "timeSample utime:" << timeSample.tms_utime << std::endl; // Comment
    // clock_t now = times(&timeSample);
    // std::cout << "now:" << now << std::endl; // Comment
   

    // if (now == (clock_t)-1) {
    //     return -1.0;
    //     std::cout << "Return -1.0 (1):" << std::endl; // Comment
    // }

    // if (lastCPU == 0) {
    //     lastCPU = now;
    //     lastSysCPU = timeSample.tms_stime;
    //     lastUserCPU = timeSample.tms_utime;
    //     return 0.0;
    //     std::cout << "Return 0.0 (1):" << std::endl; // Comment
    // }
    // std::cout << "lastCPU: " << lastCPU << std::endl; // Comment
    
   

    // double percent =
    //     (timeSample.tms_stime - lastSysCPU) +
    //     (timeSample.tms_utime - lastUserCPU);
    // std::cout << "Percent: (1)" << percent << std::endl; // Comment
    // percent /= (now - lastCPU);
    // std::cout << "Percent: (2)" << percent << std::endl; // Comment
    // percent /= numProcessors;
    // std::cout << "Percent: (3)" << percent << std::endl; // Comment
    // percent *= 100.0;
    // std::cout << "Percent: (4)" << percent << std::endl; // Comment

    // lastCPU = now;
    // lastSysCPU = timeSample.tms_stime;
    // lastUserCPU = timeSample.tms_utime;
    // std::cout << "Percent: (5)" << percent << std::endl; // Comment
    // return percent;

    std::cout << "----------------------------" << std::endl; // Comment
    std::cout << "Update: " << i << std::endl; // Comment
    i++;
    std::cout << "----------------------------" << std::endl; // Comment
    std::cout << "Start getCPUProcess" << std::endl; // Comment
    struct tms timeSample;
    std::cout << "timeSample stime: " << timeSample.tms_stime << std::endl; // Comment
    std::cout << "timeSample utime: " << timeSample.tms_utime << std::endl; // Comment
    clock_t now;
    std::cout << "Now: " << now << std::endl; // Comment
    double percent;


    now = times(&timeSample);
    if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
        timeSample.tms_utime < lastUserCPU){
        //Overflow detection. Just skip this value.
        percent = -1.0;
        std::cout << "Return -1.0 (1):" << std::endl; // Comment
    }
    else{
        percent = (timeSample.tms_stime - lastSysCPU) +
            (timeSample.tms_utime - lastUserCPU);
            std::cout << "Percent: (1): " << percent << std::endl; // Comment
        percent /= (now - lastCPU);
        std::cout << "Percent (2): " << percent << std::endl; // Comment
        percent /= numProcessors;
        std::cout << "Percent (3): " << percent << std::endl; // Comment
        percent *= 100;
        std::cout << "Percent (4): " << percent << std::endl; // Comment
    }
    lastCPU = now;
    lastSysCPU = timeSample.tms_stime;
    lastUserCPU = timeSample.tms_utime;
    std::cout << "Percent (5): " << percent << std::endl; // Comment
    return percent;
}


#endif


// Server setup
int main() {
    httplib::Server server;

    server.Get("/metrics/stream", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Content-Type", "text/event-stream");
        res.set_header("Cache-Control", "no-cache");
        res.set_header("Connection", "keep-alive");
        res.set_header("Access-Control-Allow-Origin", "http://localhost");

        res.set_chunked_content_provider(
            "text/event-stream",
            [](size_t, httplib::DataSink& sink) {
                json data = {
                    {"status", "connected"},
                    {"cpu", getCPU()},
                    {"cpu_process", getCPUProcess()},
                    {"total_ram", getTotalPhysicalMemory()},
                    {"used_ram", getUsedPhysicalMemory()},
                    {"process_ram", getProcessPhysicalMemory()},
                    {"total_virtual_ram", getTotalVirtualMemory()},
                    {"used_virtual_ram", getUsedVirtualMemory()},
                    {"process_virtual_ram", getProcessVirtualMemory()}
                };

                std::string msg = "data: " + data.dump() + "\n\n";
                sink.write(msg.data(), msg.size());

                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                return true;
            }
        );
    });


    server.listen("0.0.0.0", 80);
}
