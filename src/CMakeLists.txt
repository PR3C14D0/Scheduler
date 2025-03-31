add_executable("Scheduler-Injector" "main.cpp")

add_library("Scheduler-DLL" SHARED "dllmain.cpp")
set_target_properties("Scheduler-DLL" PROPERTIES OUTPUT_NAME "SchedulerDLL")
set_target_properties("Scheduler-DLL" PROPERTIES PREFIX "")
