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

#include "meta_file_intf.h"
#include "src/logger/logger.h"

#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace pos
{
MetaFileIntf::MetaFileIntf(std::string fname, std::string aname)
: fileName(fname),
  arrayName(aname),
  size(0),
  isOpened(false),
  fd(-1)
{
}

int
MetaFileIntf::IssueIO(MetaFsIoOpcode opType, uint64_t fileOffset,
    uint64_t length, char* buffer)
{
    int ret = 0;

    if (opType == MetaFsIoOpcode::Read)
    {
        ret = _Read(fd, fileOffset, length, buffer);
    }
    else if (opType == MetaFsIoOpcode::Write)
    {
        ret = _Write(fd, fileOffset, length, buffer);
    }

    return ret;
}

int
MetaFileIntf::AppendIO(MetaFsIoOpcode opType, uint64_t& offset,
    uint64_t length, char* buffer)
{
    int ret = 0;

    if (opType == MetaFsIoOpcode::Read)
    {
        ret = _Read(fd, offset, length, buffer);
    }
    else if (opType == MetaFsIoOpcode::Write)
    {
        ret = _Write(fd, offset, length, buffer);
    }

    offset += length;

    return ret;
}

int
MetaFileIntf::Open(void)
{
    isOpened = true;
    POS_TRACE_INFO(EID(SUCCESS), "File Opened, fileName:{}  fd:{}", fileName, fd);
    return 0;
}

int
MetaFileIntf::Close(void)
{
    isOpened = false;
    POS_TRACE_INFO(EID(SUCCESS), "File Closed, fileName:{}  fd:{}", fileName, fd);
    fd = -1;

    return 0;
}

bool
MetaFileIntf::IsOpened(void)
{
    return isOpened;
}

} // namespace pos
