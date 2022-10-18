# SPDX-License-Identifier: MIT

VERSION ?= $(shell git describe --long --tags --dirty --always)
VERSION_STR := $(subst -, , $(VERSION))
VERSION_TAG_ORG := $(strip $(word 1, $(VERSION_STR)))
VERSION_PATCH := $(word 2, $(VERSION_STR))
VERSION_TAG_LIST := $(subst ., , $(subst v,, $(VERSION_TAG_ORG)))
VERSION_MAJOR := $(strip $(word 1, $(VERSION_TAG_LIST)))
VERSION_MINOR := $(strip $(word 2, $(VERSION_TAG_LIST)))
ifeq ($(VERSION_PATCH),)
VERSION_PATCH := $(strip $(word 3, $(VERSION_TAG_LIST)))
endif

VERSION_TAG := v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

export VERSION
export VERSION_TAG
