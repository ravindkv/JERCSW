#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

#include "TDirectory.h"

class HelperDir {
public:
    // Method to create or get nested directories
    static inline TDirectory* createTDirectory(TDirectory* origDir,
                                               const std::string& directoryPath)
    {
        if (!origDir) {
            throw std::invalid_argument("HelperDir::createTDirectory - Invalid directory pointer provided.");
        }

        TDirectory* currentDir = origDir;

        // Fast path: empty path -> return origDir
        if (directoryPath.empty()) return currentDir;

        std::stringstream ss(directoryPath);
        std::string dirName;

        while (std::getline(ss, dirName, '/')) {
            if (dirName.empty()) continue; // tolerate leading/trailing//double slashes

            TDirectory* subDir = currentDir->GetDirectory(dirName.c_str());
            if (!subDir) {
                subDir = currentDir->mkdir(dirName.c_str());
                if (!subDir) {
                    throw std::runtime_error(
                        std::string("HelperDir::createTDirectory - Failed to create directory: ") +
                        dirName + " in path " + currentDir->GetPath()
                    );
                }
            }
            currentDir = subDir;
        }
        return currentDir;
    }
};

