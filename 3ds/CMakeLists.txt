cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

SET(name nengl_3ds)

include_directories("../extern_glm")
include_directories("../core/")

add_executable (${name} ../core/nengl_core.cpp ../core/nengl_ws.cpp ../core/lodepng.cpp ./nengl_test.cpp ./3ds.cpp)

IF(WIN32)
	include_directories("../extern/include/gles2")
	include_directories("../extern/include")
        target_link_libraries(${name} ../../extern/libs/glfw3 ../../extern/libs/libEGL ../../extern/libs/libGLESv2)
ELSE(WIN32)
	target_link_libraries( ${name} glfw3 EGL GLESv2 pthread X11 Xrandr GL Xxf86vm Xi Xcursor Xinerama )
	SET(CMAKE_CXX_FLAGS "-std=c++0x")
ENDIF(WIN32)

