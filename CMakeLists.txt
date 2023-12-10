cmake_minimum_required(VERSION 3.16)

project(libmcu LANGUAGES C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/projects/modules.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/projects/interfaces.cmake)

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
