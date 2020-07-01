/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
*
* Test code for 3DS model file loading functionality
* 
*******************************************************************/
#define GLFW_INCLUDE_ES2 1
#include <GLFW/glfw3.h>

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

#include "nengl_ws.h"
#include "nengl_core.h"

#include "3ds.h"			// Include the 3DS header file.

//#version 300 es \

static const char* vshader_lighting1[] = { 
"attribute vec4 aVertexPosition;"
"attribute vec2 aTextureCoord;"
"vec3 normal;"
"uniform mat4 mvMatrix;"
"uniform vec3 uLightVector;"
"varying vec2 vTextureCoord;"
"varying vec3 vColor;"
"varying float diffuse;"
"float distance;"
"vec3 lightPos = vec3(0.0,18.0, 25.0);"
"vec4 converted;"
"vec4 converted_lightPos;"
"void main(void) {"
"converted = mvMatrix * aVertexPosition;"
"converted_lightPos = mvMatrix * vec4(lightPos,1.0);"
"vTextureCoord = aTextureCoord;"
"normal=normalize(converted.xyz);"
"distance=length(vec3(converted_lightPos)-vec3(converted));"
"diffuse = max(dot(normal, normalize(vec3(converted_lightPos-converted))), 0.1);"
"vColor= diffuse*vec3(0.5,0.5,0.5);"
"gl_Position = mvMatrix * aVertexPosition;"
"}"
};


//Note: the higher the Y value of lightPos is - the more it simulates night time.
//" diffuse = diffuse*(1.0/ (1.0 + (0.25* distance*distance)) );"

static const char* fshader_lighting1[] = {
"precision mediump float;"
"varying vec2 vTextureCoord;"
"uniform sampler2D uSampler0;"
"varying vec3 vColor;"
"varying float diffuse;"
"vec4 tempc;"
"void main(void) {"
"tempc=diffuse*texture2D(uSampler0, vec2(vTextureCoord.x, 1.0-vTextureCoord.y));"
"gl_FragColor=tempc;"
"}"
}
;

//vec4(vColor,0.0)+
// + vec4(vColor,1.0)


//3DS Variable Information
#define FILE_TEXTURE_3DS "somename.3ds"

//"cube.3ds"

CLoad3DS g_Load3ds;	// This is 3DS class.  This should go in a good model class.
t3DModel g_3DModel;	// This holds the 3D Model info that we load in
float* dsvertexBuffer;
unsigned int *dsindexBuffer;

typedef struct _ds_object_info
{
	char* objname;
	unsigned int num_vertices;
	unsigned int num_indices;
	unsigned int num_tex_coords;
	unsigned short* indices;
	float* vertices;
	float* tex_coords;
	char* texturefilename;
}ds_object_info;

ds_object_info model_info;
static int sky_objid = 0; //for clockwise cull

int load_3ds_model(char* filename, unsigned int* num_objects)
{
	bool ret;
    // Load our .3DS file into our model structure
	ret = g_Load3ds.Import3DS(&g_3DModel, filename);

	if(ret) 
		*num_objects = g_3DModel.numOfObjects;

	return ret;
}

int load_3ds_object(int id, ds_object_info* info)
{
	t3DObject* pObject;
	int i;

	if(id >= g_3DModel.numOfObjects) 
	{
		D_PRINTF("Error retrieving object id = %d\n", id);
		return -1;
	}

	pObject = &g_3DModel.pObject[id];
	info->objname = pObject->strName;
	info->num_vertices = pObject->numOfVerts;
	info->num_indices = pObject->numOfFaces*3;
	info->num_tex_coords = pObject->numTexVertex;
	info->vertices = pObject->vertices;
	info->tex_coords = pObject->uvcoords;
	info->indices = pObject->indices;
	info->texturefilename = pObject->texture_filename;
	//if by any chance we do not get a filename, store default!! - TODO
	if(!info->texturefilename) {info->texturefilename = "4.PNG"; printf("no Texture for id=%d, %s\n", id, info->objname); exit(-1);}

    //TODO - find better way to do front cull
    if(!strcmp(pObject->strName, "Sphere")) sky_objid = id;

	D_PRINTF("Objectname = %s, id=%d\n", pObject->strName, id);
	D_PRINTF("numtexcoords = %d\n", info->num_tex_coords);
	D_PRINTF("num indices = %d\n", info->num_indices);
	D_PRINTF("num vertices = %d\n", info->num_vertices);
	D_PRINTF("texturefilename = %s\n", info->texturefilename);
#if 0
	//if(!strcmp(info->texturefilename, "glass.png"))
	{
		for(i = 0;i < info->num_tex_coords;i ++)
		{
			D_PRINTF("texcoord[%d].x, y = %f, %f\n", i, info->tex_coords[i*2], info->tex_coords[i*2 + 1]);
		}
		for(i = 0;i < info->num_vertices;i ++)
		{
			D_PRINTF("vertices[%d].x, y = %f, %f\n", i, info->vertices[i*3], info->vertices[i*3 + 1]);
		}
		for(i = 0;i < info->num_indices;i ++)
		{
			D_PRINTF("indices[%d] = %d\n", i, info->indices[i]);
		}
		//exit(-1);
	}
#endif
	return 0;
}

