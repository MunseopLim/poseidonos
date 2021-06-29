#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import test_result
import pos_constant
import MOUNT_ARRAY_BASIC
import volume

VOL_NAME = "vol1"
VOL_SIZE = pos_constant.SIZE_1GB
VOL_IOPS = 0
VOL_BW = 0

SPARE = MOUNT_ARRAY_BASIC.SPARE
ANY_DATA = MOUNT_ARRAY_BASIC.ANY_DATA

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(detail):
    code = json_parser.get_response_code(out)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_ARRAY_BASIC.execute()
    out = cli.create_volume(VOL_NAME, str(VOL_SIZE), "", "", "wrong_array_name")
    print(out)
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    pos.kill_pos()