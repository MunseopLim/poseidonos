#!/usr/bin/env python3

import subprocess
import sys

IBOFOS_ROOT = '../../../'
IBOFOS_CLI = IBOFOS_ROOT + "bin/cli"

def send_request(msg):
    cli_req = IBOFOS_CLI + " --json "
    out = subprocess.check_output(cli_req + msg, universal_newlines=True, shell=True)
    return out

def mount_ibofos():
    return send_request("system mount")

def unmount_ibofos():
    return send_request("system unmount")

def exit_ibofos():
    return send_request("system exit")

def get_ibofos_info():
    return send_request("system info")

def get_version():
    return send_request("system version")

def create_array(buffer, data, spare, array, raid):
    param_str = ""
    if (buffer != ""):
        param_str += "-b " + buffer
    if (data != ""):
        param_str += " -d " + data
    if spare != "":
        param_str += " -s " + spare
    if array != "":
        param_str += " --name " + array
    if raid != "":
        param_str += " --raidtype " + raid
    return send_request("array create " + param_str)

def delete_array(array):
    param_str = ""
    if array != "":
        param_str += "--name " + array
    return send_request("array delete " + param_str)

def load_array(array):
    param_str = ""
    if array != "":
        param_str += "--name " + array
    return send_request("array load " + param_str)

def list_array_device(array):
    param_str = ""
    if array != "":
        param_str += "--name " + array
    return send_request("array list_device " + param_str)

def array_info(array):
    param_str = ""
    if array != "":
        param_str += "--name " + array
    return send_request("array info " + param_str)

def add_device(spare, array):
    param_str = "--spare " + spare
    if array != "":
        param_str += " --array " + array
    return send_request("array add " + param_str)

def remove_device(spare, array):
    param_str = "--spare " + spare
    if array != "":
        param_str += " --array " + array
    return send_request("array remove " + param_str)

def create_volume(vol_name, size, iops, bw, array):
    param_str = "--name " + vol_name + " --size " + size
    if iops != "":
        param_str += " --maxiops " + iops
    if bw != "":
        param_str += " --maxbw " + bw
    if array != "":
        param_str += " --array " + array

    return send_request("volume create " + param_str)

def delete_volume(vol_name, array):
    param_str = "--name " + vol_name
    if array != "":
        param_str += " --array " + array
    return send_request("volume delete " + param_str)

def mount_volume(vol_name, array, subnqn):
    param_str = "--name " + vol_name
    if array != "":
        param_str += " --array " + array
    if subnqn != "":
        param_str += " --subnqn " + subnqn
    return send_request("volume mount " + param_str)

def unmount_volume(vol_name, array):
    param_str = "--name " + vol_name
    if array != "":
        param_str += " --array " + array
    return send_request("volume unmount " + param_str)

def list_volume(array):
    param_str = ""
    if array != "":
        param_str += " --array " + array
    return send_request("volume list " + param_str)

def update_volume_qos(vol_name, iops, bw, array):
    param_str = "--name " + vol_name
    if iops != "":
        param_str += " --maxiops " + iops
    if bw != "":
        param_str += " --maxbw " + bw
    if array != "":
        param_str += " --array " + array

    return send_request("volume update_qos " + param_str)

def rename_volume(vol_name, new_name, array):
    param_str = "--name " + vol_name + " --newname " + new_name
    if array != "":
        param_str += " --array " + array

    return send_request("volume rename " + param_str)

def scan_device():
    return send_request("device scan")

def list_device():
    return send_request("device list")

def device_monitoring_state():
    return send_request("request monitoring_state")

def update_event_qos(event_name, perf_impact):
    prio = -1
    weight = -1
    if perf_impact == "high":
        prio = 0
        weight = 1
    elif perf_impact == "medium":
        prio = 1
        weight = 2
    elif  perf_impact == "low":
        prio = 2
        weight = 3

    return send_request("internal update_event_wrr --name " + event_name + 
        " --prio " + str(prio) + " --weight " + str(weight))

def wbt_request(event_name, argument):
    return send_request("wbt " + event_name + argument)