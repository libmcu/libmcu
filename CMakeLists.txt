cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/project/modules.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/project/interfaces.cmake)

set(LIBMCU_PORT_SRCS)
set(LIBMCU_RATELIM_PORT posix CACHE STRING "ratelim time source port for non-platform CMake builds")
set_property(CACHE LIBMCU_RATELIM_PORT PROPERTY STRINGS posix stubs)
list(FIND LIBMCU_MODULES ratelim LIBMCU_RATELIM_INDEX)

if(COMMAND idf_component_register)
	set(LIBMCU_PUBLIC_INCS)
	if(NOT LIBMCU_RATELIM_INDEX EQUAL -1)
		list(APPEND LIBMCU_PORT_SRCS
			${CMAKE_CURRENT_LIST_DIR}/ports/esp-idf/ratelim.c)
	endif()

	if(EXISTS "${PROJECT_DIR}/main/include")
		list(APPEND LIBMCU_PUBLIC_INCS "${PROJECT_DIR}/main/include")
	endif()

	if(METRICS_USER_DIR)
		list(APPEND LIBMCU_PUBLIC_INCS "${METRICS_USER_DIR}")
	endif()

	idf_component_register(
		SRCS ${LIBMCU_MODULES_SRCS} ${LIBMCU_PORT_SRCS}
		INCLUDE_DIRS ${LIBMCU_MODULES_INCS} ${LIBMCU_INTERFACES_INCS} ${LIBMCU_PUBLIC_INCS}
	)

	target_compile_features(${COMPONENT_LIB} PRIVATE c_std_99)
else()
	project(libmcu LANGUAGES C CXX)
	if(NOT LIBMCU_RATELIM_INDEX EQUAL -1)
		if(NOT LIBMCU_RATELIM_PORT MATCHES "^(posix|stubs)$")
			message(FATAL_ERROR "Unsupported LIBMCU_RATELIM_PORT: ${LIBMCU_RATELIM_PORT}")
		endif()
		list(APPEND LIBMCU_PORT_SRCS
			${CMAKE_CURRENT_LIST_DIR}/ports/${LIBMCU_RATELIM_PORT}/ratelim.c)
	endif()

	add_library(${PROJECT_NAME} STATIC
		${LIBMCU_MODULES_SRCS}
		${LIBMCU_INTERFACES_SRCS}
		${LIBMCU_PORT_SRCS}
	)

	target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

	target_include_directories(${PROJECT_NAME}
		PUBLIC
			${LIBMCU_MODULES_INCS}
			${LIBMCU_INTERFACES_INCS}
		PRIVATE
			${METRICS_USER_DIR}
	)
endif()
