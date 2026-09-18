cmake_minimum_required(VERSION 3.10)

# Project Name
project(MAATS_ColletAgent C ASM)

# Toolchain configuration
set(CMAKE_C_STANDARD 99)

# Core definitions
add_definitions(-DGD32F470)
add_definitions(-DUSE_STDPERIPH_DRIVER)

# MCU Flags: Cortex-M4 with FPU
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MCU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${MCU_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MCU_FLAGS} -Wl,--gc-sections --specs=nano.specs --specs=nosys.specs")

# Paths
set(PROJ_DIR "/home/rainti/project/GD32F4xx_Demo_Suites/GD32470Z_EVAL_Demo_Suites/Projects/29_MY_ColletAgent")
set(FW_DIR "/home/rainti/project/GD32F4xx_Demo_Suites/GD32F4xx_Firmware_Library")
set(UTIL_DIR "/home/rainti/project/GD32F4xx_Demo_Suites/GD32470Z_EVAL_Demo_Suites/Utilities")
# FreeRTOS Path
set(FREERTOS_DIR "${FW_DIR}/../GD32470Z_EVAL_Demo_Suites/Projects/26_ENET/Third_party/FreeRTOS-Kernel-11.2.0")

# Include Directories
include_directories(
    ${PROJ_DIR}
    ${UTIL_DIR}
    ${FW_DIR}/CMSIS
    ${FW_DIR}/CMSIS/GD/GD32F4xx/Include
    ${FW_DIR}/GD32F4xx_standard_peripheral/Include
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F
)

# Source Files
file(GLOB SOURCES
    "${PROJ_DIR}/*.c"
    "${UTIL_DIR}/*.c"
    "${FW_DIR}/GD32F4xx_standard_peripheral/Source/*.c"
    "${FW_DIR}/CMSIS/GD/GD32F4xx/Source/system_gd32f4xx.c"
)

# FreeRTOS Source
file(GLOB FREERTOS_SOURCES
    "${FREERTOS_DIR}/*.c"
    "${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c"
    "${FREERTOS_DIR}/portable/MemMang/heap_4.c"
)

# Startup file
set(STARTUP_FILE "${FW_DIR}/CMSIS/GD/GD32F4xx/Source/GCC/startup_gd32f450_470.S")

# Linker script
set(LINKER_SCRIPT "${FW_DIR}/CMSIS/GD/GD32F4xx/Source/GCC/Ld/gd32f470xI_flash.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT}")

# Executable
add_executable(${PROJECT_NAME}.elf ${SOURCES} ${FREERTOS_SOURCES} ${STARTUP_FILE})

# Post-build: Create hex and bin
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
)
