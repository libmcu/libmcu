# libmcu
![Build Status](https://github.com/onkwon/libmcu/workflows/build/badge.svg)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=alert_status)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=onkwon_libmcu&metric=security_rating)](https://sonarcloud.io/dashboard?id=onkwon_libmcu)
[![codecov](https://codecov.io/gh/onkwon/libmcu/branch/master/graph/badge.svg?token=KBLNIEKUF4)](https://codecov.io/gh/onkwon/libmcu)

## Overview
A toolkit for firmware development.

Some of components have dependency on [libmcu/logging](#logging). You can get
rid of the dependency by defining an empty `logging_save()` in case you're not
using it. Please refer to [tests/stubs/logging.c](tests/stubs/logging.c).

The documentation of each components are under the subdirectories. Some usage
examples can also be found under [examples](examples).

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
* [queue](components/queue)
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
