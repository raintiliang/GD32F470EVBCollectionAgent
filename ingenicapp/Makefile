# Multi-platform IP Camera Application Makefile
#
# Build Targets:
#   make PLATFORM=cvitek      # Build for CVITEK platform (default)
#   make PLATFORM=ingenic     # Build for Ingenic T31 platform
#   make PLATFORM=rk          # Build for Rockchip platform
#   make clean                # Clean build artifacts
#   make help                 # Show help message

# Platform selection (default: cvitek)
PLATFORM ?= cvitek

# Project directories
ROOT_DIR := $(shell pwd)
HAL_DIR := $(ROOT_DIR)/hal
MODULES_DIR := $(ROOT_DIR)/modules
OUTPUT_DIR := $(ROOT_DIR)/out
BUILD_DIR := $(OUTPUT_DIR)/obj

# Compiler and flags
CROSS_COMPILE ?= 
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

# Common compiler flags
CFLAGS_COMMON := -Wall -Werror -Wextra -Wno-unused-parameter
CFLAGS_COMMON += -O2 -g -fPIC -fstack-protector-strong
CFLAGS_COMMON += -I$(HAL_DIR)/include
CFLAGS_COMMON += -I$(MODULES_DIR)/common/include
CFLAGS_COMMON += -I$(MODULES_DIR)/video/include
CFLAGS_COMMON += -I$(MODULES_DIR)/audio/include
CFLAGS_COMMON += -I$(MODULES_DIR)/ai/include
CFLAGS_COMMON += -I$(MODULES_DIR)/osd/include
CFLAGS_COMMON += -I$(MODULES_DIR)/network/include
CFLAGS_COMMON += -I$(MODULES_DIR)/peripheral/include

# Linker flags
LDFLAGS_COMMON := -lpthread -lm -ldl -lrt

# Platform-specific configuration
ifeq ($(PLATFORM),cvitek)
	# CVITEK platform
	CFLAGS_PLATFORM := -DPLATFORM_CVITEK -DARCH_CV181X -D__CV181X__
	LDFLAGS_PLATFORM := -lcvi_vi -lcvi_venc -lcvi_vpss -lcvi_audio -lcviai
	PLATFORM_SUFFIX := cvitek
	
	# CVITEK SDK paths (adjust as needed)
	CVITEK_SDK_PATH ?= /data/cvitekbsp
	CFLAGS_COMMON += -I$(CVITEK_SDK_PATH)/linux_5.10/build/cv1810c_c101_wifi_sc2331_8m/riscv/usr/include
	CFLAGS_COMMON += -I$(CVITEK_SDK_PATH)/middleware/v2/include
	LDFLAGS_COMMON += -L$(CVITEK_SDK_PATH)/install/soc_cv1810c_c101_wifi_sc2331_8m/tpu_musl_riscv64/cvitek_ai_sdk/lib
	
else ifeq ($(PLATFORM),ingenic)
	# Ingenic T31 platform
	CFLAGS_PLATFORM := -DPLATFORM_INGENIC
	LDFLAGS_PLATFORM := -limp_framesource -limp_encoder -limp_audio -limp_isp -limp_osd
	PLATFORM_SUFFIX := ingenic
	
	# Ingenic SDK paths
	INGENIC_SDK_PATH ?= /data/Ingenic-SDK-T31-1.1.6-20221013
	CFLAGS_COMMON += -I$(INGENIC_SDK_PATH)/sdk/5.4.0/include
	CFLAGS_COMMON += -I$(INGENIC_SDK_PATH)/sdk/5.4.0/samples/include
	LDFLAGS_COMMON += -L$(INGENIC_SDK_PATH)/sdk/5.4.0/libs
	
else ifeq ($(PLATFORM),rk)
	# Rockchip platform (placeholder)
	CFLAGS_PLATFORM := -DPLATFORM_RK
	LDFLAGS_PLATFORM := -lrk_vi -lrk_venc -lrk_audio
	PLATFORM_SUFFIX := rk
	
	# TODO: Add Rockchip SDK paths
else
$(error Unknown platform: $(PLATFORM). Valid options: cvitek, ingenic, rk)
endif

# Combine flags
CFLAGS := $(CFLAGS_COMMON) $(CFLAGS_PLATFORM)
LDFLAGS := $(LDFLAGS_COMMON) $(LDFLAGS_PLATFORM)

# Source files
# Common HAL sources
HAL_SRCS := \
	$(HAL_DIR)/src/app_hal_common.c \
	$(HAL_DIR)/src/hal_osd.c \
	$(HAL_DIR)/src/hal_peripheral.c \
	$(HAL_DIR)/src/hal_video.c

# Platform-specific HAL source
HAL_SRCS += $(HAL_DIR)/src/platform_$(PLATFORM).c

# Module sources (example - these would come from the actual socapp modules)
# TODO: Add actual module source files
MODULE_SRCS := \
	$(MODULES_DIR)/video/src/app_ipcam_vi.c \
	$(MODULES_DIR)/video/src/app_ipcam_venc.c \
	$(MODULES_DIR)/video/src/app_ipcam_vpss.c \
	$(MODULES_DIR)/video/src/app_ipcam_rtsp.c \
	$(MODULES_DIR)/video/src/app_ipcam_sys.c \
	$(MODULES_DIR)/audio/src/app_ipcam_audio.c \
	$(MODULES_DIR)/ai/src/app_ipcam_ai.c \
	$(MODULES_DIR)/osd/src/app_ipcam_osd.c \
	$(MODULES_DIR)/common/src/app_ipcam_comm.c \
	$(MODULES_DIR)/common/src/app_ipcam_os.c

