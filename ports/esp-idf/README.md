## cpuload

The CPU load can be calculated for different periods (1 second, 1 minute, 5 minutes) and for different cores. The CPU load is calculated based on the time spent in idle and active tasks. The feature can be enabled by adding a compile definition in the CMakeLists.txt file like the follwing.

```cmake
idf_build_set_property(COMPILE_DEFINITIONS -DtraceTASK_SWITCHED_IN=on_task_switch_in APPEND)
```
