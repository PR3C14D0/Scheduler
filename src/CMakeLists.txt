add_executable("Scheduler-Injector" "main.cpp")

add_library("Scheduler-DLL" SHARED "dllmain.cpp" "il2cpp.h")
set_target_properties("Scheduler-DLL" PROPERTIES OUTPUT_NAME "SchedulerDLL")
set_target_properties("Scheduler-DLL" PROPERTIES PREFIX "")
target_link_libraries(Scheduler-Injector -static)
target_link_libraries(Scheduler-DLL -static)