#if 0
void temp_load()
{
	// Initialise each object
	for(int index = 0; index < g_3DModel.numOfObjects; index++)
	{
		pObject = &g_3DModel.pObject[index];

		//Allocate memory for this object
		dsvertexBuffer = (float*)malloc(sizeof(float)*pObject->numOfVerts*3);
		dsindexBuffer = (unsigned int*)malloc(sizeof(unsigned int)*pObject->numOfFaces*3);

		//Store all vertices of this object
		for(int i = 0; i < pObject->numOfVerts; i++)
		{
			printf("vertices of %dth object = [%f,%f,%f]\n", index, pObject->pVerts[i].x, pObject->pVerts[i].y, pObject->pVerts[i].z);
			dsvertexBuffer[3*i] = pObject->pVerts[i].x;
			dsvertexBuffer[3*i + 1] = pObject->pVerts[i].y;
			dsvertexBuffer[3*i + 2] = pObject->pVerts[i].z;
		}
		// Go through all of the faces in this object
		for(int i = 0; i < pObject->numOfFaces; i++)
		{
			printf("indices of %dth object = [%d,%d,%d]\n", index, pObject->pFaces[i].vertIndex[0], pObject->pFaces[i].vertIndex[1], pObject->pFaces[i].vertIndex[2]);
			// Next, we read in the A then B then C index for the face, but ignore the 4th value.
			// The fourth value is a visibility flag for 3D Studio Max, we don't care about this.
			//Use the index for drawing
			dsindexBuffer[3*i] = pObject->pFaces[i].vertIndex[0];
			dsindexBuffer[3*i + 1] = pObject->pFaces[i].vertIndex[1];
			dsindexBuffer[3*i + 2] = pObject->pFaces[i].vertIndex[2];
		}
		printf("# of Texture coords of %dth object = %d\n", index, pObject->numTexVertex);

		if(dsvertexBuffer) free(dsvertexBuffer);
		if(dsindexBuffer) free(dsindexBuffer);
	}
}
#endif

