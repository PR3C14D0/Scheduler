add_executable("Scheduler-Injector" "main.cpp")
target_link_libraries(Scheduler-Injector -static)


add_library("Scheduler-DLL" SHARED "dllmain.cpp" "il2cpp.h" "Memory.h" "Memory.cpp" "pragma.h" "Util.h"      )
set_target_properties("Scheduler-DLL" PROPERTIES OUTPUT_NAME "dxgi")
set_target_properties("Scheduler-DLL" PROPERTIES PREFIX "")
target_link_libraries(Scheduler-DLL -static)

set("PUBLIC_INCLUDE" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_include_directories("Scheduler-DLL" PUBLIC "${PUBLIC_INCLUDE}")

set("IMGUI_DIR" "${PUBLIC_INCLUDE}/imgui")
file(GLOB "IMGUI_SRC"
	"${IMGUI_DIR}/*.cpp" 
	"${IMGUI_DIR}/*.h")
add_library("ImGui" STATIC "${IMGUI_SRC}")

target_link_libraries(Scheduler-DLL "d3d11.lib;dxgi.lib")
target_link_libraries(Scheduler-DLL ImGui)
