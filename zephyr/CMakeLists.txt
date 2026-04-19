# SPDX-License-Identifier: MIT

if(CONFIG_LIBMCU)
  set(LIBMCU_METRICS_USER_INCS)

  set(LIBMCU_MODULES common)

  foreach(mod
      actor ao apptimer bitmap button buzzer cleanup cli
      dfu fsm jobqueue logging metrics pm pubsub ratelim retry runner)
    string(TOUPPER ${mod} MOD_UPPER)
    if(CONFIG_LIBMCU_${MOD_UPPER})
      list(APPEND LIBMCU_MODULES ${mod})
    endif()
  endforeach()

  include(${CMAKE_CURRENT_LIST_DIR}/../project/modules.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/../project/interfaces.cmake)

  zephyr_library()
  zephyr_library_sources(${LIBMCU_MODULES_SRCS})
  zephyr_include_directories(${LIBMCU_MODULES_INCS} ${LIBMCU_INTERFACES_INCS})

  if(EXISTS ${APPLICATION_SOURCE_DIR}/include)
    list(APPEND LIBMCU_METRICS_USER_INCS ${APPLICATION_SOURCE_DIR}/include)
  endif()

  if(METRICS_USER_DIR)
    list(APPEND LIBMCU_METRICS_USER_INCS ${METRICS_USER_DIR})
  endif()

  if(LIBMCU_METRICS_USER_INCS)
    zephyr_library_include_directories(${LIBMCU_METRICS_USER_INCS})
  endif()

  if(CONFIG_LIBMCU_METRICS_USER_DEFINES AND NOT CONFIG_LIBMCU_METRICS_USER_DEFINES STREQUAL "")
    zephyr_library_compile_definitions(
      METRICS_USER_DEFINES="${CONFIG_LIBMCU_METRICS_USER_DEFINES}"
    )
  endif()
endif()
