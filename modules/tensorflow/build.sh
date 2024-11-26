#!/bin/bash

set -e

# Get the directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TFLM_SRC_DIR=$DIR/../ns-tflm

TARGET=cortex_m_generic
TOOLCHAIN=gcc
OPTIM_KERNEL=cmsis_nn
TARGET_ARCH=cortex-m55
# Build TFLM with release, release_with_logs, and debug
BUILDS=(release release_with_logs debug)

cd $DIR

mkdir -p $DIR/lib

for BUILD in ${BUILDS[@]}; do
    echo "Building TFLM with $BUILD"

    cd $TFLM_SRC_DIR

    make -f $TFLM_SRC_DIR/tensorflow/lite/micro/tools/make/Makefile \
        TARGET=$TARGET \
        TARGET_ARCH=$TARGET_ARCH \
        TOOLCHAIN=$TOOLCHAIN \
        OPTIMIZED_KERNEL_DIR=$OPTIM_KERNEL  \
        BUILD_TYPE=$BUILD \
        microlite -j8

    # Replace _ with - in the build name
    BUILD_NAME=$(echo $BUILD | tr _ -)
    TARGET_NAME=$(echo $TARGET_ARCH | sed 's/ortex-//')
    cp $TFLM_SRC_DIR/gen/${TARGET}_${TARGET_ARCH}_${BUILD}_${OPTIM_KERNEL}_${TOOLCHAIN}/lib/libtensorflow-microlite.a \
        $DIR/lib/libtensorflow-microlite-${TARGET_NAME}-${TOOLCHAIN}-${BUILD_NAME}.a
done

python $TFLM_SRC_DIR/tensorflow/lite/micro/tools/project_generation/create_tflm_tree.py \
    --makefile_options "TARGET=$TARGET TARGET_ARCH=$TARGET_ARCH OPTIMIZED_KERNEL_DIR=$OPTIM_KERNEL" \
    treedir

cp -R $TFLM_SRC_DIR/treedir/ $DIR/
