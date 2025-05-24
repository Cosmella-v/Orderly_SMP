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
void writejsonSomethin(){
    std::ifstream inFile(getMinecraftFolder()+"/launcher_profiles.json");
    nlohmann::json j;

    if (inFile.is_open()) {
        inFile >> j;
        inFile.close();
    } else {
        j = nlohmann::json::object();
    }

    nlohmann::json customProfile = {
        {"icon", "dirt"},
        {"lastVersionId", "1.20.1-OrderlySMP"},
        {"name", "Orderly SMP : 1.20.1"},
        {"type", "custom"}
    };

    if (!j.contains("profiles") || !j["profiles"].is_object()) {
        j["profiles"] = nlohmann::json::object();
    }

    j["profiles"]["Orderly_SMP"] = customProfile;

    std::ofstream outFile(getMinecraftFolder()+"/launcher_profiles.json");
    outFile << j.dump(4);
    outFile.close();

    std::cout << "[✓] Custom profile added/updated successfully!" << std::endl;
    return;
}
static int Process() {
    std::cout << "[*] Downloading Base Profile...\n";
    if (!downloadFile("https://github.com/Cosmella-v/Orderly_SMP/blob/main/BaseID?raw=true", "Profile.zip")) {
        std::cerr << "[!] Download failed.\n";
        return 1;
    }
    
    std::cout << "[*] Extracting Base Profile...\nYou may be prompted to input your mc dir";
    if (!unzip_file("Profile.zip", getMinecraftFolder()+"/versions")) {
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
}


std::string getMinecraftFolder() {
    if (!path.empty()) return path;

    path = getDefaultMinecraftPath();
    if (path.empty()) {
        path = promptUserForMinecraftPath();
    }

    return path;
}
