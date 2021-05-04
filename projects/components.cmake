if (NOT "common" IN_LIST LIBMCU_COMPONENTS)
	list(APPEND LIBMCU_COMPONENTS common)
endif()

foreach(component ${LIBMCU_COMPONENTS})
	file(GLOB LIBMCU_${component}_SRCS
		${LIBMCU_ROOT}/components/${component}/src/*.c)
	file(GLOB LIBMCU_${component}_INCS
		${LIBMCU_ROOT}/components/${component}/include)
	list(APPEND LIBMCU_COMPONENTS_SRCS_LIST ${LIBMCU_${component}_SRCS})
	list(APPEND LIBMCU_COMPONENTS_INCS_LIST ${LIBMCU_${component}_INCS})
endforeach()

list(APPEND LIBMCU_COMPONENTS_INCS_LIST
	${LIBMCU_ROOT}/components/common/include/libmcu/posix)

set(LIBMCU_COMPONENTS_SRCS ${LIBMCU_COMPONENTS_SRCS_LIST})
set(LIBMCU_COMPONENTS_INCS ${LIBMCU_COMPONENTS_INCS_LIST})
