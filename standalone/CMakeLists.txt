cmake_minimum_required(VERSION 3.20)
project(FusionStudio LANGUAGES CXX RC)
if(NOT WIN32)
  message(FATAL_ERROR "FusionStudio currently targets Windows.")
endif()
set(BASE "${CMAKE_CURRENT_SOURCE_DIR}/../fusion-plus")
# Inter 4.1 from its official release, verified once at build time. No runtime download.
set(INTER_ARCHIVE "${CMAKE_CURRENT_BINARY_DIR}/Inter-4.1.zip")
if(NOT EXISTS "${INTER_ARCHIVE}")
  file(DOWNLOAD "https://github.com/rsms/inter/releases/download/v4.1/Inter-4.1.zip" "${INTER_ARCHIVE}"
    EXPECTED_HASH SHA256=9883fdd4a49d4fb66bd8177ba6625ef9a64aa45899767dde3d36aa425756b11e TLS_VERIFY ON)
endif()
file(SHA256 "${INTER_ARCHIVE}" INTER_HASH)
if(NOT INTER_HASH STREQUAL "9883fdd4a49d4fb66bd8177ba6625ef9a64aa45899767dde3d36aa425756b11e")
  message(FATAL_ERROR "Inter archive checksum mismatch")
endif()
file(ARCHIVE_EXTRACT INPUT "${INTER_ARCHIVE}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/inter")
set(INTER_REGULAR "${CMAKE_CURRENT_BINARY_DIR}/inter/extras/ttf/Inter-Regular.ttf")
set(INTER_SEMIBOLD "${CMAKE_CURRENT_BINARY_DIR}/inter/extras/ttf/Inter-SemiBold.ttf")
configure_file(fonts.rc.in "${CMAKE_CURRENT_BINARY_DIR}/fonts.rc" @ONLY)
configure_file(LICENSE-Inter.txt "${CMAKE_CURRENT_BINARY_DIR}/LICENSE-Inter.txt" COPYONLY)
add_executable(FusionStudio WIN32 main.cpp
  "${BASE}/ext/imgui/imgui.cpp"
  "${BASE}/ext/imgui/imgui_draw.cpp"
  "${BASE}/ext/imgui/imgui_tables.cpp"
  "${BASE}/ext/imgui/imgui_widgets.cpp"
  "${BASE}/ext/imgui/imgui_impl_win32.cpp"
  "${BASE}/ext/imgui/imgui_impl_opengl2.cpp")
target_sources(FusionStudio PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/fonts.rc")
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
