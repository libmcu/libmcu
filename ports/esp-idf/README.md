## cpuload

Add the following to your `CMakeLists.txt` file to include the `cpuload` functionality in your project:

```cmake
idf_build_set_property(COMPILE_DEFINITIONS -DtraceTASK_SWITCHED_IN=on_task_switch_in APPEND)
```
