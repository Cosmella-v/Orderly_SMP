#include "downloader.hpp"
#include "unzipper.hpp"
#include "parser.hpp"
#include "CreateProfile.cpp"
#include <filesystem>
#include <iostream>
#include "json.hpp" 
#include <cpr/cpr.h>
std::string fetch_url(const std::string& url) {
    auto response = cpr::Get(cpr::Url{url}, cpr::Header{{"User-Agent", "Mozilla/5.0"}}); 

    if (response.error) {
        std::cerr << "[X] Request error: " << response.error.message << std::endl;
        return {};
    }
    if (response.status_code != 200) {
        std::cerr << "[X] HTTP error: " << response.status_code << std::endl;
        return {};
    }
    return response.text;
}
std::string getLatest() {
     std::string api_url = "https://api.github.com/repos/JuJubeanYT/Orderly_SMP/releases/latest";
    std::string json_data = fetch_url(api_url);
    if (json_data.empty()) {
        std::cerr << "Failed to fetch release info." << std::endl;
        return "https://github.com/JuJubeanYT/Orderly_SMP/releases/download/v1.1.1/Orderly.SMP-v1.1.1-Modrinth.mrpack";
    }

    try {
        auto json = nlohmann::json::parse(json_data);
        std::string tag_name = json["tag_name"];

        for (const auto& asset : json["assets"]) {
            std::string name = asset["name"];
            if (name.size() >= 7 && name.compare(name.size() - 7, 7, ".mrpack") == 0) {
                std::string download_url = asset["browser_download_url"];
                std::cout << "Release tag: " << tag_name << std::endl;
                std::cout << "Download URL: " << download_url << std::endl;
                return download_url;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[X] JSON parse error: " << e.what() << std::endl;
    }
    return "https://github.com/JuJubeanYT/Orderly_SMP/releases/download/v1.1.1/Orderly.SMP-v1.1.1-Modrinth.mrpack";
};

int main() {
    std::cout << "Getting latest build!\n";
    const std::string url = getLatest();
    const std::string mrpackFile = "OrderlySMP.zip";
    const std::string extractDir = "temp_extract";
    const std::string profileDir = "OrderlySMP";
    std::cout << "[!!!!!!] CLOSE YOUR MINECRAFT LAUNCHER [!!!!!!]\n";
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
    if (fs::is_directory(extractDir)) {
        fs::remove_all(extractDir);
    }
    if (fs::is_regular_file(mrpackFile)) {
        fs::remove_all(mrpackFile);
    }
    Process();
    return 1;
}
