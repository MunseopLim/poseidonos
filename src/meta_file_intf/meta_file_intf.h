/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <string>

#include "async_context.h"
#include "meta_file_include.h"

namespace pos
{
class MetaFileIntf
{
public:
    explicit MetaFileIntf(std::string fname, std::string aname);
    virtual ~MetaFileIntf(void) = default;

    virtual int Create(uint64_t size, StorageOpt storageOpt = StorageOpt::DEFAULT) = 0;
    virtual bool DoesFileExist(void) = 0;
    virtual int Delete(void) = 0;
    virtual uint64_t GetFileSize(void) = 0;

    virtual int AsyncIO(AsyncMetaFileIoCtx* ctx) = 0;
    virtual int CheckIoDoneStatus(void* data) = 0;

    virtual int Open(void);
    virtual int Close(void);
    virtual bool IsOpened(void);

    virtual int GetFd(void) { return fd; }
    virtual std::string GetFileName(void) { return fileName; }
    // SyncIO APIs
    virtual int IssueIO(MetaFsIoOpcode opType, uint64_t fileOffset, uint64_t length,
        char* buffer);
    virtual int AppendIO(MetaFsIoOpcode opType, uint64_t& offset, uint64_t length,
        char* buffer);

protected:
    virtual int _Read(int fd, uint64_t fileOffset, uint64_t length,
        char* buffer) = 0;
    virtual int _Write(int fd, uint64_t fileOffset, uint64_t length,
        char* buffer) = 0;

    std::string fileName;
    std::string arrayName;
    uint64_t size;
    bool isOpened;
    int fd;
};
} // namespace pos
