#!/usr/bin/env bash

set -e


sudo pacman -S --needed gcc lib32-gcc-libs aarch64-linux-gnu-gcc arm-none-eabi-gcc riscv64-linux-gnu-gcc 

rm -rf ./build
mkdir -p ./build

# Lista de compiladores con arquitectura/sistema
declare -A compilers=(
    # Linux
    ["linux-x86_64"]="gcc"
    ["linux-i686"]="gcc -m32"
    ["linux-aarch64"]="aarch64-linux-gnu-gcc"
    ["linux-riscv64"]="riscv64-linux-gnu-gcc"
)

for target in "${!compilers[@]}"; do
    make clean
    make BUILD_DIR="build"      \
    CC="${compilers[$target]}"  \
    FLAGS=""                    \
    BIN_NAME=vicel_$target
done
