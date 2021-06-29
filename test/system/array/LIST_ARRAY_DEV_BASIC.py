#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")

import json_parser
import pos
import cli
import test_result
import CREATE_ARRAY_BASIC
import array_device

ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME

def check_result(detail):
    expected_list = []
    expected_list.append(array_device.ArrayDevice("uram0", "BUFFER"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-0", "DATA"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-1", "DATA"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-2", "DATA"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-3", "SPARE"))

    data = json.loads(detail)
    actual_list = []
    for item in data['Response']['result']['data']['devicelist']:
        dev = array_device.ArrayDevice(**item)
        actual_list.append(dev)
    
    for actual in actual_list:
        checked = False
        for expected in expected_list:
            if actual.name == expected.name and actual.type == expected.type:
                checked = True
                break
        if checked == False:
            return "fail"
    return "pass"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = check_result(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    out = CREATE_ARRAY_BASIC.execute()
    ret = json_parser.get_response_code(out)
    if ret == 0:
        out = cli.list_array_device(ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()