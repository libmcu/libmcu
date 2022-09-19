cmake_minimum_required(VERSION 3.16)

project(libmcu LANGUAGES C CXX)

set(LIBMCU_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBMCU_MODULES apptimer bitmap button cli common jobqueue logging metrics pubsub retry)
include(projects/modules.cmake)

foreach (header_dir ${LIBMCU_MODULES_INCS})
	file(GLOB_RECURSE header_files ${header_dir}/*.h)
	list(APPEND headers ${header_files})
endforeach()

add_library(${PROJECT_NAME} STATIC ${LIBMCU_MODULES_SRCS})

target_compile_features(${PROJECT_NAME} PUBLIC c_std_99)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${LIBMCU_MODULES_INCS}
	PRIVATE ${METRICS_USER_DIR}
)

install(FILES ${headers} DESTINATION include/${PROJECT_NAME})
install(TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib)
