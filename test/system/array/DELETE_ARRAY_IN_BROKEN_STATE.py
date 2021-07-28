#!/usr/bin/env python3
import subprocess
import os
import sys
import time
sys.path.append("../lib/")

import json_parser
import pos
import pos_util
import cli
import test_result
import CREATE_ARRAY_NO_SPARE

ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME


def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)


def execute():
    pos_util.pci_rescan()
    out = CREATE_ARRAY_NO_SPARE.execute()
    out = cli.mount_array(ARRAYNAME)
    pos_util.pci_detach(CREATE_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(10)
    pos_util.pci_detach(CREATE_ARRAY_NO_SPARE.DATA_DEV_2)
    time.sleep(10)
    out = cli.delete_array(ARRAYNAME)
    return out


if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()
