#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

double getTotalVirtualMemory();
double getUsedVirtualMemory();
double getTotalPhysicalMemory();
double getUsedPhysicalMemory();
double getCurrentCPU();

#ifdef _WIN32

#include "windows.h"
#include "TCHAR.h"
#include "pdh.h"


static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void init(){
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
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

double getTotalPhysicalMemory() {
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    return (double)totalPhysMem;
}

double getUsedPhysicalMemory() {
    DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    return (double)physMemUsed;
}


double getCurrentCPU(){
    PDH_FMT_COUNTERVALUE counterVal;

    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    /return counterVal.doubleValue;
}


#else

#include "sys/types.h"
#include "sys/sysinfo.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

struct sysinfo memInfo;
static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;


void init() {
    FILE* file = fopen("/proc/stat", "r");
    if (!file) return;

    if (fscanf(file, "cpu %llu %llu %llu %llu",
               &lastTotalUser,
               &lastTotalUserLow,
               &lastTotalSys,
               &lastTotalIdle) != 4) {
        fclose(file);
        return;
    }

    fclose(file);
}

double getTotalVirtualMemory() {
    sysinfo (&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    //Add other values in next statement to avoid int overflow on right hand side...
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

    return (double)totalVirtualMem;
}

double getUsedVirtualMemory() {
    sysinfo (&memInfo);
    long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
    //Add other values in next statement to avoid int overflow on right hand side...
    virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
    virtualMemUsed *= memInfo.mem_unit;

    return (double)virtualMemUsed;
}

double getTotalPhysicalMemory() {
    sysinfo (&memInfo);
    long long totalPhysMem = memInfo.totalram;
    //Multiply in next statement to avoid int overflow on right hand side...
    totalPhysMem *= memInfo.mem_unit;

    return (double)totalPhysMem;
}

double getUsedPhysicalMemory() {
    sysinfo (&memInfo);
    long long physMemUsed = memInfo.totalram - memInfo.freeram;
    //Multiply in next statement to avoid int overflow on right hand side...
    physMemUsed *= memInfo.mem_unit;

    return (double)physMemUsed;
}

double getCurrentCPU() {
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

    return percent;
}

#endif

// double i1 = 0.0;
// double i2 = 0.0;

// double cpu_usage() {
//     if (i1 > 100.0) {
//         i1 = 0.0;
//     }
//     i1 += 0.1;
//     return i1;
// }

// double ram_usage() {
//     if (i2 > 100.0) {
//         i2 = 0.0;
//     }
//     i2 += 0.1;
//     return i2;
// }



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
                {"cpu", getCurrentCPU()},
                {"total_ram", getTotalPhysicalMemory()},
                {"used_ram", getUsedPhysicalMemory()},
                {"total_virtual_ram", getTotalVirtualMemory()},
                {"used_virtual_ram", getUsedVirtualMemory()}
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
