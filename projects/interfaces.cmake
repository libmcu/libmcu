# SPDX-License-Identifier: MIT

if (NOT DEFINED LIBMCU_INTERFACES)
	set(LIBMCU_INTERFACES adc flash gpio i2c kvstore l4 pwm spi timer uart wifi)
endif()

foreach(iface ${LIBMCU_INTERFACES})
	file(GLOB LIBMCU_${iface}_INCS
		${CMAKE_CURRENT_LIST_DIR}/../interfaces/${iface}/include)
	list(APPEND LIBMCU_INTERFACES_INCS_LIST ${LIBMCU_${iface}_INCS})
endforeach()

set(LIBMCU_INTERFACES_INCS ${LIBMCU_INTERFACES_INCS_LIST})
