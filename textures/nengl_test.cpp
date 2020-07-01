/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
*
* Test code for various functionalities
* 
*******************************************************************/
#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

#include "nengl_ws.h"
#include "nengl_core.h"


//#version 300 es \

static const char* vshader_simple[] = { 
"attribute vec4 aVertexPosition;"
"attribute vec2 aTextureCoord;"
"varying vec2 vTextureCoord;"
"uniform mat4 mvMatrix;"
"void main(void) {"
"gl_Position = mvMatrix * aVertexPosition;"
"vTextureCoord = aTextureCoord;"
"}"
};

static const char* fshader_simple[] = {
"precision mediump float;"
"varying vec2 vTextureCoord;"
"uniform sampler2D uSampler0;"
"vec4 tempc;"
"void main(void) {"
"gl_FragColor=texture2D(uSampler0, vTextureCoord);"
"}"
}
;

void init_object(nengl_core* obj1)
{

    float vattribs_vertical[3 * 4] = {
        -1, 1, 0,
        -1, -1, 0,
        1, -1, 0,
        1, 1, 0,

    };
    float vattribs[3 * 4] = {
        -1, 0, 1,
        -1, 0, -1,
        1, 0, -1,
        1, 0, 1,

    };

    float texture_coords_2d[2 * 4] = {
        0, 1,
        0, 0,
        1, 0,
        1, 1
    };

    short indices[6] = { 0, 1, 2, 0, 2, 3  };
    NENGL_TEXTURE_OBJ texture[2] = { 0 };
    unsigned char green[] = { 0, 0xFF, 0, 0 };
    unsigned char red[] = { 0xFF, 0, 0, 0 };
    unsigned char blue[] = { 0, 0, 0xFF, 0 };
    unsigned char green_red_2_2_2[] = { 0, 0xFF, 0, 0,    0xFF, 0, 0, 200,
				0, 0xFF, 0, 0,    0xFF, 0, 0, 200,
				 0, 0xFF, 0, 200,    0xFF, 0, 0, 100,   
				0, 0xFF, 0, 200,    0xFF, 0, 0, 100,

				 0, 0xFF, 0, 0,    0xFF, 0, 0, 200,   
				0, 0xFF, 0, 0,    0xFF, 0, 0, 200, 
				0, 0xFF, 0, 0,    0xFF, 0, 0, 100,   
				0, 0xFF, 0, 0,    0xFF, 0, 0, 100, 				
		  };

    obj1->setup_shaders_array(vshader_simple, sizeof(vshader_simple)/sizeof(void*), fshader_simple, sizeof(fshader_simple)/sizeof(void*));

    obj1->setup_attribute_mesh(vattribs, sizeof(vattribs), "aVertexPosition", 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    obj1->setup_index_mesh(indices, 6, sizeof(short));

#if 0
    texture[0].data = green_red_2_2_2;
    texture[0].filename = 0;
    texture[0].height = texture[0].width = 2;
    texture[0].depth = 2; //TODO
    texture[0].type = NENGL_COLOR_FORMAT_ARGB32;
    if(textureType == NENGL_TEXTURE_3D)
    	texture[0].dim = NENGL_TEXTURE_3D; 
    else 
	texture[0].dim = NENGL_TEXTURE_2D; 
#else
    texture[0].filename = "../../images/myladder.raw.data";
    texture[0].height = 1024;
    texture[0].width = 512;
    texture[0].depth = 1;
    texture[0].data = 0;
    texture[0].type = NENGL_COLOR_FORMAT_RGB24;
    texture[0].dim = NENGL_TEXTURE_2D;
#endif
    texture[0].sampler_name = "uSampler0";
    obj1->setup_texture_data(&texture[0]);

    obj1->setup_attribute_mesh(texture_coords_2d, sizeof(texture_coords_2d), "aTextureCoord", 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    GL_CHECK(setup_attribute_mesh);
}

int main()
{
    nengl_ws* ws;

    //create window
    ws = new nengl_ws();
    void* win = ws->create_window(640, 480, "Nengl Window");
    if (!win)
    {
        D_PRINTF("Error creating window!");
	exit(-1);
    }
    ws->depth_flag(true);
    int extcheck = ws->check_extension("GL_OES_packed_depth_stencil");

    /* Loop until the user closes the window */
    static float angle = 0;
    static float translator = 0.0;
    while (ws->get_window_status(win))
    {
        //setup the object obj1
        nengl_core* obj1 = new nengl_core();
        init_object(obj1);
	
        ws->clear_screen(0.2, 0.6, 0.5);
        ws->clear_depth();
        /* Rotate obj1 */
        obj1->default_transform(NENGL_VIEW_PERSPECTIVE);
        obj1->translate(0, 0, 5, "mvMatrix");
        obj1->rotate(angle, 0, 0, "mvMatrix");
        /* Display */
        obj1->draw();

        ws->swap(win);

        /* Poll for and process events */
        ws->handle_events(win);

        /* completely remove obj1 */
        delete obj1;
        angle += 1.0;
    }
    return 0;
}

