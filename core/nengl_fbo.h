/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
*
* Simple class to encapsulate FBO
* 
*******************************************************************/
#ifndef __NENGL_CORE_FBO_H
#define __NENGL_CORE_FBO_H

#include "nengl_core.h"

    /**
    Class to manage fbo
    */
    class nengl_fbo
    {
	GLuint fbo_id;
	GLuint fbo_texture_id;
	int width, height;
    public:
        nengl_fbo(int width, int height);
        ~nengl_fbo();
	int set();
	int unset();
	int get_reference_id();
	int get_width();
	int get_height();
    };
#endif
