#!/bin/bash

BASE_PROJECTS_DIR="/home/rainti/project/GD32G5x3_Demo_Suites/GD32G553R_EVAL_Demo_Suites/Projects"
FW_DIR="/home/rainti/project/GD32G5x3_Demo_Suites/GD32G5x3_Firmware_Library"
UTIL_DIR="/home/rainti/project/GD32G5x3_Demo_Suites/GD32G553R_EVAL_Demo_Suites/Utilities"
TEMPLATE_PROJ="09_I2C_EEPROM"
PRINTF_PROJ="04_USART_Printf"

projects=$(ls -d ${BASE_PROJECTS_DIR}/[0-9][0-9]_*)

for proj in $projects; do
    proj_name=$(basename $proj)
    
    if [ -f "${proj}/CMakeLists.txt" ]; then
        echo "Skipping $proj_name, CMakeLists.txt already exists."
        continue
    fi

    echo "Migrating $proj_name..."

    # 1. Create build directory
    mkdir -p "${proj}/build"

    # 2. Copy toolchain.cmake
    cp "${BASE_PROJECTS_DIR}/${TEMPLATE_PROJ}/toolchain.cmake" "${proj}/"

    # 3. Generate CMakeLists.txt
    # Detect if it's a printf/syscalls style project (common for USART)
    # We'll check if main.c contains 'printf' as a heuristic or if it's 04_USART_Printf (already done)
    # Or simply add syscalls.c if it's helpful and doesn't conflict. 
    # Actually, the prompt says "If a project folder contains specific sub-folders like Third_party or needs syscalls.c (like USART Printf)"
    
    NEEDS_SYSCALLS=0
    if [[ "$proj_name" == *"USART"* ]] || grep -q "printf" "${proj}/main.c" 2>/dev/null; then
        NEEDS_SYSCALLS=1
    fi

    CAT_SYSCALLS=""
    if [ $NEEDS_SYSCALLS -eq 1 ]; then
        CAT_SYSCALLS="    \"\${FW_DIR}/CMSIS/GD/GD32G5x3/Source/GCC/newlib/syscalls.c\""
    fi

    # Clean project name for CMake (no dots or special chars if any, though here they are simple)
    CLEAN_NAME="GD32G553_${proj_name// /_}"

    cat <<EOF > "${proj}/CMakeLists.txt"
cmake_minimum_required(VERSION 3.10)

# Project Name
project(${CLEAN_NAME} C ASM)

# Toolchain configuration
set(CMAKE_C_STANDARD 99)

# Core definitions
add_definitions(-DGD32G553)
add_definitions(-DUSE_STDPERIPH_DRIVER)

# MCU Flags: Cortex-M33 with FPU and DSP
set(MCU_FLAGS "-mcpu=cortex-m33 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "\${CMAKE_C_FLAGS} \${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "\${CMAKE_ASM_FLAGS} \${MCU_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "\${CMAKE_EXE_LINKER_FLAGS} \${MCU_FLAGS} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs")

# Paths
set(PROJ_DIR "${proj}")
set(FW_DIR "${FW_DIR}")
set(UTIL_DIR "${UTIL_DIR}")

# Include Directories
include_directories(
    \${PROJ_DIR}
    \${UTIL_DIR}
    \${FW_DIR}/CMSIS
    \${FW_DIR}/CMSIS/GD/GD32G5x3/Include
    \${FW_DIR}/GD32G5x3_standard_peripheral/Include
)

# Source Files
file(GLOB SOURCES
    "\${PROJ_DIR}/*.c"
    "\${UTIL_DIR}/*.c"
    "\${FW_DIR}/GD32G5x3_standard_peripheral/Source/*.c"
    "\${FW_DIR}/CMSIS/GD/GD32G5x3/Source/system_gd32g5x3.c"
$CAT_SYSCALLS
)

# Startup file
set(STARTUP_FILE "\${FW_DIR}/CMSIS/GD/GD32G5x3/Source/GCC/startup_gd32g5x3.S")

# Linker script (GD32G553_xE - 512KB Flash)
set(LINKER_SCRIPT "\${FW_DIR}/CMSIS/GD/GD32G5x3/Source/GCC/Ld/gd32g553_xE_flash.ld")
set(CMAKE_EXE_LINKER_FLAGS "\${CMAKE_EXE_LINKER_FLAGS} -T \${LINKER_SCRIPT}")

# Executable
add_executable(\${PROJECT_NAME}.elf \${SOURCES} \${STARTUP_FILE})

# Post-build: Create hex and bin
add_custom_command(TARGET \${PROJECT_NAME}.elf POST_BUILD
    COMMAND \${CMAKE_OBJCOPY} -O ihex \${PROJECT_NAME}.elf \${PROJECT_NAME}.hex
    COMMAND \${CMAKE_OBJCOPY} -O binary \${PROJECT_NAME}.elf \${PROJECT_NAME}.bin
    COMMAND \${CMAKE_SIZE} \${PROJECT_NAME}.elf
)
EOF

    # 4. Compile
    echo "Compiling $proj_name..."
    cd "${proj}/build"
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake > build.log 2>&1
    if [ $? -eq 0 ]; then
        make -j$(nproc) >> build.log 2>&1
        if [ $? -eq 0 ]; then
            echo "Successfully compiled $proj_name"
            touch ../.compiled_success
        else
            echo "Failed to make $proj_name. Check ${proj}/build/build.log"
            touch ../.compiled_failed
        fi
    else
        echo "Failed to run cmake for $proj_name. Check ${proj}/build/build.log"
        touch ../.compiled_failed
    fi
    cd - > /dev/null
done
