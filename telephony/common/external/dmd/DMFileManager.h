/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * DMFileManager.h
 *
 *  Created on: 2018. 5. 18.
 */

#ifndef __DM_FILE_MANAGER_H__
#define __DM_FILE_MANAGER_H__

#include <fstream>
#include <string>
#include <list>

using namespace std;

class DMFileManager {
private:
    struct DMFile {
        string fileName;
        unsigned int fileSize;
    };
    list<DMFile> mFileList;
    unsigned int mCapacity; /// disk used percent
    unsigned int mManagedFileSize;  // file limit
    string mBaseDir;
    unsigned int mTotalSize;
    unsigned int mMaxSize;

private:
    DMFileManager();
public:
    virtual ~DMFileManager();

public:
    void init();
    void add(const char *fileName);
    void add(const string &fileName);
    void removeAll();
    void refreshFileList();
    void shrink();
    void setLimit(unsigned int capacity, unsigned int managedFileSize);
    void setBaseDir(const char *baseDir);
protected:
    void createManagedFileList();

public:
    static DMFileManager *getInstance();
};

#endif // __DM_FILE_MANAGER_H__
