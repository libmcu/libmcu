# libmcu
![Build Status](https://github.com/onkwon/libmcu/workflows/build/badge.svg)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=security_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=coverage)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![codecov](https://codecov.io/gh/onkwon/libmcu/branch/master/graph/badge.svg?token=KBLNIEKUF4)](https://codecov.io/gh/onkwon/libmcu)

## Overview
A toolkit for firmware development.

The documentation of each components are under the subdirectories. Some usage
examples can also be found under [examples](examples) and [test cases](tests/src).

## Components
* [apptimer](components/apptimer)
* [bitmap](components/bitmap)
* [button](components/button)
* [common](components/common)
* [jobqueue](components/jobqueue)
* [logging](components/logging)
* [metrics](components/metrics)
* [mode](components/mode)
* [pubsub](components/pubsub)
* [pubsub_tiny](components/pubsub_tiny)
* [retry](components/retry)
* [shell](components/shell)
* [timext](components/timext)

## Integration Guide
### Include `libmcu` into your project

```bash
$ cd ${YOUR_PROJECT_DIR}
$ git submodule add https://github.com/onkwon/libmcu.git ${THIRD_PARTY_DIR}/libmcu
```

### Add `libmcu` into your build system
#### Make

```make
LIBMCU_ROOT ?= <THIRD_PARTY_DIR>/libmcu
LIBMCU_COMPONENTS := logging pubsub metrics
include $(LIBMCU_ROOT)/projects/components.mk

<SRC_FILES> += $(LIBMCU_COMPONENTS_SRCS)
<INC_PATHS> += $(LIBMCU_COMPONENTS_INCS)
```

#### CMake

```cmake
set(LIBMCU_ROOT <THIRD_PARTY_DIR>/libmcu)
list(APPEND LIBMCU_COMPONENTS logging pubsub metrics)
include(${LIBMCU_ROOT}/projects/components.cmake)

# Add ${LIBMCU_COMPONENTS_SRCS} to your target sources
# Add ${LIBMCU_COMPONENTS_INCS} to your target includes
```
