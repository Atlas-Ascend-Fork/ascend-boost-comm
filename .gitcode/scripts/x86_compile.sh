#!/bin/bash
set -ex

cd ${WORKSPACE}/scripts
ls -lr
source ./set_env.sh --namespace=AtbOps
source /home/slave1/Ascend/ascend-toolkit/latest/set_env.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${ASCEND_HOME_PATH}/$(uname -i)-linux/devlib
bash -x ./build.sh testframework
