# libmcu
![Build Status](https://github.com/onkwon/libmcu/workflows/build/badge.svg)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=libmcu_libmcu&metric=security_rating)](https://sonarcloud.io/dashboard?id=libmcu_libmcu)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=libmcu_libmcu&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=libmcu_libmcu)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=libmcu_libmcu&metric=coverage)](https://sonarcloud.io/dashboard?id=libmcu_libmcu)
[![codecov](https://codecov.io/gh/onkwon/libmcu/branch/master/graph/badge.svg?token=KBLNIEKUF4)](https://codecov.io/gh/onkwon/libmcu)

A toolkit for firmware development.

libmcu prioritizes simplicity and minimal code size. Dynamic memory allocation
is avoided whenever possible, and no linker script tweaks are required.

Documentation for each module is available in its corresponding subdirectory.
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
├── tests/         # Unit tests for modules and interfaces
├── examples/      # Integration examples for users
├── config/        # Build integration scripts (e.g., modules.mk, interfaces.mk)
```

### Naming Philosophy

- `modules/` contains **logic modules**, which implement platform-agnostic behavior
- `interfaces/` defines **hardware abstraction interfaces (HAL)** that isolate platform details
- `ports/` implements those interfaces for specific hardware platforms (e.g., STM32, mock)

This structure makes the separation of concerns between logic, hardware abstraction, and platform adaptation clear and extensible.

All public symbols follow the `lm_` prefix convention to avoid symbol collisions and maintain namespace clarity.

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
The library can be integrated into your project as a [git
submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules), using [CMake
FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html), or
[downloading](https://github.com/libmcu/libmcu/archive/refs/heads/main.zip)
manually.

### git submodule
#### Include `libmcu` into your project

```bash
$ cd ${YOUR_PROJECT_DIR}
$ git submodule add https://github.com/libmcu/libmcu.git ${THIRD_PARTY_DIR}/libmcu
```

#### Add `libmcu` into your build system
##### Make

```make
LIBMCU_ROOT ?= <THIRD_PARTY_DIR>/libmcu
# The commented lines below are optional. All modules and interfaces included
# by default if not specified.
#LIBMCU_MODULES := actor metrics
include $(LIBMCU_ROOT)/projects/modules.mk

<SRC_FILES> += $(LIBMCU_MODULES_SRCS)
<INC_PATHS> += $(LIBMCU_MODULES_INCS)

#LIBMCU_INTERFACES := gpio pwm
include $(LIBMCU_ROOT)/projects/interfaces.mk

<SRC_FILES> += $(LIBMCU_INTERFACES_SRCS)
<INC_PATHS> += $(LIBMCU_INTERFACES_INCS)
```

##### CMake

```cmake
add_subdirectory(<THIRD_PARTY_DIR>/libmcu)
```

or

```cmake
set(LIBMCU_ROOT <THIRD_PARTY_DIR>/libmcu)
#list(APPEND LIBMCU_MODULES metrics pubsub)
include(${LIBMCU_ROOT}/projects/modules.cmake)

#list(APPEND LIBMCU_INTERFACES i2c uart)
include(${LIBMCU_ROOT}/projects/interfaces.cmake)

# Add ${LIBMCU_MODULES_SRCS} to your target sources
# Add ${LIBMCU_MODULES_INCS} to your target includes
```

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(libmcu
		     GIT_REPOSITORY https://github.com/libmcu/libmcu.git
		     GIT_TAG main
)
FetchContent_MakeAvailable(libmcu)
```

## License

MIT License. See `LICENSE` file.

## OSS Notes

libmcu is an open-source project (OSS), licensed under MIT, and welcomes
community contributions. All code is written in standard C99 for maximum
portability. Its modular directory structure, naming conventions, and interface
boundaries are designed to be intuitive and scalable across a wide range of
embedded platforms.
