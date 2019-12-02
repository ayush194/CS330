#! bin/sh

rm /home/kparun/Documents/CS330_TA/2019/gem5/fullsystem_images/x86-system/binaries/gemOS.kernel

echo "calling make"

make clean

make

echo "calling copy"

cp gemOS.kernel /home/kparun/Documents/CS330_TA/2019/gem5/fullsystem_images/x86-system/binaries/

echo "done"
