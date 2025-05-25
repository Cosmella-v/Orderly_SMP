#include <iostream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "downloader.hpp"
#include "unzipper.hpp"
#include "parser.hpp"
#include "json.hpp"

namespace fs = std::filesystem;
std::string path = "";

std::string getDefaultMinecraftPath() {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        std::string path = std::string(appdata) + "\\.minecraft";
        if (fs::exists(path)) return path;
    }
    const char* localappdata = std::getenv("LOCALAPPDATA");
    if (localappdata) {
        std::string path = std::string(localappdata) + "\\.minecraft";
        if (fs::exists(path)) return path;
    }
    return "";
#elif __APPLE__
    const char* home = std::getenv("HOME");
    if (home) {
        std::string path = std::string(home) + "/Library/Application Support/minecraft";
        if (fs::exists(path)) return path;
    }
    return "";
#else
    const char* home = std::getenv("HOME");
    if (home) {
        std::string path = std::string(home) + "/.minecraft";
        if (fs::exists(path)) return path;
    }
    return "";
#endif
}

std::string promptUserForMinecraftPath() {
    std::string inputPath;
    while (true) {
        std::cout << "[?] Could not find Minecraft folder automatically.\n";
        std::cout << "[?] Please enter your Minecraft folder path: ";
        std::getline(std::cin, inputPath);
        if (fs::exists(inputPath)) {
            return inputPath;
        } else {
            std::cout << "[!] The path you entered does not exist. Please try again.\n";
        }
    }
}

std::string getMinecraftFolder() {
    if (!path.empty()) return path;

    path = getDefaultMinecraftPath();
    if (path.empty()) {
        path = promptUserForMinecraftPath();
    }

    return path;
}


void writejsonSomethin() {
    std::cout << "Closing Launcher";
    fs::path launcherProfilesPath = getMinecraftFolder();
    launcherProfilesPath /= "launcher_profiles.json";

    std::ifstream inFile(launcherProfilesPath);
    nlohmann::json j;

    if (inFile.is_open()) {
        inFile >> j;
        inFile.close();
    } else {
        j = nlohmann::json::object();
    }

    nlohmann::json customProfile = {
        {"icon", "data:image/webp;base64,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"},
        {"lastVersionId", "1.20.1-OrderlySMP"},
        {"gameDir", (fs::path(getMinecraftFolder()) / "installations" / "OrderlySMP").string()},
        {"name", "Orderly SMP"},
        {"javaArgs", "-Xmx6G -XX:+UnlockExperimentalVMOptions -XX:+UseG1GC -XX:G1NewSizePercent=20 -XX:G1ReservePercent=20 -XX:MaxGCPauseMillis=50 -XX:G1HeapRegionSize=32M"},
        {"type", "custom"}
    };

    if (!j.contains("profiles") || !j["profiles"].is_object()) {
        j["profiles"] = nlohmann::json::object();
    }

    j["profiles"]["Orderly_SMP"] = customProfile;

    std::ofstream outFile(launcherProfilesPath);
    outFile << j.dump(4);
    outFile.close();

    std::cout << "[✓] Custom profile added/updated successfully!" << std::endl;
}
static int Process() {
    std::cout << "[*] Downloading Base Profile...\n";
    const std::string filen = "profile.zip";
    if (!downloadFile("https://github.com/Cosmella-v/Orderly_SMP/blob/main/BaseID.zip?raw=true", filen)) {
        std::cerr << "[!] Download failed.\n";
        return 1;
    }
    
    std::cout << "[*] Extracting Base Profile...";
    const std::string filen2 = (fs::path(getMinecraftFolder()) / "versions").string();
    if (!unzip_file(filen, filen2)) {
        std::cerr << "[!] Unzip failed.\n";
        return 1;
    }

    fs::path dst = fs::path(getMinecraftFolder()) / "installations" / "OrderlySMP";
    if (!fs::exists(dst)) {
        fs::create_directories(dst);
    }

    for (const auto& entry : fs::directory_iterator("OrderlySMP")) {
        fs::path destPath = dst / entry.path().filename();
        if (fs::exists(destPath)) {
            std::error_code ec;
            fs::remove_all(destPath, ec);
            if (ec) {
                std::cerr << "[!] Failed to delete " << destPath << ": " << ec.message() << "\n";
                return 1;
            }
        }
        std::error_code ec;
        if (fs::is_directory(entry.status())) {
            fs::copy(entry.path(), destPath, fs::copy_options::recursive, ec);
        } else {
            fs::copy_file(entry.path(), destPath, fs::copy_options::overwrite_existing, ec);
        }

        if (ec) {
            std::cerr << "[!] Failed to copy " << entry.path() << " to " << destPath << ": " << ec.message() << "\n";
            return 1;
        }
    }
    std::cout << "[✓] Copy completed successfully. Lastly modifing the json\n";
    writejsonSomethin();

    if (fs::is_directory("OrderlySMP")) {
        fs::remove_all("OrderlySMP");
    }
    if (fs::is_regular_file(filen)) {
        fs::remove_all(filen);
    }

    return 1;
}


