#!/bin/bash

source tc_lib_ci.sh

############################
# tc_vol_0
# do nothing
############################
tc_vol_0()
{
    bringup_ibofos create

    tcName="tc_vol_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: do nothing"

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_vol_1
# [create] vol1 -> [delete] vol1
############################
tc_vol_1()
{
    bringup_ibofos create

    tcName="tc_vol_1"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [delete] vol1"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_vol_2
# [create] vol1 -> [mount] vol1 -> [unmount] vol1 -> [delete] vol1
############################
tc_vol_2()
{
    bringup_ibofos create

    tcName="tc_vol_2"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [unmount] vol1 -> [delete] vol1"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_vol_3
# [create] vol1 -> [mount] vol1 -> [io] vol1 -> NPOR -> [delete] vol1
############################
tc_vol_3()
{
    bringup_ibofos create

    tcName="tc_vol_3"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [io] vol1 -> NPOR -> [delete] vol1"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_and_verify '1'
    EXPECT_PASS "write_and_verify" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_vol_4
# [create] vol1 -> [mount] vol1 -> [create] vol2 -> [mount] vol2 -> [unmount] vol2 -> [delete] vol2 
############################
tc_vol_4()
{
    bringup_ibofos create

    tcName="tc_vol_4"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [create] vol2 -> [mount] vol2 -> [unmount] vol2 -> [delete] vol2"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    create_and_check '2' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '2'
    EXPECT_PASS "mount_and_check" $?

    unmount_and_check '2'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_vol_5
# [create] vol1 -> [mount] vol1 -> [create] vol2 -> [delete] vol2 -> NPOR
############################
tc_vol_5()
{
    bringup_ibofos create

    tcName="tc_vol_5"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [create] vol2 -> [delete] vol2 -> NPOR"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    create_and_check '2' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_vol_6
#  {[create/mount] vol1 & vol2  -> [delete] vol1 & vol2 } x 10
############################
tc_vol_6()
{
    bringup_ibofos create

    tcName="tc_vol_6"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: {[create/mount] vol1 & vol2  -> [unmount] vol1 & vol2 -> [delete] vol1 & vol2 } x 10"

    tcTestCount=10
    for fidx in `seq 1 ${tcTestCount}`
    do           
        print_notice "TC=${tcName} : All test count =  ${fidx}, total = ${tcTestCount}"    
    
        create_and_check '1' 2147483648 0 0
        EXPECT_PASS "create_and_check" $?

        mount_and_check '1'
        EXPECT_PASS "mount_and_check" $?

        create_and_check '2' 2147483648 0 0
        EXPECT_PASS "create_and_check" $?

        mount_and_check '2'
        EXPECT_PASS "mount_and_check" $?

        unmount_and_check '1'
        EXPECT_PASS "unmount_and_check" $?

        unmount_and_check '2'
        EXPECT_PASS "unmount_and_check" $?

        delete_and_check '1'
        EXPECT_PASS "delete_and_check" $?
        
        delete_and_check '2'
        EXPECT_PASS "delete_and_check" $?
    done

    end_tc "${tcName}"
    graceful_shutdown
}


############################
# tc_vol_7
#  [create] vol1 & vol2  -> {[mount] vol1 & vol2 -> [unmount] vol1 & vol2 } x 10"
############################
tc_vol_7()
{
    bringup_ibofos create

    tcName="tc_vol_7"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 & vol2  -> {[mount] vol1 & vol2 -> [unmount] vol1 & vol2 } x 10"
    
    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    create_and_check '2' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    tcTestCount=10
    for fidx in `seq 1 ${tcTestCount}`
    do           
        print_notice "TC=${tcName} : All test count =  ${fidx}, total = ${tcTestCount}"    

        mount_and_check '1'
        EXPECT_PASS "mount_and_check" $?

        mount_and_check '2'
        EXPECT_PASS "mount_and_check" $?

        unmount_and_check '1'
        EXPECT_PASS "unmount_and_check" $?

        unmount_and_check '2'
        EXPECT_PASS "unmount_and_check" $?
    done

    end_tc "${tcName}"
    graceful_shutdown
}


tc_vol_8()
{
    tcName="tc_vol8"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [mount] ibofos -> [create/mount/unmount] vol -> [delete] vol -> [unmount] ibofos } x 10"
        
    tcTestCount=10
    for fidx in `seq 1 ${tcTestCount}`
    do
        print_notice "TC=${tcName} : All test count =  ${fidx}, total = ${tcTestCount}"    

        bringup_ibofos create

        create_and_check '01' 4194304 0 0
        EXPECT_PASS "create_and_check" $?

        texecc ./bin/cli request rename_vol --name vol01 --newname volxx
        texecc ./bin/cli request rename_vol --name volxx --newname vol01
        texecc ./bin/cli request list_vol

        texecc ./bin/cli request mount_vol --name vol01
        texecc ./bin/cli request unmount_vol --name vol01
        
        texecc ./bin/cli request delete_vol --name vol01

        texecc ./bin/cli request unmount_ibofos
        texecc ./bin/cli request delete_array
    done

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_npor_0
# [create] vol1 -> [mount] vol1 -> [write] vol1 -> NPOR -> [mount] vol1 -> [verify] vol1
############################
tc_npor_0()
{
    bringup_ibofos create

    tcName="tc_npor_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: vol1 -> [mount] vol1 -> [write] vol1 -> NPOR -> [mount] vol1 -> [verify] vol1"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_npor_1
# [create] vol1 -> [mount] vol1 -> [create] vol2 -> [mount] vol2 -> [write] vol1 -> [write] vol2 -> NPOR -> [mount] vol1 -> [verify] vol1 -> [mount] vol2 -> [verify] vol2
############################
tc_npor_1()
{
    bringup_ibofos create

    tcName="tc_npor_1"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [create] vol2 -> [mount] vol2 -> [write] vol1 -> [write] vol2 -> NPOR -> [mount] vol1 -> [verify] vol1 -> [mount] vol2 -> [verify] vol2"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    create_and_check '2' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '2'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    write_data '2'
    EXPECT_PASS "write_data" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    mount_and_check '2'
    EXPECT_PASS "mount_and_check" $?

    verify_data '2'
    EXPECT_PASS "verify_data" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_npor_2
# [create] vol1 -> [mount] vol1 -> [write] vol1 -> NPOR -> [mount] vol1 -> [verify] vol1 -> [unmount] vol1 -> [delete] vol1 -> NPOR
############################
tc_npor_2()
{
    bringup_ibofos create

    tcName="tc_npor_2"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> [mount] vol1 -> [write] vol1 -> NPOR -> [mount] vol1 -> [verify] vol1 -> [unmount] vol1 -> [delete] vol1 -> NPOR"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    delete_and_check '1'
    EXPECT_PASS "delete_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_spor_0
# [create] vol1 -> [mount] vol1 -> [write] vol1 -> SPOR -> [mount] vol1 -> [verify] vol1
############################
tc_spor_0()
{
    bringup_ibofos create

    tcName="tc_spor_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "[create] vol1 -> [mount] vol1 -> [write] vol1 -> SPOR -> [mount] vol1 -> [verify] vol1"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    spor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc_inode_0
# simple test for compaction
############################
tc_inode_0()
{
    bringup_ibofos create
    EXPECT_PASS "bringup" $?

    tcName="tc_inode_0"
    show_tc_info "${tcName}"
    start_tc "${tcName}"

    create_and_check '2' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    delete_and_check '2'
    EXPECT_PASS "delete_and_check" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?

    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    verify_data '1'
    EXPECT_PASS "verify_data" $?

    end_tc "${tcName}"
    graceful_shutdown
}

tc_npor_3()
{
    bringup_ibofos create

    tcName="tc_npor_3"
    show_tc_info "${tcName}"
    start_tc "${tcName}"
    print_info "scenario: [create] vol1 -> ([mount] vol1 -> [write] vol1 -> [unmount] vol1 -> [NPOR]) x 50"

    create_and_check '1' 2147483648 0 0
    EXPECT_PASS "create_and_check" $?

    tcTestCount=50
   for fidx in `seq 1 ${tcTestCount}`
    do           
        print_notice "TC=${tcName} : All test count =  ${fidx}, total = ${tcTestCount}"    

    mount_and_check '1'
    EXPECT_PASS "mount_and_check" $?

    write_data '1'
    EXPECT_PASS "write_data" $?

    unmount_and_check '1'
    EXPECT_PASS "unmount_and_check" $?
    
    npor_and_check_volumes
    EXPECT_PASS "npor_and_check_volumes" $?

    done    

    end_tc "${tcName}"
    graceful_shutdown
}

############################
# tc array
############################
run()
{
    ####################################
    # add tc name here
    ####################################
    if [ $isVm == 0 ];
    then
        tc_array=(
                c_vol_2 tc_vol_3 tc_vol_4
                tc_vol_5 tc_vol_6 tc_vol_7 tc_vol_8
                tc_npor_1 tc_npor_2
                tc_inode_0
            )
    else
        tc_array=(
            tc_vol_5 tc_vol_6 tc_vol_7 
            tc_npor_2
            tc_npor_3
            tc_inode_0
        )
    fi
    
    tcTotalCount=${#tc_array[@]}

    ####################################
    # setting env.
    ####################################
    # 0: loop-back(vm), 1: nvmeof(pm), 2: echo
    exec_mode=0

    # # of subsystems
    support_max_subsystem=2

    # protocol
    trtype="tcp"

    # # target ip
    # target_ip=127.0.0.1
    target_ip=`ifconfig | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}' | sed -n '1,1p'`
    target_fabric_ip=`ifconfig | awk '$1 == "inet" {gsub(/\/.*$/, "", $2); print $2}' | sed -n '2,1p'`

    # root dir: initiator
    rootInit=$(readlink -f $(dirname $0))/../..

    # root dir: target
    rootTarget=$(readlink -f $(dirname $0))/../..
    ####################################

    update_config

    local tcList=""

    for fidx in "${!tc_array[@]}"
    do
        ${tc_array[fidx]}
        tcList+="   $((fidx+1)): ${tc_array[fidx]}\n"
    done

    print_notice "All TCs (${tcTotalCount} TCs) have passed.\n${tcList}"
}

isVm=0

while getopts "f:v:" opt
do
    case "$opt" in
        f) target_fabric_ip="$OPTARG"
            ;;
        v) isVm="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

network_module_check
run;

exit 0
