add_executable("Scheduler-Injector" "main.cpp")
target_link_libraries(Scheduler-Injector -static)

add_library("Scheduler-DLL" SHARED "dllmain.cpp" "il2cpp.h" "Memory.h" "Memory.cpp" "pragma.h" "Util.h")
set_target_properties("Scheduler-DLL" PROPERTIES OUTPUT_NAME "dxgi")
set_target_properties("Scheduler-DLL" PROPERTIES PREFIX "")
target_link_libraries(Scheduler-DLL -static)

target_link_libraries(Scheduler-DLL "d3d11.lib;dxgi.lib")
