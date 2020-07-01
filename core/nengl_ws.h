/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
*
* Header file for window systems
* 
*******************************************************************/

#ifndef __NENGL_WS_H
#define __NENGL_WS_H

#include "stdio.h"
#include "stdlib.h"


#ifdef __cplusplus
extern "C" {
#endif

    typedef void(*MOUSE_BUTTON_HANDLER_FUNC)(int button, int action);
    typedef void(*CURSOR_POS_HANDLER_FUNC)(double x, double y);

    /**
    Class to handle construction of EGL contexts, and enable working with OpenGL ES2 on a single window.
    */
    class nengl_ws
    {
    public:
        nengl_ws();
        ~nengl_ws();
        void* create_window(int width, int height, const char* title);
        void set_mouse_button_handler(MOUSE_BUTTON_HANDLER_FUNC handler);
        void set_cursor_pos_handler(CURSOR_POS_HANDLER_FUNC handler);
        bool get_window_status(void* window);
        void swap(void* window);
        void handle_events(void* window);
        void clear_screen(float r, float g, float b);
        void depth_flag(bool flag);
        void clear_depth();
        int check_extension(char* ext);
    };

#ifdef __cplusplus
}
#endif

#endif
