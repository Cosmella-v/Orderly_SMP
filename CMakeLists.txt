cmake_minimum_required(VERSION 3.15)
project(OrderlySMPInstaller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)

find_package(OpenGL REQUIRED)
include(FetchContent)

FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git GIT_TAG 1.9.2)
FetchContent_Declare(glad GIT_REPOSITORY https://github.com/Dav1dde/glad.git GIT_TAG v0.1.36)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8)

FetchContent_MakeAvailable(cpr glad glfw)

add_executable(OrderlySMPInstaller MACOSX_BUNDLE
  main.cpp
  downloader.cpp
  unzipper.cpp
  parser.cpp
  CreateProfile.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp  
  ${CMAKE_CURRENT_SOURCE_DIR}/external/miniz/miniz.c  
  ${CMAKE_CURRENT_SOURCE_DIR}/external/TinyFileDialogs/tinyfiledialogs.c  
)

target_include_directories(OrderlySMPInstaller PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
  ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
  ${CMAKE_CURRENT_SOURCE_DIR}/external/miniz
  ${CMAKE_CURRENT_SOURCE_DIR}/external/TinyFileDialogs
)

target_include_directories(OrderlySMPInstaller PRIVATE
  ${glad_SOURCE_DIR}/include
  ${glfw_SOURCE_DIR}/include
)
target_link_libraries(OrderlySMPInstaller PRIVATE
  cpr::cpr
  glad
  glfw
  OpenGL::GL
)
target_include_directories(OrderlySMPInstaller PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


if(WIN32)
    target_sources(OrderlySMPInstaller PRIVATE resources/icon.rc)

elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set_source_files_properties(resources/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(OrderlySMPInstaller PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist
    )

elseif(UNIX)
    set(APP_NAME "Orderly SMP Installer")
    set(APP_EXEC "OrderlySMPInstaller")
    set(APP_ICON "orderlysmp")

    configure_file(
        ${CMAKE_SOURCE_DIR}/resources/orderlysmp.desktop.in
        ${CMAKE_BINARY_DIR}/orderlysmp.desktop
        @ONLY
    )

    install(FILES ${CMAKE_BINARY_DIR}/orderlysmp.desktop
        DESTINATION share/applications
    )

    install(FILES ${CMAKE_SOURCE_DIR}/resources/icon.png
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME ${APP_ICON}.png
    )
endif()
