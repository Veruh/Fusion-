cmake_minimum_required(VERSION 3.20)
project(FusionStudio LANGUAGES CXX)
if(NOT WIN32)
  message(FATAL_ERROR "FusionStudio currently targets Windows.")
endif()
set(BASE "${CMAKE_CURRENT_SOURCE_DIR}/../fusion-plus")
add_executable(FusionStudio WIN32 main.cpp
  "${BASE}/ext/imgui/imgui.cpp"
  "${BASE}/ext/imgui/imgui_draw.cpp"
  "${BASE}/ext/imgui/imgui_tables.cpp"
  "${BASE}/ext/imgui/imgui_widgets.cpp"
  "${BASE}/ext/imgui/imgui_impl_win32.cpp"
  "${BASE}/ext/imgui/imgui_impl_opengl2.cpp")
target_compile_features(FusionStudio PRIVATE cxx_std_20)
target_compile_definitions(FusionStudio PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN IMGUI_IMPL_WIN32_DISABLE_GAMEPAD)
target_include_directories(FusionStudio PRIVATE "${BASE}/ext" "${BASE}/src/base")
target_link_libraries(FusionStudio PRIVATE opengl32 gdi32 user32 imm32 dwmapi shell32 ole32)
if(MSVC)
  set_property(TARGET FusionStudio PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(FusionStudio PRIVATE /W4 /utf-8)
endif()
enable_testing()
add_test(NAME standalone_self_test COMMAND FusionStudio --self-test)
