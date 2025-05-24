#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
// before i added json.hpp lol
std::vector<ModFile> parseModrinthIndex(const std::string& jsonFile) {
    std::ifstream in(jsonFile);
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string content = buffer.str();

    std::vector<ModFile> mods;

    size_t pos = 0;
    while ((pos = content.find("\"path\":", pos)) != std::string::npos) {
        size_t start = content.find("\"", pos + 7) + 1;
        size_t end = content.find("\"", start);
        std::string path = content.substr(start, end - start);
        pos = end;

        size_t urlStart = content.find("\"downloads\":", pos);
        urlStart = content.find("[", urlStart);
        urlStart = content.find("\"", urlStart) + 1;
        size_t urlEnd = content.find("\"", urlStart);
        std::string url = content.substr(urlStart, urlEnd - urlStart);
        pos = urlEnd;

        mods.push_back({ path, url });
    }

    return mods;
}
