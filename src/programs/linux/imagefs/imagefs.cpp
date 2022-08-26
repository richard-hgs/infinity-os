#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <stdexcept>
#include <vector>


#include "fat.h"

enum class EnMode {
    UNDEFINED,
    CREATE,
    WRITE
};

enum class EnFormat {
    UNDEFINED,
    FAT32
};

inline std::vector<std::string> split(const std::string& path, const std::string& sep)
{
    std::vector<std::string> out;

    size_t prev = 0;
    size_t pos = 0;

    do {
        // find next match (starting from "prev")
        pos = path.find(sep, prev);

        // no match found -> use length of string as 'match'
        if (pos == std::string::npos) {
            pos = path.length();
        }

        // get sub-string
        std::string token = path.substr(prev, pos - prev);

        // store sub-string in list
        if (!token.empty()) {
            out.push_back(token);
        }

        // move further
        prev = pos + sep.length();
    } while (pos < path.length() && prev < path.length());

    return out;
}

inline std::string join(const std::vector<std::string>& paths, const std::string& sep)
{
    if (paths.size() == 1) {
        return paths[0];
    }

    std::string out = "";

    for (auto path : paths) {

        if (out.size() == 0) {
            out += path;
            continue;
        }

        if (path[0] == sep[0]) {
            out += path;
        }
        else if (out[out.size() - 1] == sep[0]) {
            out += path;
        }
        else {
            out += sep + path;
        }
    }

    return out;
}

inline std::string join(const std::vector<std::string>& paths, const char* sep)
{
    return join(paths, std::string(sep));
}

inline std::string normpath(const std::string& path, const std::string& sep) {
    bool root = path[0] == sep[0];

    // list of path components (this removes already all "//")
    std::vector<std::string> paths = split(path, sep);

    // filter "."
    {
        std::vector<std::string> tmp;

        for (auto& i : paths) {
            if (i != ".") {
                tmp.push_back(i);
            }
        }

        paths = tmp;
    }

    // filter "foo/../"
    {
        while (true) {

            bool found = false;

            for (size_t i = 1; i < paths.size(); ++i) {

                if (paths[i] == "..") {

                    std::vector<std::string> tmp;

                    for (size_t j = 0; j < paths.size(); ++j) {
                        if (j != i && j != i - 1) {
                            tmp.push_back(paths[j]);
                        }
                    }

                    paths = tmp;
                    found = true;
                    break;
                }
            }

            if (!found) {
                break;
            }
        }
    }

    if (root) {
        return sep + join(paths, sep);
    }

    return join(paths, sep);
}

/**
 * @brief Program to create a file system
 * 
 * USAGE: 
 *   - ARG 0            : imagefs       command name
 *   - ARG 1            : <mode>        'c'=CREATE, 'w'=WRITE
 *   - <mode 'c'> ARG 2 : <format>      fat32=Fat32 file system format
 *   - <mode 'c'> ARG 3 : <size>        Total storage sectors amount
 *   - <mode 'c'> ARG 4 : <secsize>     Size in bytes for one sector
 *   - <mode 'c'> ARG 5 : <file>        Output file that will store this new created file system
 * 
 * @param argc Arguments count
 * @param argv Arguments list
 * @return int Result code or Error code
 */
int main(int argc, char **argv) {
    bool printUsage = false;
    EnMode mode = EnMode::UNDEFINED;
    EnFormat format = EnFormat::UNDEFINED;
    int totalSec = 0;
    int bytesPerSec = 0;
    char* filePath = 0;
    char cwd[PATH_MAX];
    
    // Get current working directory path
    if (getcwd(cwd, PATH_MAX) != NULL) {

        if (argc >= 2) {
            if (strcmp(argv[1], "w") == 0) {
                mode = EnMode::WRITE;
            } else if (strcmp(argv[1], "c") == 0) {
                mode = EnMode::CREATE;
            } else {
                fprintf(stderr, "\e[31mUnknow <mode> \"%s\" used. Use {c = CREATE, w = WRITE}. Use help to know more.\e[m\n", argv[1]);
                return 1;
            }
        }

        if (mode == EnMode::CREATE) { // Create a new file system architecture binary data
            if (argc < 6) {
                printUsage = true;
            } else {
                if (strcmp(argv[2], "fat32") == 0) {
                    format = EnFormat::FAT32;
                } else {
                    fprintf(stderr, "\e[31mUnknow <format> \"%s\" used. Use {fat32}. Use help to know more.\e[m\n", argv[1]);
                    return 1;
                }

                totalSec = atoi(argv[3]);
                if (totalSec <= 0) {
                    fprintf(stderr, "\e[31mUnknow <size> \"%s\" used. Size must be greather than 0.\e[m\n", argv[1]);
                    return -1;
                }

                bytesPerSec = atoi(argv[4]);
                if (bytesPerSec <= 0) {
                    fprintf(stderr, "\e[31mUnknow <secsize> \"%s\" used. Sector size must be greather than 0.\e[m\n", argv[1]);
                    return -1;
                }

                filePath = argv[5];

                fprintf(stdout, "Creating format.\n");

                // Begin the creation of the fat32 file system format.
                FatBS32 fatBS32 = {};
                fat::create(totalSec, bytesPerSec, 0x3A1B, FAT_MEDIA_TYPE_REMOVABLE, &fatBS32);

                // Join paths together and normalize then
                std::string fullFilePath = cwd;
                fullFilePath += "/";
                fullFilePath += filePath;

                // Normalize the path to remove relative paths
                fullFilePath = normpath(fullFilePath, "/");
                
                // Create a new file open and write binary contents of the file system
                FILE *o_file = fopen(fullFilePath.c_str(), "w+");
                if (o_file) {
                    std::string content = "testando123";
                    fwrite(content.c_str(), 1, content.size(), o_file);
                    fclose(o_file);
                }

                fprintf(stdout, "outputFile: %s \n", fullFilePath.c_str());

                // fprintf(stdout, "fatBS32 - oemName: %s\n", (char*) fatBS32.header.BS_OEMName);
            }
        }

        if (printUsage) {
            fprintf(stderr, "\e[31mUsage %s <mode> <format> <size> <secsize> <file>\e[m\n", argv[0]);
            return 1;
        }

        fprintf(stdout, "Argument count: %i - mode: %i.\n", argc, static_cast<int>(mode));
    } else {
        // Couldn't get CWD - Current Working Dir
        fprintf(stderr, "\e[31mCouldn't get \"cwd\" Current working dir path\e[m\n");
    }

    return 0;
}