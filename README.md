# libmcu
![Build Status](https://github.com/onkwon/libmcu/workflows/build/badge.svg)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=security_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=reliability_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=coverage)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![codecov](https://codecov.io/gh/onkwon/libmcu/branch/master/graph/badge.svg?token=KBLNIEKUF4)](https://codecov.io/gh/onkwon/libmcu)

## Overview
A toolkit for firmware development.

Simplicity and code size are considered first while trying to avoid dynamic
allocation as much as possible.

The documentation of each modules are under the subdirectories. Some usage
examples can also be found under [examples](examples) and [test cases](tests/src).

Any feedback would be appreciated.

## Modules
* [apptimer](modules/apptimer)
* [bitmap](modules/bitmap)
* [button](modules/button)
* [cli](modules/cli)
* [common](modules/common)
* [jobqueue](modules/jobqueue)
* [logging](modules/logging)
* [metrics](modules/metrics)
* [pubsub](modules/pubsub)
* [pubsub_tiny](modules/pubsub_tiny)
* [retry](modules/retry)
* [timext](modules/timext)

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
LIBMCU_MODULES := logging pubsub metrics
include $(LIBMCU_ROOT)/projects/modules.mk

<SRC_FILES> += $(LIBMCU_MODULES_SRCS)
<INC_PATHS> += $(LIBMCU_MODULES_INCS)
```

#### CMake

```cmake
set(LIBMCU_ROOT <THIRD_PARTY_DIR>/libmcu)
list(APPEND LIBMCU_MODULES logging pubsub metrics)
include(${LIBMCU_ROOT}/projects/modules.cmake)

# Add ${LIBMCU_MODULES_SRCS} to your target sources
# Add ${LIBMCU_MODULES_INCS} to your target includes
```
