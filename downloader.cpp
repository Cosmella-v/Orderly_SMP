#include <cpr/cpr.h>
#include <fstream>
#include <iostream>
#include <string>
#include "downloader.hpp"

bool downloadFile(const std::string& url, const std::string& outputPath) {
    auto response = cpr::Get(cpr::Url{url});
    if (response.error) {
        std::cerr << "[X] Request error: " << response.error.message << std::endl;
        return false;
    }

    if (response.status_code != 200) {
        std::cerr << "[X] HTTP error: Status code " << response.status_code << std::endl;
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "[X] Failed to open output file: " << outputPath << std::endl;
        return false;
    }

    outFile.write(response.text.data(), response.text.size());
    outFile.close();

    std::cout << "[*] Download succeeded: " << outputPath << std::endl;
    return true;
}
