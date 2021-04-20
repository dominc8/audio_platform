#!/bin/sh
CURR_PATH="$(dirname $(realpath $0))"
OUT_DIR="${CURR_PATH}/../"
MFILESPATH="${CURR_PATH}/../mfiles"
MATLAB_COMMAND=""
for d in ${MFILESPATH}/*/; do
    MATLAB_COMMAND="${MATLAB_COMMAND}addpath('${d}');"
done
MATLAB_COMMAND="${MATLAB_COMMAND} try; fft_16hist_generate('${OUT_DIR}'); catch; end; quit"
matlab -nodisplay -nodesktop -r "${MATLAB_COMMAND}"

