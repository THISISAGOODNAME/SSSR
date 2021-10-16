add_library(winpixeventruntime INTERFACE)
target_include_directories(winpixeventruntime INTERFACE "${project_root}/3rdparty/winpixeventruntime/Include")
target_link_directories(winpixeventruntime INTERFACE "${project_root}/3rdparty/winpixeventruntime/bin/x64")
target_link_libraries(winpixeventruntime INTERFACE "WinPixEventRuntime")
