# SPDX-License-Identifier: MIT

VERSION ?= $(shell git describe --long --tags --dirty --always)
VERSION_STR := $(subst -, , $(VERSION))
VERSION_NUMBER := $(strip $(word 1, $(VERSION_STR)))
VERSION_NUMBER_LIST := $(subst ., , $(subst v,, $(VERSION_NUMBER)))
VERSION_MAJOR := $(strip $(word 1, $(VERSION_NUMBER_LIST)))
VERSION_MINOR := $(strip $(word 2, $(VERSION_NUMBER_LIST)))
VERSION_PATCH := $(word 2, $(VERSION_STR))
ifneq ($(word 3, $(VERSION_NUMBER_LIST)),)
VERSION_PATCH := $(shell echo $(VERSION_PATCH) + \
		 $(strip $(word 3, $(VERSION_NUMBER_LIST))) | bc)
endif

VERSION_TAG := v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

export VERSION
export VERSION_TAG
