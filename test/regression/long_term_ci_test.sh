#!/bin/bash

logfile="pos.log"
rootdir=$(readlink -f $(dirname $0))/../..
iopathdir=${rootdir}/test/system/io_path
test_iteration=4
totalsize=100 #pm : 12500
volume_cnt=4
clean_bringup=1
transport=TCP
target_ip=127.0.0.1
subsystem_count=4
test_time=300
cpusallowed="10-11"

# array mode [normal, degraded]
arraymode="normal"
# shutdown type [none, spor, npor]
shutdowntype="none"
# rebuild mode [none, rebuild_before_gc, rebuild_after_gc]
rebuild="none"

# volume test [none, vol_unmount, vol_delete]
volumetest="none"
res=0
array_name="POSArray"

while getopts "f:t:i:s:c:p:a:r:v:" opt
do
    case "$opt" in
        f) target_ip="$OPTARG"
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
            ;;
        v) volumetest="$OPTARG"
    esac
done

sizepervol=`expr $totalsize / $volume_cnt `

shutdown()
{
	while :
	do
		ps -C poseidonos > /dev/null
		if [[ ${?} != 0 ]]; then
			break;
		fi

		state=$(${rootdir}/bin/cli array info --name ${array_name} --json | jq -r '.Response.result.data.state' 2>/dev/null)
		if [[ $state = "NORMAL" ]]; then
			${rootdir}/bin/cli array unmount --name $array_name
		elif [[ $state = "OFFLINE" ]]; then
			${rootdir}/bin/cli system exit
		fi

		sleep 1s
	done
}

check_result()
{
    if [ $res -ne 0 ];
    then
        sudo ${rootdir}/test/script/kill_poseidonos.sh
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
    echo "  - Target IP:                ${target_ip}"
    echo "  - Total Test Time:          `expr ${test_iteration} \* ${test_time}`"
    echo "  - Volume Count:             ${volume_cnt}"
    echo "  - Total Size:               ${totalsize}"
    echo "  - Size per Volume:          ${sizepervol}"
    echo "  - Shutdown Option:          ${shutdowntype}"
    echo "  - Array Mode:               ${arraymode}"
    echo "  - Rebuild trigger:          ${rebuild}"
    echo "  - Working directory:        ${rootdir}"
    echo "  - FIO/Bring up directory:   ${iopathdir}"
    echo ""
    echo "------------------------------------------"

}

print_test_configuration

sudo ${rootdir}/test/script/kill_poseidonos.sh
sudo ${rootdir}/script/start_poseidonos.sh
sleep 10

sudo ${iopathdir}/setup_ibofos_nvmf_volume.sh -c $clean_bringup -t $transport -a $target_ip -s $subsystem_count -v $volume_cnt -S $((sizepervol*1024*1024*1024))
clean_bringup=0

iotype="write"
#blocksize="512b-128k"
blocksize="128k"
timebase=1
runtime=$test_time
sudo ${iopathdir}/fio_bench.py --traddr=${target_ip} --trtype=tcp --readwrite=${iotype} \
--io_size=${sizepervol}G --verify=false --bs=${blocksize} --time_based=${timebase} \
--run_time=${runtime} --iodepth=4 --file_num=${volume_cnt} --cpus_allowed=${cpusallowed}

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
            sudo ${rootdir}/test/script/kill_poseidonos.sh
            sleep 5
            sudo ${rootdir}/script/backup_latest_hugepages_for_uram.sh
            sleep 5
        fi
        
        while [ `pgrep "poseidonos"` ]
        do
            echo "wait exit poseidonos"
            sleep 5
            ibofid=`pgrep "poseidonos"`
            echo "process ID : $ibofid "
        done
        sudo ${rootdir}/script/start_poseidonos.sh
        sleep 10
        sudo ${iopathdir}/setup_ibofos_nvmf_volume.sh -c $clean_bringup -t $transport -a $target_ip -s $subsystem_count -v $volume_cnt -S $((sizepervol*1024*1024*1024))
    fi

    iotype="randwrite"
    sudo ${iopathdir}/fio_bench.py --traddr=${target_ip} --trtype=tcp --readwrite=${iotype} --io_size=${sizepervol}G \
    --verify=true --bs=${blocksize} --time_based=${timebase} --run_time=${runtime} --iodepth=4 --file_num=${volume_cnt} \
    --cpus_allowed=${cpusallowed}
    res=$?
    check_result

    if [ $volumetest != "none" ]; then
        if [ $volume_cnt == 1 ]; then
            break;
        fi
        volName=vol${volume_cnt}
        echo "vol name : $volName"
        sudo ${rootdir}/bin/cli volume unmount --name $volName --array POSArray
        if [ $volumetest == "vol_delete" ]; then
            sudo ${rootdir}/bin/cli volume delete --name $volName --array POSArray
        fi
        volume_cnt=$((volume_cnt-1))
        echo "vol cnt : $volume_cnt"
    fi

    if [ ${rebuild} == "rebuild_after_gc" ]; then
        rebuild=none
        sudo ${rootdir}/script/detach_device.sh unvme-ns-0 1
    fi

    echo "io test conut : ${i}" >> mem_history.txt
    sudo ${rootdir}/test/system/longterm/mem_check.sh >> mem_history.txt
done

#sudo ${rootdir}/test/script/kill_poseidonos.sh
shutdown
echo "test end"
