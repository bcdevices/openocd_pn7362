#!/bin/bash
SCRIPTS=/usr/local/share/openocd/scripts
INTERFACE=$SCRIPTS/interface/ftdi/ft232h-module-swd.cfg

CPU=./pn73xxxx.cfg
EXE=openocd

CHECK_FILE(){
    F1=$1
    F2=${F1}_Readback
    MDFB1=`cat ${F1}.bin | md5sum`
    MDFB2=`cat ${F2}.bin | md5sum`
    if [ "$MDFB1" != "$MDFB2" ]; then
        echo ERROR on $F1
        xxd $F1.bin >$F1.hex
        xxd $F2.bin >$F2.hex
        diff $F1.hex $F2.hex
        CHECKS_OK=0 
    else
        echo Verified $F1
    fi
}


DATASIZE=161792
DEMOFILE=FactoryBlinkDemo

ITERATIONS=50

for i in {0..50}
do

    rm -f *_Readback*
    echo
    echo =============== Iteration $i of $ITERATIONS ===================
    echo == Generating random data ==
    dd if=/dev/urandom of=RandomData.bin bs=1024 count=158
    dd if=/dev/urandom of=RandomDataEEPROM.bin bs=64 count=1

    echo $EXE -f $INTERFACE -f $CPU -f testrun.cfg
    sudo $EXE -f $INTERFACE -f $CPU -f testrun.cfg

    if [ $? == "0" ]; then
        CHECKS_OK=1

        CHECK_FILE $DEMOFILE
        CHECK_FILE RandomData
        CHECK_FILE RandomDataEEPROM
    
        if [ $CHECKS_OK == "0" ]; then
            echo ERROR verifying data on iteration $i
            break
        else
            echo Verified ok
        fi
    else
        echo OpenOCD failed on iteration $i
        break
    fi

done