void init_object(nengl_core* obj1)
{
    int ret;
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

    float texture_coords_2d[2 * 4 *2] = {
        0, 1,
        0, 0,
        1, 0,
        1, 1,

        0, 1,
        0, 0,
        1, 0,
        1, 1
    };

    short indices[6 * 6] = { 0, 1, 2, 0, 2, 3,
				3, 2, 6, 3, 6, 7,
				7, 6, 5, 7, 5, 4,
				4, 5, 1, 4, 1,0,
				0,3,7, 0, 7,4,
				1,2,6,1,6,5  };

    NENGL_TEXTURE_OBJ texture[2] = { 0 };
    unsigned char green[] = { 0, 0xFF, 0, 0 };
    unsigned char red[] = { 0xFF, 0, 0, 0 };
    unsigned char blue[] = { 0, 0, 0xFF, 0x80 };

    ret = obj1->setup_shaders_array(vshader_lighting1, sizeof(vshader_lighting1)/sizeof(void*), fshader_lighting1, sizeof(fshader_lighting1)/sizeof(void*));
    if(ret)
    {
       D_PRINTF("Error setting up shaders\n");
       exit(-1);
    }

    obj1->setup_attribute_mesh(vattribs, sizeof(vattribs), "aVertexPosition", 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    obj1->setup_index_mesh(indices, 6*6, sizeof(short));

    texture[0].data = blue;
    texture[0].filename = 0;
    texture[0].height = texture[0].width = 1;
    texture[0].depth = 1;
    texture[0].type = NENGL_COLOR_FORMAT_ARGB32;
    texture[0].dim = NENGL_TEXTURE_2D; 
    texture[0].sampler_name = "uSampler0";
    obj1->setup_texture_data(&texture[0]);

    obj1->setup_attribute_mesh(texture_coords_2d, sizeof(texture_coords_2d), "aTextureCoord", 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    float lightv[]={1.0,0.0,0.0};
    obj1->setup_uniform3fv("uLightVector", lightv);
}

int init_object_3ds(nengl_core* obj1, int objid)
{
    int ret;
    NENGL_TEXTURE_OBJ texture[2] = { 0 };
    unsigned char green[] = { 0, 0xFF, 0, 0 };
    unsigned char red[] = { 0xFF, 0, 0, 0 };
    unsigned char blue[] = { 0, 0, 0xFF, 0x80 };

    //LOAD 0th index only for now - TODO - move this id to be as input
    ret = load_3ds_object(objid, &model_info);
    if(ret)
    {
       D_PRINTF("Error loading obj id %d\n", objid);
       return -1;
    }

    ret = obj1->setup_shaders_array(vshader_lighting1, sizeof(vshader_lighting1)/sizeof(void*), fshader_lighting1, sizeof(fshader_lighting1)/sizeof(void*));
    if(ret)
    {
       D_PRINTF("Error setting up shaders\n");
       return -1;
    }

    obj1->setup_attribute_mesh(model_info.vertices, model_info.num_vertices*3*sizeof(float), "aVertexPosition", 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

#if 0
    texture[0].data = blue;
    texture[0].filename = 0;
    texture[0].height = texture[0].width = 1;
    texture[0].depth = 1;
    texture[0].type = NENGL_COLOR_FORMAT_ARGB32;
    texture[0].dim = NENGL_TEXTURE_2D; 
    texture[0].sampler_name = "uSampler0";
    obj1->setup_texture_data(&texture[0]);
#else
    texture[0].data = 0;
    texture[0].filename = model_info.texturefilename;
    texture[0].depth = texture[0].height = texture[0].width = 1; //to enter into texturing in core
    texture[0].dim = NENGL_TEXTURE_2D;
    texture[0].sampler_name = "uSampler0";
    obj1->setup_texture_data(&texture[0]);
#endif
    obj1->setup_attribute_mesh(model_info.tex_coords, 2*sizeof(float)*model_info.num_tex_coords, "aTextureCoord", 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    //TODO - due to the way restore attrib is setup, index setup HAS to be last!!
    obj1->setup_index_mesh(model_info.indices, model_info.num_indices, sizeof(unsigned short));

    float lightv[]={1.0,0.0,0.0};
    obj1->setup_uniform3fv("uLightVector", lightv);
}

static double offset_x=0;
static double offset_y=0;
static double offset_z=0;
void main_cursor_pos_func(double x, double y)
{
	offset_x = x;
	offset_y = y;
}
void main_mouse_button_func(int button, int action)
{
	D_PRINTF("button = %d\n", button);
	if(button == GLFW_MOUSE_BUTTON_1)
	{
		//move towards -z
		offset_z -= 0.5;
	}else
	{
		//move towards z
		offset_z += 0.5;
	}
}

//Profiling
static struct timeval starttime, endtime;

long get_profile_data(char* task, struct timeval* start, struct timeval* end)
{
    long mtime, seconds, useconds; 

    seconds  = end->tv_sec  - start->tv_sec;
    useconds = end->tv_usec - start->tv_usec;

    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

    printf("Elapsed time for task [%s]: %ld milliseconds\n", task, mtime);

    return mtime;
}

//Main code
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
    ws->set_cursor_pos_handler(win, main_cursor_pos_func);
    ws->set_mouse_button_handler(win, main_mouse_button_func);
    ws->depth_flag(true);
    int extcheck = ws->check_extension("GL_OES_packed_depth_stencil");
  
    /* Load 3d model */
    unsigned int numobjects;
    gettimeofday(&starttime, NULL);
    bool ret = load_3ds_model(FILE_TEXTURE_3DS, &numobjects);
    if(!ret) 
    {
        D_PRINTF("Error loading 3D model!");        
        exit(-1);
    }
    gettimeofday(&endtime, NULL);
    get_profile_data("3D MODEL LOAD", &starttime, &endtime);

    /* Loop until the user closes the window */
    static float angle = 0;
    static float scaler=0;
    static int objid = 0;

	//Create the objects in the mesh
	//setup the object obj1
    gettimeofday(&starttime, NULL);
	static std::vector<nengl_core*> mesh_objects;
	while(1)
	{
		nengl_core* obj1 = new nengl_core();
		ret = init_object_3ds(obj1, objid++);
		//TODO find a better way to go till only max object num of each mesh
		if(ret) {delete obj1; break;}
		mesh_objects.push_back(obj1);
	}
    gettimeofday(&endtime, NULL);
    get_profile_data("3D MODEL LOAD", &starttime, &endtime);

    while (ws->get_window_status(win))
    {
	ws->clear_screen(0.2, 0.6, 0.5);
	ws->clear_depth();
	//mesh draw loop
	int currobj_id=0;
	for(std::vector<nengl_core*>::iterator obj1=mesh_objects.begin(); obj1 != mesh_objects.end(); obj1 ++)
	{
		/* Rotate obj1 */
		(*obj1)->default_transform(NENGL_VIEW_PERSPECTIVE);
		(*obj1)->translate(0, -3, 25+offset_z, "mvMatrix"); //-6+(float)(offset_y)/(float)200
		(*obj1)->rotate(-8, (1280-offset_x), 0, "mvMatrix");
		/* Display */
		(*obj1)->draw();
		currobj_id++;
	}
        ws->swap(win);
	/* Poll for and process events */
	ws->handle_events(win);

        angle += 5.0;
	scaler += 0.001;
	if(scaler > 1.0) scaler = 0.0;

	D_PRINTF("offsety= %d, offsetz = %d\n", offset_y, offset_z);
    }
    return 0;
}

