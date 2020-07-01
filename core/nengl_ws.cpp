/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
*
* Simple class to encapsulate Window Systems using GLFW3
* 
*******************************************************************/


#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

#include "nengl_ws.h"
#include "nengl_core.h"

nengl_ws::nengl_ws()
{
    /* Initialize the library */
    if (!glfwInit())
        return;

    /* Set hints */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
}

nengl_ws::~nengl_ws()
{
    glfwTerminate();
}

void* nengl_ws::create_window(int width, int height, const char* title)
{
    GLFWwindow *window;
    /* Create a windowed mode window and its OpenGL context. After this time, all EGL context has been created for us */
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return 0;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    return window;
}

void nengl_ws::set_mouse_button_handler(MOUSE_BUTTON_HANDLER_FUNC handler)
{}
void nengl_ws::set_cursor_pos_handler(CURSOR_POS_HANDLER_FUNC handler)
{}

bool nengl_ws::get_window_status(void* window)
{
    return !glfwWindowShouldClose((GLFWwindow*)window);
}

void nengl_ws::clear_screen(float r, float g, float b)
{
    glClearColor(r, g, b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void nengl_ws::swap(void* window)
{
    glfwSwapBuffers((GLFWwindow*)window);
}

void nengl_ws::handle_events(void* window)
{
    glfwPollEvents();
}

void nengl_ws::depth_flag(bool flag)
{
    //glfw default enables DEPTHBITS to 24
    if (true == flag)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void nengl_ws::clear_depth()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

int nengl_ws::check_extension(const char* ext)
{
    /* Get extension string checked, returns GL_FALSE or GL_TRUE */
    int ret = glfwExtensionSupported(ext);
    return ret;
}


