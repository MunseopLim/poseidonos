#!/usr/bin/env python3

import subprocess
import sys

import TEST
import TEST_LOG
import TEST_LIB

#######################################################################################
q_depth = 4
block_size = '4K'
core_mask = '0x40'
#######################################################################################


def write(arrayId, volumeId, offset, size, pattern, runTime=0):
    TEST_LOG.print_info("* Write data: array" + str(arrayId) + ", vol" + str(volumeId) + ", offset " + str(offset) +
                        ", size " + str(size) + ", pattern " + str(pattern) + ", run_time " + str(runTime))
    test_out = open(TEST.output_log_path, "a")
    ret = run_fio("write", arrayId, volumeId, size, offset, pattern, TEST.traddr, TEST.port, "tcp", runTime, logger=test_out)
    if ret != 0:
        TEST_LOG.print_err("* Write Failed (array{}, vol{})".format(arrayId, volumeId))
        sys.exit(1)
    else:
        TEST_LOG.print_info("* Write Success (array{}, vol{})".format(arrayId, volumeId))


def verify(arrayId, volumeId, offset, size, pattern):
    TEST_LOG.print_info("* Verify data: array" + str(arrayId) + ", vol" + str(volumeId) + ", offset " + str(offset) +
                        ", size " + str(size) + ", pattern " + str(pattern))
    test_out = open(TEST.output_log_path, "a")
    ret = run_fio("read", arrayId, volumeId, size, offset, pattern, TEST.traddr, TEST.port, "tcp", logger=test_out)
    if ret != 0:
        TEST_LOG.print_err("* Verify Failed (array{}, vol{})".format(arrayId, volumeId))
        sys.exit(1)
    else:
        TEST_LOG.print_info("* Verify Success (array{}, vol{})".format(arrayId, volumeId))


# TODO (huijeong.kim) to update test cases with this verify
def verify_with_two_patterns(arrayId, volumeId, offset, size, pattern1, pattern2):
    TEST_LOG.print_info("* Verify data: array " + str(arrayId) + " vol " + str(volumeId) + ", offset " + str(offset) +
                        ", size " + str(size) + ", pattern " + str(pattern1) + " or " + str(pattern2))
    test_out = open(TEST.output_log_path, "a")

    current_offset = offset
    while current_offset < offset + TEST_LIB.parse_size(size):
        verify_block(volumeId, current_offset, "4k", pattern1, pattern2, test_out)
        current_offset += 4096


def verify_block(arrayId, volumeId, offset, size, pattern1, pattern2, logger=0):
    ret = run_fio("read", volumeId, size, offset, pattern1, TEST.traddr, TEST.port, "tcp", logger=logger)
    if ret != 0:
        ret = run_fio("read", volumeId, size, offset, pattern2, TEST.traddr, TEST.port, "tcp", logger=logger)
        if ret != 0:
            TEST_LOG.print_err("* Verify Failed (array{}, vol{})".format(arrayId, volumeId))
            sys.exit(1)
        else:
            TEST_LOG.print_info("* Verify Success with pattern {} at offset {} (vol {})".format(pattern2, offset, volumeId))
            return 0
    else:
        TEST_LOG.print_info("* Verify Success with pattern {} at offset {} (vol {})".format(pattern1, offset, volumeId))
        return 0


def run_fio(workload, arrayId, volumeId, io_size_bytes, io_offset, verify_pattern, ip, port, transport="tcp", runTime=0, logger=0):
    subsystemId = TEST_LIB.get_subsystem_id(arrayId, volumeId)
    msg = "io={}, pattern={}".format(workload, verify_pattern)
    filename = "trtype=" + transport + " adrfam=IPv4" + " traddr=" + ip + " trsvcid=" + str(port) + " subnqn=nqn.2019-04.pos\\:subsystem" + str(subsystemId) + " ns=1"
    command = "fio --thread=1 --group_reporting=1 --direct=1 " \
        + " --ioengine=" + TEST.ioengine + "" \
        + " --size=" + str(io_size_bytes) + "" \
        + " --bs=" + block_size + "" \
        + " --iodepth=" + str(q_depth) + "" \
        + " --readwrite=" + workload

    command += " --offset=" + str(io_offset)
    command += " --verify=pattern --verify_pattern=" + verify_pattern + " --verify_dump=1 --verify_state_save=0"
    command += " --numjobs=1 --ramp_time=0 --bs_unaligned=1 "
    command += " --name=test --filename='" + filename + "'"

    if runTime != 0:
        command += " --runtime=" + str(runTime) + ""
        command += " --time_based=" + str(1) + ""
        msg += ", run_time={}".format(runTime)
    else:
        command += " --time_based=" + str(0) + ""
        msg += ", size={}".format(io_size_bytes)
    msg += ", block_size={}, qd={}, cpu mask={}".format(block_size, q_depth, core_mask)
    TEST_LOG.print_info("[FIO-array{},vol{}] Started. {}".format(arrayId, volumeId, msg), color="")
    TEST_LOG.print_debug(command)

    if logger != 0:
        ret = subprocess.call(command, shell=True, stdout=logger, stderr=logger)
    else:
        ret = subprocess.call(command, shell=True)

    if ret != 0:
        TEST_LOG.print_info("[FIO-array{},vol{}] Terminated".format(arrayId, volumeId))
    else:
        TEST_LOG.print_info("[FIO-array{},vol{}] Success.".format(arrayId, volumeId), color="")

    return ret
