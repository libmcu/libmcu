# SPDX-License-Identifier: MIT

if (NOT DEFINED LIBMCU_INTERFACES)
	set(LIBMCU_INTERFACES adc gpio i2c l4 pwm spi timer uart wifi)
endif()

foreach(iface ${LIBMCU_INTERFACES})
	file(GLOB LIBMCU_${iface}_SRCS
		${CMAKE_CURRENT_LIST_DIR}/../interfaces/${iface}/src/*.c)
	file(GLOB LIBMCU_${iface}_INCS
		${CMAKE_CURRENT_LIST_DIR}/../interfaces/${iface}/include)
	list(APPEND LIBMCU_INTERFACES_SRCS_LIST ${LIBMCU_${iface}_SRCS})
	list(APPEND LIBMCU_INTERFACES_INCS_LIST ${LIBMCU_${iface}_INCS})
endforeach()

set(LIBMCU_INTERFACES_SRCS ${LIBMCU_INTERFACES_SRCS_LIST})
set(LIBMCU_INTERFACES_INCS ${LIBMCU_INTERFACES_INCS_LIST})
