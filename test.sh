#!/bin/bash

./build.sh || exit 1

SIM_LOG_FOLDER=./sim_logs
MEM_LOG_FOLDER=./mem_logs

mkdir -p $SIM_LOG_FOLDER
mkdir -p $MEM_LOG_FOLDER

test_manager () {
    NAME=$1
    SIM_LOG=$SIM_LOG_FOLDER/$NAME.txt
    ./build/main $NAME > $SIM_LOG 2>&1; OK="$(echo $?)"

    if [[ "$OK" -ne "0" ]]; then
        echo "❌ - $NAME";
    else
        ALL_GOOD=1
        for MEM_LOG in $MEM_LOG_FOLDER/$NAME*.log; do     
            if [[ "$(tail -n 1 $MEM_LOG)" -ne "end" ]]; then
                echo "⚠️ - $NAME"
                ALL_GOOD=0
                break
            fi
        done
        if [[ $ALL_GOOD -eq 1 ]]; then 
            echo "✅ - $NAME"
        fi
    fi
}

test_manager bnb
test_manager seg
test_manager pag