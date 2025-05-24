#include "downloader.hpp"
#include "unzipper.hpp"
#include "parser.hpp"
#include "CreateProfile.hpp"
#include <filesystem>
#include <iostream>

int main() {
    const std::string url = "https://github.com/Cosmella-v/Orderly_SMP/blob/main/ModScriptPack.mrpack?raw=true";
    const std::string mrpackFile = "OrderlySMP.zip";
    const std::string extractDir = "temp_extract";
    const std::string profileDir = "OrderlySMP";

    std::cout << "[*] Downloading mrpack...\n";
    if (!downloadFile(url, mrpackFile)) {
        std::cerr << "[!] Download failed.\n";
        return 1;
    }

    std::cout << "[*] Extracting mrpack...\n";
    if (!unzip_file(mrpackFile, extractDir)) {
        std::cerr << "[!] Unzip failed.\n";
        return 1;
    }

    std::cout << "[*] Parsing modrinth.index.json...\n";
    auto mods = parseModrinthIndex(extractDir + "/modrinth.index.json");

    std::cout << "[*] Downloading mods...\n";
    for (const auto& mod : mods) {
        std::string dest = profileDir + "/" + mod.path;
        std::filesystem::create_directories(std::filesystem::path(dest).parent_path());
        if (!downloadFile(mod.url, dest)) {
            std::cerr << "[!] Failed to download: " << mod.url << "\n";
        }
    }

    std::cout << "[*] Copying overrides...\n";
    copyOverrides(extractDir + "/overrides", profileDir);

    std::cout << "[✓] Created Profile At "<< profileDir << "\nImporting this into the mc launcher" << "\n";
    Process()
    return 0;
}
