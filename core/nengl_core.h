/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
* 
* Header file
*
*******************************************************************/

#ifndef __NENGL_CORE_H
#define __NENGL_CORE_H

#include "stdio.h"
#include "stdlib.h"

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4, glm::ivec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#define GLFW_INCLUDE_ES2 1
#define GLFW_INCLUDE_GLEXT 1
#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

    //debug
#ifdef _DEBUG
#define D_PRINTF  printf
#define GL_CHECK(x) \
    { \
    int err = glGetError(); \
    printf("GL Error = %x for %s\n", err, (char*)(#x)); \
    if (GL_NO_ERROR != err) \
    { \
        exit(-1); \
    }\
}
#else //DEBUG
#define D_PRINTF(x)
#define GL_CHECK(x)
#endif //DEBUG

    enum NENGL_COLOR_FORMAT{
        NENGL_COLOR_FORMAT_RGB24,
        NENGL_COLOR_FORMAT_ARGB32,
        NENGL_COLOR_FORMAT_RGB16,
	    NENGL_COLOR_FORMAT_LUM8
    };
    enum NENGL_VIEW_TYPE{
        NENGL_VIEW_PERSPECTIVE,
        NENGL_VIEW_ORTHO
    };

    enum NENGL_TEXTURE_DIMENSION{
        NENGL_TEXTURE_2D,
        NENGL_TEXTURE_3D
    };

#define MAX_STATE_ID 8

    typedef struct _gl_state
    {
        int program;
        int num_attribs;
        int num_textures;
        unsigned int textureID[MAX_STATE_ID];
        unsigned int textureType[MAX_STATE_ID];
        int samplerLoc[MAX_STATE_ID];
        unsigned int vboID[MAX_STATE_ID];
        int attrib_loc[MAX_STATE_ID];
        int attrib_num_elems[MAX_STATE_ID]; 
        int attrib_type[MAX_STATE_ID];
        bool attrib_normalised[MAX_STATE_ID];
        int attrib_stride[MAX_STATE_ID];
        void* attrib_elem_offset[MAX_STATE_ID];
        unsigned int index_mode; //0 0r 1
    }gl_state;

    typedef struct _NENGL_TEXTURE_OBJ
    {
        void* data;
        const char* filename;
        unsigned int width;
        unsigned int height;
        unsigned int depth;
        NENGL_COLOR_FORMAT type;
	    NENGL_TEXTURE_DIMENSION dim;
        const char* sampler_name;
    }NENGL_TEXTURE_OBJ;


    /**
    Class to draw objects using GLES2
    */
    class nengl_core
    {
        gl_state curr_state;
        glm::mat4 curr_xfm;
        int numindices;
        bool draw_mode_indexed;
    public:
        nengl_core();
        ~nengl_core();
        int setup_attribute_mesh(void* attribs, int buffer_size, const char* attrib_name,
            int attrib_num_elems, int attrib_type,
            bool attrib_normalised, int attrib_stride, void* attrib_elem_offset);
        int setup_index_mesh(void* indices, int num, int bytes_per_index);
        int setup_texture_data(NENGL_TEXTURE_OBJ* objArray);
        int update_texture_data(NENGL_TEXTURE_OBJ* objArray);
	    int setup_fbo_as_texture_data(NENGL_TEXTURE_OBJ* objArray, unsigned int textureId);
	    int setup_shaders(const char** s, int len, GLenum type, GLuint * obj);
        int setup_shaders_array(const char** v, int vlen, const char** f, int flen);
        int scale(float x, float y, float z, const char* matrix);
        int rotate(float x, float y, float z, const char* matrix);
        int translate(float x, float y, float z, const char* matrix);
        int default_transform(NENGL_VIEW_TYPE type);
        int alpha(float a);
        int setup_uniform1f(const char* name, float f);
        int setup_uniform3fv(const char* name, float* f);
        int setup_point_light(float originx, float originy, float originz, float farx, float fary, float farz);
        int restore_attribs();
        int draw();
    };

#ifdef __cplusplus
}
#endif

#endif
