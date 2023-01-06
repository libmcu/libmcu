cmake_minimum_required(VERSION 3.16)

project(libmcu LANGUAGES C CXX)

set(LIBMCU_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBMCU_MODULES ao apptimer bitmap button cli common jobqueue logging metrics pubsub retry)
include(${LIBMCU_ROOT}/projects/modules.cmake)

add_library(${PROJECT_NAME} STATIC ${LIBMCU_MODULES_SRCS})

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${LIBMCU_MODULES_INCS}
	PRIVATE ${METRICS_USER_DIR}
)
