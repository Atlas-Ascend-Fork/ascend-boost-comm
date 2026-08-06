#!/bin/bash
set -ex

gcc --version

source /home/slave1/Ascend/ascend-toolkit/latest/set_env.sh
export LD_LIBRARY_PATH=/usr/local/Ascend/driver/lib64/driver/:$LD_LIBRARY_PATH
bash scripts/build.sh unittest

source output/mki/set_env.sh
mkdir tmp_output
GTEST_OUTPUT="xml:./tmp_output/log" mki_unittest --gtest_filter=-*Memset*:Rt*
cat ./tmp_output/log

ret=$?
if [ $ret -ne 0 ]; then
    echo "run ut fail"
    exit 1
fi
exit 0
