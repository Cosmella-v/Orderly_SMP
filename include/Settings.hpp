#include <filesystem>


#include <thread>
#include <atomic>
#include <algorithm>

#include <iostream>
#include "json.hpp" 

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

#include <streambuf>
#include <iostream>
#include <mutex>
#include <vector>
#include <string>

namespace fs = std::filesystem;

struct Settings {
    int memoryMB = 4096;

    static constexpr const char* settingsFile = "OrderlySMPInstallerSettings.json";

    void save() const {
        nlohmann::json j{
            {"Memory", memoryMB},
        };
        std::ofstream(settingsFile) << j.dump(4);
    }

    void load() {
        std::ifstream f(settingsFile);
        if (!f) return;
        try {
            nlohmann::json j;
            f >> j;
            memoryMB = j.value("Memory", memoryMB);
        } catch (...) {
            std::cerr << "[!] Failed to load settings.\n";
        }
    }
};
static Settings settings;

static Settings settingsLauncherProcessing;