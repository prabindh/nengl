#Linux
#mkdir build && cd build && cmake ..
#windows
#mkdir build && cd build && cmake -G "Visual Studio 12"

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

SET(name nengl_fbo)

include_directories("../extern_glm")
include_directories("../core/")

#link_directories("")

add_executable (${name} ../core/nengl_core.cpp ../core/nengl_ws.cpp ../core/nengl_fbo.cpp ./nengl_test.cpp )

IF(WIN32)
	TARGET_LINK_LIBRARIES(${name} angle glfw3 egl glesv2)
ELSE(WIN32)
	TARGET_LINK_LIBRARIES( ${name} glfw3 EGL GLESv2 pthread X11 Xrandr GL Xxf86vm Xi Xcursor Xinerama )
ENDIF(WIN32)

