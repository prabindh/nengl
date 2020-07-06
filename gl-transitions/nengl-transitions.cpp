#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <string>
#include <vector>

#include "nengl_ws.h"
#include "nengl_core.h"

// shader reads
std::string ReadShaderFromFile(const char *filePath);

int init_object(nengl_core* obj1, int id)
{
    int ret = -1;

    if (id > 0)
        return -1;

    float vattribs[3 * 4 * 2] = {
        -1, 1, -1,
        -1, -1, -1,
        1, -1, -1,
        1, 1, -1,

        -1, 1, 1,
        -1, -1, 1,
        1, -1, 1,
        1, 1, 1,

    };

    float texture_coords_2d[2 * 4 * 2] = {
        0, 1,
        0, 0,
        1, 0,
        1, 1,

        1, 1,
        1, 0,
        0, 0,
        0, 1
    };

    short indices[6 * 6] = { 0, 1, 2, 0, 2, 3,
                3, 2, 6, 3, 6, 7,
                7, 6, 5, 7, 5, 4,
                4, 5, 1, 4, 1,0,
                0,3,7, 0, 7,4,
                1,2,6,1,6,5 };

    NENGL_TEXTURE_OBJ texture[2] = { 0 };
    unsigned char green[] = { 0, 0xFF, 0, 0 };
    unsigned char red[] = { 0xFF, 0, 0, 0 };
    unsigned char blue[] = { 0, 0, 0xFF, 0x80 };  

    std::string vshader = ReadShaderFromFile("v_textureshader.glsl");
    const char* vshaderstr = vshader.c_str();
    std::string fshader = ReadShaderFromFile("f_textureshader.glsl");
    const char* fshaderstr = fshader.c_str();

    ret = obj1->setup_shaders_array(&vshaderstr, 1, &fshaderstr, 1);
    if (ret)
    {
        D_PRINTF("Error setting up shaders\n");
        exit(-1);
    }
    obj1->setup_uniform1f("progress", 0.0f);

    ret = obj1->setup_attribute_mesh(vattribs, sizeof(vattribs), "aVertexPosition", 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);


    if (true)
    {
        texture[0].data = red;
        texture[0].filename = 0;
        texture[0].height = texture[0].width = 1;
        texture[0].depth = 1;
        texture[0].type = NENGL_COLOR_FORMAT_ARGB32;
        texture[0].dim = NENGL_TEXTURE_2D;
        texture[0].sampler_name = "uSampler0";
        obj1->setup_texture_data(&texture[0]);
    }
    ret = obj1->setup_attribute_mesh(texture_coords_2d, sizeof(texture_coords_2d), 
                    "aTextureCoord", 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    ret = obj1->setup_index_mesh(indices, 6 * 6, sizeof(short));

    return ret;
}

//Main code
int main()
{
    nengl_ws* ws;
    float angle = 0;
    float scaler = 0;
    int objid = 0;
    int loopCount = 0;
    NENGL_TEXTURE_OBJ texture[2] = { 0 };
    unsigned char green[] = { 0, 0xFF, 0, 0 };
    unsigned char red[] = { 0xFF, 0, 0, 0 };
    unsigned char blue[] = { 0, 0, 0xFF, 0x80 };
    std::vector<nengl_core*> mesh_objects;
    int ret = -1;
    nengl_core* obj1 = NULL;
    void* win = NULL;
    float curr_progress = 0.0;


    //create window
    ws = new nengl_ws();
    win = ws->create_window(640, 480, "Nengl Window");
    if (!win)
    {
        D_PRINTF("Error creating window!");
        exit(-1);
    }
    ws->depth_flag(true);
    int extcheck = ws->check_extension("GL_OES_packed_depth_stencil");

    /* Loop until the user closes the window */

    obj1 = new nengl_core();
    ret = init_object(obj1, objid++);

    //TODO find a better way to go till only max object num of each mesh
    if (ret) { delete obj1; exit(-1); }
    mesh_objects.push_back(obj1);

    while (ws->get_window_status(win))
    {
        ws->clear_screen(0.0, 0.9, 0.0);
        ws->clear_depth();
        //mesh draw loop
        for (std::vector<nengl_core*>::iterator obj1 = mesh_objects.begin(); obj1 != mesh_objects.end(); obj1++)
        {
            /* Rotate obj1 */
            (*obj1)->default_transform(NENGL_VIEW_PERSPECTIVE);
            (*obj1)->translate(0, 0, 5, "mvMatrix");
            (*obj1)->rotate(0, curr_progress * 90, 0, "mvMatrix");
            /* Update texture */
            if (true)
            {
                int texOption = loopCount % 3;
                texture[0].data = texOption == 0 ? red : (texOption == 1 ? blue : green);
                texture[0].filename = 0;
                texture[0].height = texture[0].width = 1;
                texture[0].depth = 1;
                texture[0].type = NENGL_COLOR_FORMAT_ARGB32;
                texture[0].dim = NENGL_TEXTURE_2D;
                texture[0].sampler_name = "uSampler0";
            }
            (*obj1)->update_texture_data(&texture[0]);
            (*obj1)->setup_uniform1f("progress", curr_progress);
            /* Display */
            (*obj1)->draw();

            loopCount++;
            curr_progress += 0.001;
            if (curr_progress > 0.9)
                curr_progress = 0.0;
        }
        ws->swap(win);
        /* Poll for and process events */
        ws->handle_events(win);
    }
    return 0;
}

