# SPDX-License-Identifier: MIT

cmake_policy(SET CMP0057 NEW)

if (NOT "common" IN_LIST LIBMCU_MODULES)
	list(APPEND LIBMCU_MODULES common)
endif()

foreach(module ${LIBMCU_MODULES})
	file(GLOB LIBMCU_${module}_SRCS
		${LIBMCU_ROOT}/modules/${module}/src/*.c)
	file(GLOB LIBMCU_${module}_INCS
		${LIBMCU_ROOT}/modules/${module}/include)
	list(APPEND LIBMCU_MODULES_SRCS_LIST ${LIBMCU_${module}_SRCS})
	list(APPEND LIBMCU_MODULES_INCS_LIST ${LIBMCU_${module}_INCS})
endforeach()

set(LIBMCU_MODULES_SRCS ${LIBMCU_MODULES_SRCS_LIST})
set(LIBMCU_MODULES_INCS ${LIBMCU_MODULES_INCS_LIST})
