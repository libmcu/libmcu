# libmcu
![Build Status](https://github.com/onkwon/libmcu/workflows/build/badge.svg)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=libmcu_libmcu&metric=security_rating)](https://sonarcloud.io/dashboard?id=libmcu_libmcu)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=libmcu_libmcu&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=libmcu_libmcu)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=libmcu_libmcu&metric=coverage)](https://sonarcloud.io/dashboard?id=libmcu_libmcu)
[![codecov](https://codecov.io/gh/onkwon/libmcu/branch/master/graph/badge.svg?token=KBLNIEKUF4)](https://codecov.io/gh/onkwon/libmcu)

A toolkit for firmware development.

libmcu prioritizes simplicity and minimal code size. Dynamic memory allocation
is avoided whenever possible, and no linker script tweaks are required.

All code is written in standard C99 for maximum portability. Its modular
directory structure, naming conventions, and interface boundaries are designed
to be intuitive and scalable across a wide range of embedded platforms.

Documentation for each module is located in its respective subdirectory.
You can also find usage examples in [examples](examples) and test cases in
[tests](tests/src).

Feedback, suggestions, and contributions are always welcome.

## Project Structure

libmcu is organized into three clear layers:

```plaintext
libmcu/
├── modules/       # Platform-independent logic modules (actor, metrics, pubsub, etc.)
├── interfaces/    # Vendor-neutral HAL abstraction interfaces (UART, GPIO, etc.)
├── ports/         # Platform-specific backend implementations (e.g., STM32 HAL, simulation)
├── project/       # Build integration scripts (e.g., modules.mk, interfaces.mk)
├── tests/         # Unit tests for modules and interfaces
├── examples/      # Integration examples for users
```

### Naming Guidelines

- `modules/` contains **logic modules** that implement platform-agnostic behavior
- `interfaces/` defines **hardware abstraction interfaces (HAL)** that isolate platform-specific details
- `ports/` provides platform-specific implementations of those interfaces (e.g., STM32, ESP-IDF)

This structure clearly separates logic, hardware abstraction, and platform
adaptation, making the system easier to maintain and extend.

All public symbols follow the `lm_` prefix convention to avoid name collisions
and to maintain a consistent global namespace.

## Modules
* [Actor](modules/actor)
* [Application Timer](modules/apptimer)
* [Button](modules/button)
* [Buzzer](modules/buzzer)
* [Cleanup](modules/cleanup)
* [Command Line Interface](modules/cli)
* [Common](modules/common)
* [DFU](modules/dfu)
* [FSM](modules/fsm)
* [Logging](modules/logging)
* [Metrics](modules/metrics)
* [Power Management](modules/pm)
* [PubSub](modules/pubsub)
* [RateLim](modules/ratelim)
* [Retry with exponential backoff](modules/retry)
* [Runner](modules/runner)

## Interfaces
* [ADC](interfaces/adc)
* [BLE](interfaces/ble)
* [Flash](interfaces/flash)
* [GPIO](interfaces/gpio)
* [I2C](interfaces/i2c)
* [KVStore](interfaces/kvstore)
* [PWM](interfaces/pwm)
* [SPI](interfaces/spi)
* [Application Timer](interfaces/apptmr)
* [UART](interfaces/uart)
* [WDT](interfaces/wdt)
* [WiFi](interfaces/wifi)

## Integration Guide

Choose the integration method that matches your platform and build system.

### Zephyr

Add as a [west module](https://docs.zephyrproject.org/latest/develop/modules.html)
in your manifest:

```yaml
# west.yml
manifest:
  projects:
    - name: libmcu
      url: https://github.com/libmcu/libmcu.git
      revision: main
      path: modules/lib/libmcu
```

Enable in `prj.conf`:

```conf
CONFIG_LIBMCU=y
```

All modules are enabled by default. Individual modules can be toggled via
`west build -t menuconfig` under the **libmcu** menu.

### ESP-IDF

Clone or add as a git submodule under your project's `components/` directory:

```bash
cd components
git submodule add https://github.com/libmcu/libmcu.git libmcu
```

No wrapper `CMakeLists.txt` is needed. The root `CMakeLists.txt` auto-detects
the ESP-IDF build environment via `ESP_PLATFORM` and calls
`idf_component_register()` accordingly.

By default, all modules are enabled. To select only specific modules, set
`LIBMCU_MODULES` before the component is registered:

```cmake
# In your project's top-level CMakeLists.txt, before idf_build_process()
set(LIBMCU_MODULES actor logging metrics CACHE STRING "" FORCE)
```

### CMake

```cmake
add_subdirectory(<THIRD_PARTY_DIR>/libmcu)
```

or

```cmake
set(LIBMCU_ROOT <THIRD_PARTY_DIR>/libmcu)
#list(APPEND LIBMCU_MODULES metrics pubsub)
include(${LIBMCU_ROOT}/project/modules.cmake)

#list(APPEND LIBMCU_INTERFACES i2c uart)
include(${LIBMCU_ROOT}/project/interfaces.cmake)

# Add ${LIBMCU_MODULES_SRCS} to your target sources
# Add ${LIBMCU_MODULES_INCS} to your target includes
```

or via [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html):

```cmake
include(FetchContent)
FetchContent_Declare(libmcu
		     GIT_REPOSITORY https://github.com/libmcu/libmcu.git
		     GIT_TAG main
)
FetchContent_MakeAvailable(libmcu)
```

### Make

```make
LIBMCU_ROOT ?= <THIRD_PARTY_DIR>/libmcu
# The commented lines below are optional. All modules and interfaces included
# by default if not specified.
#LIBMCU_MODULES := actor metrics
include $(LIBMCU_ROOT)/project/modules.mk

<SRC_FILES> += $(LIBMCU_MODULES_SRCS)
<INC_PATHS> += $(LIBMCU_MODULES_INCS)

#LIBMCU_INTERFACES := gpio pwm
include $(LIBMCU_ROOT)/project/interfaces.mk

<SRC_FILES> += $(LIBMCU_INTERFACES_SRCS)
<INC_PATHS> += $(LIBMCU_INTERFACES_INCS)
```

## License

MIT License. See [LICENSE](LICENSE) file.
