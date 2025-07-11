// version 2.0

#pragma once

#include "include/Types.h"

class FileHolder {
public:
    FileHolder() : mFileName("Dummy"), mFileData(0), mFileSize(0) {}
    FileHolder(const char* pFileName);
    FileHolder(const FileHolder*);

    bool loadFile();
    void closeFile();
    bool writeFile();

    const char* mFileName;
    char* mFileData;
    size_t mFileSize;
};