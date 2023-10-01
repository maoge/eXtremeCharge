#! /bin/bash

function stop_process(){
    pid_file=$1
    process_name=$2

    if [ ! -f "$pid_file" ]; then
        echo "$pid_file 文件不存在"
        exit 127
    fi

    pid=`cat $pid_file`
    pid_exist=`ps -f -p $pid | grep $process_name`

    if [ -z "$pid_exist" ]; then
        echo "error: The $process_name process:$pid does not exists!"
        exit 127
    fi

    echo "stopping $process_name $pid ......"
    kill $pid > /dev/null 2>&1

    sleep 1
    pid_exist=`ps -f -p $pid | grep $process_name`
    if [ -n "$pid_exist" ]; then
        kill $pid
    fi

    echo "stop $process_name $pid ok ......"
}

pid_file=./log/zz_collect.pid
process_name="zz_collect"

stop_process $pid_file $process_name