# Application entry point
APP_SRCS := $(MODULES_DIR)/solutions/ipcamera/src/app_ipcam_main.c

# Object files
HAL_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(HAL_SRCS))
MODULE_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(MODULE_SRCS))
APP_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(APP_SRCS))

ALL_OBJS := $(HAL_OBJS) $(MODULE_OBJS) $(APP_OBJS)

# Output binary
TARGET := $(OUTPUT_DIR)/ipcamera_$(PLATFORM_SUFFIX)

# Default target
.PHONY: all
all: $(TARGET)

# Build the target executable
$(TARGET): $(ALL_OBJS)
	@echo "  LD      $@"
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)
	@echo "  STRIP   $@"
	$(STRIP) --strip-unneeded $@
	@echo "Build complete: $@"

# Compile C source files
$(BUILD_DIR)/%.o: %.c
	@echo "  CC      $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create platform-specific HAL source from template
$(HAL_DIR)/src/platform_$(PLATFORM).c: $(HAL_DIR)/src/platform_template.c
	@echo "Creating platform-specific HAL source for $(PLATFORM)"
	@cp $< $@
	@sed -i 's/PLATFORM_TEMPLATE/$(PLATFORM)/g' $@

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OUTPUT_DIR)
	rm -f $(HAL_DIR)/src/platform_*.c

# Show help message
.PHONY: help
help:
	@echo "Multi-platform IP Camera Application Build System"
	@echo ""
	@echo "Usage:"
	@echo "  make [PLATFORM=platform] [target]"
	@echo ""
	@echo "Platforms:"
	@echo "  cvitek      Build for CVITEK platform (default)"
	@echo "  ingenic     Build for Ingenic T31 platform"
	@echo "  rk          Build for Rockchip platform"
	@echo ""
	@echo "Targets:"
	@echo "  all         Build the application (default)"
	@echo "  clean       Clean all build artifacts"
	@echo "  help        Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                      # Build for CVITEK"
	@echo "  make PLATFORM=ingenic     # Build for Ingenic T31"
	@echo "  make clean                # Clean build directory"
	@echo ""

# Create directory structure
.PHONY: init
init:
	@echo "Initializing project structure..."
	@mkdir -p $(HAL_DIR)/include
	@mkdir -p $(HAL_DIR)/src
	@mkdir -p $(MODULES_DIR)
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(BUILD_DIR)
	@echo "Project structure created."

# Dependencies for header files
$(ALL_OBJS): $(HAL_DIR)/include/app_hal.h

# Print build configuration
.PHONY: config
config:
	@echo "Build Configuration:"
	@echo "  Platform:      $(PLATFORM)"
	@echo "  Platform suffix: $(PLATFORM_SUFFIX)"
	@echo "  CC:            $(CC)"
	@echo "  CFLAGS:        $(CFLAGS)"
	@echo "  LDFLAGS:       $(LDFLAGS)"
	@echo "  Target:        $(TARGET)"
	@echo "  Output dir:    $(OUTPUT_DIR)"
	@echo "  Build dir:     $(BUILD_DIR)"

# Test compilation without linking
.PHONY: check
check:
	@echo "Checking source files..."
	$(CC) $(CFLAGS) -fsyntax-only $(HAL_SRCS) 2>/dev/null && echo "HAL sources: OK" || echo "HAL sources: ERROR"
	@echo "Done."

# Create source file list for IDE
.PHONY: filelist
filelist:
	@echo "Generating file list..."
	@echo "# Source files for $(PLATFORM) platform" > files.txt
	@echo "" >> files.txt
	@echo "## HAL Sources" >> files.txt
	@for f in $(HAL_SRCS); do echo "$$f" >> files.txt; done
	@echo "" >> files.txt
	@echo "## Module Sources" >> files.txt
	@for f in $(MODULE_SRCS); do echo "$$f" >> files.txt; done
	@echo "" >> files.txt
	@echo "## Application Sources" >> files.txt
	@for f in $(APP_SRCS); do echo "$$f" >> files.txt; done
	@echo "" >> files.txt
	@echo "## Include Directories" >> files.txt
	@echo $(CFLAGS) | tr ' ' '\n' | grep '^-I' | sed 's/^-I//' >> files.txt
	@echo "File list saved to files.txt"

# Install to target directory
INSTALL_DIR ?= /usr/local/bin
.PHONY: install
install: $(TARGET)
	@echo "Installing to $(INSTALL_DIR)..."
	install -m 755 $(TARGET) $(INSTALL_DIR)/ipcamera
	@echo "Installation complete."

# Uninstall from target directory
.PHONY: uninstall
uninstall:
	@echo "Uninstalling from $(INSTALL_DIR)..."
	rm -f $(INSTALL_DIR)/ipcamera
	@echo "Uninstallation complete."

# Run static analysis (if available)
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --suppress=missingIncludeSystem $(CFLAGS) $(HAL_SRCS) 2>&1 | grep -v "^Checking"; \
	else \
		echo "cppcheck not found, skipping static analysis"; \
	fi
	@echo "Analysis complete."

# Generate tags for code navigation
.PHONY: tags
tags:
	@echo "Generating tags..."
	@ctags -R --c-kinds=+p --fields=+iaS --extra=+q .
	@echo "Tags generated."

# Size report
.PHONY: size
size: $(TARGET)
	@echo "Binary size report:"
	@size $(TARGET) | tail -1 | awk '{printf "  Text: %d bytes\n  Data: %d bytes\n  Bss: %d bytes\n  Total: %d bytes\n", $$1, $$2, $$3, $$1+$$2+$$3}'