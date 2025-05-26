#include <filesystem>
#include "unzipper.hpp"
#include <cstdlib>
#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "miniz.h" 


void copyOverrides(const std::string& src, const std::string& dest) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(src)) {
        if (!entry.is_regular_file()) continue;
        auto rel = std::filesystem::relative(entry.path(), src);
        auto outPath = std::filesystem::path(dest) / rel;
        std::filesystem::create_directories(outPath.parent_path());
        std::filesystem::copy_file(entry.path(), outPath, std::filesystem::copy_options::overwrite_existing);
    }
}


bool unzip_file(const std::string& zip_path, const std::string& output_dir) {
    mz_zip_archive zip_archive;
    memset(&zip_archive, 0, sizeof(zip_archive));

    if (!mz_zip_reader_init_file(&zip_archive, zip_path.c_str(), 0)) {
        std::cerr << "Failed to open ZIP archive: " << zip_path << std::endl;
        return false;
    }

    int file_count = (int)mz_zip_reader_get_num_files(&zip_archive);
    for (int i = 0; i < file_count; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
            std::cerr << "Failed to get file stat for index " << i << std::endl;
            continue;
        }

        std::string output_path = output_dir + "/" + file_stat.m_filename;
        if (mz_zip_reader_is_file_a_directory(&zip_archive, i)) {
            std::filesystem::create_directories(output_path);
        } else {
            std::filesystem::create_directories(std::filesystem::path(output_path).parent_path());
            if (!mz_zip_reader_extract_to_file(&zip_archive, i, output_path.c_str(), 0)) {
                std::cerr << "Failed to extract file: " << file_stat.m_filename << std::endl;
            }
        }
    }

    mz_zip_reader_end(&zip_archive);
    return true;
}

