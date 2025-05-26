#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
std::string getDefaultMinecraftPath();
std::string promptUserForMinecraftPath();
std::string getMinecraftFolder();
std::string promptUserForMinecraftPath2();
static std::string path = "";
void writejsonSomethin(int mem);
void Process(int mem);