#!/bin/bash

set -e

echo "Starting to build TFLM"

# Get the directory of this script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TFLM_SRC_DIR=$DIR/../ns-tflm

TARGET=cortex_m_generic
TOOLCHAIN=gcc
OPTIM_KERNEL=ambiq
CO_PROCESSOR=
TARGET_ARCH=cortex-m55
TARGET_TOOLCHAIN_ROOT=""
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
        CO_PROCESSOR=$CO_PROCESSOR \
        BUILD_TYPE=$BUILD \
        microlite -j8

    # Replace _ with - in the build name
    BUILD_NAME=$(echo $BUILD | tr _ -)
    TARGET_NAME=$(echo $TARGET_ARCH | sed 's/ortex-//')
    if [ -n "$CO_PROCESSOR" ]; then
        cp $TFLM_SRC_DIR/gen/${TARGET}_${TARGET_ARCH}_${BUILD}_${OPTIM_KERNEL}_${CO_PROCESSOR}_${TOOLCHAIN}/lib/libtensorflow-microlite.a \
            $DIR/lib/libtensorflow-microlite-${TARGET_NAME}-${TOOLCHAIN}-${BUILD_NAME}.a
    else
        cp $TFLM_SRC_DIR/gen/${TARGET}_${TARGET_ARCH}_${BUILD}_${OPTIM_KERNEL}_${TOOLCHAIN}/lib/libtensorflow-microlite.a \
            $DIR/lib/libtensorflow-microlite-${TARGET_NAME}-${TOOLCHAIN}-${BUILD_NAME}.a
    fi
done

echo "Creating TFLM tree"
python $TFLM_SRC_DIR/tensorflow/lite/micro/tools/project_generation/create_tflm_tree.py \
    --makefile_options "TARGET=$TARGET TARGET_ARCH=$TARGET_ARCH OPTIMIZED_KERNEL_DIR=$OPTIM_KERNEL CO_PROCESSOR=$CO_PROCESSOR" \
    treedir

cp -R $TFLM_SRC_DIR/treedir/ $DIR/
rm -rf $TFLM_SRC_DIR/treedir

echo "Finished building TFLM"
