#!/bin/sh

#export M5_PATH=/home/ayushk/gem5/gemos
make clean
make
sudo cp gemOS.kernel /home/ayushk/gem5/gemos/binaries
../../gem5/build/X86/gem5.opt ../../gem5/configs/example/fs.py --kernel=/home/ayushk/gem5/gemos/binaries/gemOS.kernel --mem-size=2048MB

#telnet localhost 3456
