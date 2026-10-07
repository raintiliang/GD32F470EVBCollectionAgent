# Hardware Abstraction Layer (HAL) for IP Camera Applications

## Overview

This Hardware Abstraction Layer (HAL) provides a unified interface for IP camera applications across different hardware platforms (CVITEK, Ingenic T31, Rockchip). It enables porting of existing IP camera applications like `socapp` to new platforms with minimal code changes.

## Project Structure

```
ingenicapp/
├── hal/                    # Hardware Abstraction Layer
│   ├── include/           # Public header files
│   │   ├── hal.h          # Main HAL header
│   │   ├── hal_common.h   # Common types and macros
│   │   ├── hal_system.h   # System abstraction
│   │   ├── hal_video.h    # Video abstraction (VI, VPSS, VENC)
│   │   ├── hal_audio.h    # Audio abstraction
│   │   ├── hal_ai.h       # AI/ML abstraction
│   │   ├── hal_osd.h      # OSD (On-Screen Display) abstraction
│   │   ├── hal_network.h  # Network abstraction
│   │   └── hal_peripheral.h # Peripheral abstraction
│   └── src/               # Source code
│       ├── hal.c          # Main HAL implementation
│       ├── platform/      # Platform-specific implementations
│       │   ├── cvitek/    # CVITEK implementation
│       │   ├── ingenic/   # Ingenic T31 implementation
│       │   └── rk/        # Rockchip implementation
│       └── modules/       # Module implementations (optional)
├── test/                  # Test programs
│   └── test_hal.c        # HAL test suite
├── examples/              # Example applications
├── Makefile              # Build system
└── README.md            # This file
```

## Design Philosophy

### 1. Unified Interface
- Single API for all supported platforms
- Platform-specific details hidden behind the interface
- Consistent error handling and logging

### 2. Modular Design
- Independent modules for different functions (video, audio, AI, etc.)
- Clear separation between abstraction and implementation
- Easy to add new modules or extend existing ones

### 3. Extensibility
- Support for future hardware platforms
- Ability to add new features without breaking existing API
- Forward-compatible design

### 4. Performance
- Minimal abstraction overhead
- Direct hardware access where needed
- Efficient memory management

## API Overview

### System Module (`hal_system.h`)
- System initialization and configuration
- Memory buffer management
- Power management
- Temperature monitoring
- Time and sleep functions

### Video Module (`hal_video.h`)
- Video input (VI) configuration and capture
- Video processing (VPSS) operations
- Video encoding (VENC) for H.264/H.265/JPEG
- ISP control (AE, AWB, AF)
- Sensor management

### Audio Module (`hal_audio.h`)
- Audio capture and playback
- Audio encoding/decoding (AAC, G.711, PCM)
- Audio processing (AEC, NS, AGC)
- Volume control

### AI Module (`hal_ai.h`)
- AI model loading and management
- Inference execution (face detection, person detection, etc.)
- Feature extraction and comparison
- Hardware acceleration support

### OSD Module (`hal_osd.h`)
- On-screen display regions
- Text, graphics, and bitmap rendering
- Time display
- Animation support
- Transparency and blending

### Network Module (`hal_network.h`)
- Network interface configuration
- Wireless network support
- Network services (HTTP, RTSP, ONVIF, P2P)
- QoS and firewall
- Socket abstraction

### Peripheral Module (`hal_peripheral.h`)
- GPIO control
- I2C/SPI/UART communication
- PWM and ADC
- IR-CUT and LED control
- Motor control (PTZ)
- Temperature sensors
- Watchdog timer

## Porting Strategy

### Step 1: Analyze Existing Code
- Identify platform-specific API calls in `socapp`
- Map CVITEK APIs to HAL interfaces
- Identify third-party libraries that need wrapping

### Step 2: Implement Platform Layer
- Create platform-specific implementation for target platform (Ingenic T31)
- Implement all HAL interfaces using native SDK (IMP API)
- Test each module independently

### Step 3: Integrate with Application
- Replace CVITEK API calls with HAL calls in `socapp`
- Update build system to link with HAL library
- Test application functionality

### Step 4: Optimize and Validate
- Performance tuning for target platform
- Memory usage optimization
- Stress testing and validation

## Build Instructions

### Prerequisites
- GCC toolchain for target platform
- Platform SDK (CVITEK, Ingenic, or Rockchip)
- Standard C library

### Building for Ingenic T31
```bash
# Set platform
export PLATFORM=ingenic

# Build HAL library
make clean
make

# Run tests
make test
./test_hal

# Install (optional)
sudo make install
```

### Building for Other Platforms
```bash
# For CVITEK
make PLATFORM=cvitek

# For Rockchip
make PLATFORM=rk
```

## Usage Example

```c
#include "hal.h"

int main() {
    // Initialize HAL
    if (hal_init_all() != HAL_OK) {
        printf("HAL initialization failed\n");
        return -1;
    }
    
    // Configure system
    hal_global_config_t config = {0};
    // ... set configuration parameters
    hal_configure(&config);
    
    // Create video input device
    hal_vi_config_t vi_config = {0};
    vi_config.width = 1920;
    vi_config.height = 1080;
    vi_config.fps = 30;
    
    uint32_t vi_dev_id;
    if (hal_vi_create_device(&vi_config, &vi_dev_id) != HAL_OK) {
        printf("Failed to create video input device\n");
        return -1;
    }
    
    // Start video capture
    hal_vi_start_capture(vi_dev_id);
    
    // Main loop
    for (int i = 0; i < 100; i++) {
        hal_frame_t frame;
        if (hal_vi_get_frame(vi_dev_id, &frame, 1000) == HAL_OK) {
            // Process frame
            printf("Got frame %d\n", i);
            hal_vi_release_frame(vi_dev_id, &frame);
        }
    }
    
    // Cleanup
    hal_vi_stop_capture(vi_dev_id);
    hal_vi_destroy_device(vi_dev_id);
    hal_deinit_all();
    
    return 0;
}
```

## Platform-Specific Notes

### Ingenic T31
- Uses IMP (Ingenic Media Platform) API
- Hardware video encoding support (H.264/H.265)
- Limited AI acceleration (may need software implementation)
- Reference: Ingenic T31 SDK documentation

### CVITEK
- Uses CVI (CVITEK) API family (cvi_vi.h, cvi_venc.h, etc.)
- Hardware AI acceleration via CVI_AI SDK
- Reference: CVITEK BSP documentation

### Rockchip
- Uses RK (Rockchip) multimedia API
- Hardware video encoding via mpp (Media Process Platform)
- AI acceleration via RKNN (Rockchip Neural Network)
- Reference: Rockchip SDK documentation

## Testing

### Unit Tests
```bash
make test
./test_hal
```

### Integration Tests
- Test with actual camera sensor
- Test network streaming (RTSP/ONVIF)
- Test AI inference with sample models
- Stress test with long-running operation

## Contributing

1. Fork the repository
2. Create a feature branch
3. Implement changes
4. Add tests
5. Submit pull request

## License

[Specify license here]

## Acknowledgments

- Based on analysis of `socapp` from Ingenic SDK
- Inspired by multi-platform abstraction patterns
- Designed for easy porting to new hardware platforms