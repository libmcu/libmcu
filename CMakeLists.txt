cmake_minimum_required(VERSION 3.16)

include(${CMAKE_CURRENT_LIST_DIR}/project/modules.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/project/interfaces.cmake)

if(ESP_PLATFORM)
	idf_component_register(
		SRCS ${LIBMCU_MODULES_SRCS}
		INCLUDE_DIRS ${LIBMCU_MODULES_INCS} ${LIBMCU_INTERFACES_INCS}
		PRIV_INCLUDE_DIRS ${METRICS_USER_DIR}
	)

	target_compile_features(${COMPONENT_LIB} PRIVATE c_std_99)
else()
	project(libmcu LANGUAGES C CXX)

	add_library(${PROJECT_NAME} STATIC
		${LIBMCU_MODULES_SRCS}
		${LIBMCU_INTERFACES_SRCS}
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
