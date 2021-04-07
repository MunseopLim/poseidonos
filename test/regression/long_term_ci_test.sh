#!/bin/bash

logfile="ibofos.log"
rootdir=$(readlink -f $(dirname $0))/../..
fiodir=${rootdir}/test/system/io_path
ip="10.100.1.25"
test_iteration=2000
totalsize=100 #pm : 12500
volcnt=4
test_time=3600
cpusallowed="10-11"

# array mode [normal, degraded]
arraymode="normal"
# shutdown type [none, spor, npor]
shutdowntype="none"
# rebuild mode [none, rebuild_before_gc, rebuild_after_gc]
rebuild="none"

res=0

while getopts "f:t:i:s:c:p:a:r:" opt
do
    case "$opt" in
        f) ip="$OPTARG"
            ;;
        t) test_time="$OPTARG"
            ;;
        i) test_iteration="$OPTARG"
            ;;
        s) totalsize="$OPTARG"
            ;;
        c) cpusallowed="$OPTARG"
            ;;
        p) shutdowntype="$OPTARG"
            ;;
        a) arraymode="$OPTARG"
            ;;
        r) rebuild="$OPTARG"
    esac
done

sizepervol=`expr $totalsize / $volcnt `

shutdown()
{
	while :
	do
		ps -C ibofos > /dev/null
		if [[ ${?} != 0 ]]; then
			break;
		fi

		state=$(${rootdir}/bin/cli request info --json | jq -r '.Response.info.state' 2>/dev/null)
		if [[ $state = "NORMAL" ]]; then
			${rootdir}/bin/cli request unmount_ibofos
		elif [[ $state = "OFFLINE" ]]; then
			${rootdir}/bin/cli request exit_ibofos
		fi

		sleep 1s
	done
}

check_result()
{
    if [ $res -ne 0 ];
    then
        sudo ${rootdir}/test/script/kill_ibofos.sh
        banner failed
        exit 1
    else
        banner success
    fi
}

print_test_configuration()
{
    echo "------------------------------------------"
    echo "[Long Term Test Information]"
    echo "> Test environment:"
    echo "  - Target IP:            ${ip}"
    echo "  - Total Test Time:      `expr ${test_iteration} \* ${test_time}`"
    echo "  - Volume Count:         ${volcnt}"
    echo "  - Total Size:           ${totalsize}"
    echo "  - Size per Volume:      ${sizepervol}"
    echo "  - Shutdown Option:      ${shutdowntype}"
    echo "  - Array Mode:           ${arraymode}"
    echo "  - Rebuild trigger:      ${rebuild}"
    echo "  - Working directory:    ${rootdir}"
    echo "  - FIO directory:        ${fiodir}"
    echo ""
    echo "------------------------------------------"

}

print_test_configuration

sudo ${rootdir}/test/script/kill_ibofos.sh
sudo ${rootdir}/script/start_ibofos.sh
sleep 10

sudo ${rootdir}/test/system/longterm/setup_ibofos.sh create ${arraymode} ${totalsize} ${volcnt} ${ip}

iotype="write"
#blocksize="512b-128k"
blocksize="128k"
timebase=1
runtime=$test_time
sudo ${fiodir}/fio_bench.py --traddr=${ip} --trtype=tcp --readwrite=${iotype} \
--io_size=${sizepervol}G --verify=false --bs=${blocksize} --time_based=${timebase} \
--run_time=${runtime} --iodepth=4 --file_num=${volcnt} --cpus_allowed=${cpusallowed}

res=$?
check_result

echo "prepare write done" >> mem_history.txt
sudo ${rootdir}/test/system/longterm/mem_check.sh >> mem_history.txt

timebase=1
runtime=$test_time

if [ ${rebuild} == "rebuild_before_gc" ]; then
    rebuild=none
    sudo ${rootdir}/script/detach_device.sh unvme-ns-0 1
fi

for ((i=1;i<${test_iteration};i++))
do

    banner testcnt:${i}

    if [ $shutdowntype != "none" ]; then
        if [ $shutdowntype == "npor" ]; then
			shutdown
        else
            echo "add spor test"
        fi
        
        while [ `pgrep "ibofos"` ]
        do
            echo "wait exit ibofos"
            sleep 5
            ibofid=`pgrep "ibofos"`
            echo "process ID : $ibofid "
        done
        sudo ${rootdir}/script/start_ibofos.sh
        sleep 10
        sudo ${rootdir}/test/system/longterm/setup_ibofos.sh load ${arraymode} ${totalsize} ${volcnt} ${ip}
    fi

    iotype="randwrite"
    sudo ${fiodir}/fio_bench.py --traddr=${ip} --trtype=tcp --readwrite=${iotype} --io_size=${sizepervol}G --verify=true --bs=${blocksize} --time_based=${timebase} --run_time=${runtime} --iodepth=4 --file_num=${volcnt} --cpus_allowed=${cpuallowed}
    res=$?
    check_result

    if [ ${rebuild} == "rebuild_after_gc" ]; then
        rebuild=none
        sudo ${rootdir}/script/detach_device.sh unvme-ns-0 1
    fi

    echo "io test conut : ${i}" >> mem_history.txt
    sudo ${rootdir}/test/system/longterm/mem_check.sh >> mem_history.txt
done

#sudo ${rootdir}/test/script/kill_ibofos.sh
shutdown
echo "test end"
