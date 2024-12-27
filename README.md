# libmcu
![Build Status](https://github.com/onkwon/libmcu/workflows/build/badge.svg)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=security_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=coverage)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![codecov](https://codecov.io/gh/onkwon/libmcu/branch/master/graph/badge.svg?token=KBLNIEKUF4)](https://codecov.io/gh/onkwon/libmcu)

## Overview
A toolkit for firmware development.

Simplicity and code size are considered first while trying to avoid dynamic
allocation as much as possible. No linker script tweak required.

The documentation of each modules are under the subdirectories. Some usage
examples can also be found under [examples](examples) and [test cases](tests/src).

Any feedback would be appreciated.

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
The library can be intergrated in your project as a [git
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
