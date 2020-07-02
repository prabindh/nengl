/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
* 
* Core functionality module
*
*******************************************************************/

#include "nengl_core.h"

nengl_core::nengl_core()
{
    //curr_xfm = glm::mat4(1.0f);
    curr_xfm = glm::perspective(30.0f, 4.0f / 3.0f, 0.1f, 50.f);
    curr_state.num_attribs = -1;
    curr_state.num_textures = 0;
    draw_mode_indexed = false;
}
nengl_core::~nengl_core()
{
    int i;
    for (i = 0; i <= curr_state.num_attribs; i++)
    {
        glDeleteBuffers(1, &curr_state.vboID[i]);
        glDisableVertexAttribArray(curr_state.attrib_loc[i]);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    for (i = 0; i < curr_state.num_textures; i++)
        glDeleteTextures(1, &curr_state.textureID[i]);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDeleteProgram(curr_state.program);
}
int nengl_core::setup_attribute_mesh(void* attribs, int buffer_size, const char* attrib_name,
        int attrib_num_elems, int attrib_type,
        bool attrib_normalised, int attrib_stride, void* attrib_elem_offset)
{
    GLint loc = glGetAttribLocation(curr_state.program, attrib_name);
    if (loc < 0) return -1;

    curr_state.num_attribs++;
    if (curr_state.num_attribs > (MAX_STATE_ID-1)) return -1;
    glGenBuffers(1, &curr_state.vboID[curr_state.num_attribs]);
    glBindBuffer(GL_ARRAY_BUFFER, curr_state.vboID[curr_state.num_attribs]);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, attribs, GL_STATIC_DRAW);
    
    curr_state.attrib_loc[curr_state.num_attribs] = loc;
    GL_CHECK(glGetAttribLocation);
	D_PRINTF("Info: [%s] attribute located at [%d]\n", attrib_name, curr_state.attrib_loc[curr_state.num_attribs]);

    curr_state.attrib_num_elems[curr_state.num_attribs] = attrib_num_elems;
    curr_state.attrib_type[curr_state.num_attribs] = attrib_type;
    curr_state.attrib_normalised[curr_state.num_attribs] = attrib_normalised;
    curr_state.attrib_stride[curr_state.num_attribs] = attrib_stride;
    curr_state.attrib_elem_offset[curr_state.num_attribs] = attrib_elem_offset;
    glVertexAttribPointer(curr_state.attrib_loc[curr_state.num_attribs], 
        curr_state.attrib_num_elems[curr_state.num_attribs],
        curr_state.attrib_type[curr_state.num_attribs],
        curr_state.attrib_normalised[curr_state.num_attribs],
        curr_state.attrib_stride[curr_state.num_attribs],
        curr_state.attrib_elem_offset[curr_state.num_attribs]);
    GL_CHECK(glVertexAttribPointer);
    glEnableVertexAttribArray(curr_state.attrib_loc[curr_state.num_attribs]);
    GL_CHECK(glEnableVertexAttribArray);
    return 0;
}

int nengl_core::setup_index_mesh(void* indices, int num, int bytes_per_index)
{ 
    curr_state.num_attribs++;
    if (curr_state.num_attribs > (MAX_STATE_ID-1)) return -1;
    glGenBuffers(1, &curr_state.vboID[curr_state.num_attribs]);
    GL_CHECK(glGetBuffers);
    numindices = num;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr_state.vboID[curr_state.num_attribs]);
    GL_CHECK(glBindBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num*bytes_per_index, indices, GL_STATIC_DRAW);
    GL_CHECK(glBufferData);
    draw_mode_indexed = true;
    //BAD!! TODO
    if (2 == bytes_per_index) curr_state.index_mode = 0;//short
    else curr_state.index_mode = 1; //long, heaven forbid TODO
    return 0;
}

int nengl_core::setup_texture_data(NENGL_TEXTURE_OBJ* objArray)
{
    int i = 0;
    int loc;
    NENGL_TEXTURE_OBJ* obj = objArray;

    restore_attribs();
    while (obj->height)
    {
        loc = glGetUniformLocation(curr_state.program, obj->sampler_name);
        if (i > GL_MAX_TEXTURE_IMAGE_UNITS)
        {
            D_PRINTF("Texture requirements exceed available HW texture units"); 
            return -1;
        }
	    D_PRINTF("Sampler loc = %d for name %s\n", loc, obj->sampler_name);
	    GL_CHECK(glGetUniformLocation);
	    glGenTextures(1, &curr_state.textureID[i]);
	
        glUniform1i(loc, 0);
        glActiveTexture(GL_TEXTURE0 + i);
	    if(obj->dim == NENGL_TEXTURE_2D)
	    {
            glBindTexture(GL_TEXTURE_2D, curr_state.textureID[i]);
		    if (obj->data) //data buffer already available
		    {
		        glTexImage2D(
		            GL_TEXTURE_2D,
		            0,
		            (obj->type == NENGL_COLOR_FORMAT_ARGB32) ? GL_RGBA : GL_RGB,
		            obj->width,
		            obj->height,
		            0,
		            (obj->type == NENGL_COLOR_FORMAT_ARGB32) ? GL_RGBA : GL_RGB,
		            GL_UNSIGNED_BYTE,
		            obj->data
		            );
		        GL_CHECK(glTexImage2D);
		    }
		    else if (obj->filename)
		    {
			    D_PRINTF("loading from %s\n", obj->filename);
			    int bpp = (obj->type == NENGL_COLOR_FORMAT_ARGB32) ? 4:3;
                FILE* fp = NULL;
                fopen_s(&fp, obj->filename, "rb");
			    if(!fp) {printf("Error loading\n"); return -1;}
			    void* texbuf = malloc(obj->width*obj->height*bpp);
			    if(!texbuf) {printf("Error allocating\n"); return -1;}
			    fread(texbuf, obj->width*obj->height*bpp, 1, fp);
			    fclose(fp);
		        glTexImage2D(
		            GL_TEXTURE_2D,
		            0,
		            (obj->type == NENGL_COLOR_FORMAT_ARGB32) ? GL_RGBA : GL_RGB,
		            obj->width,
		            obj->height,
		            0,
		            (obj->type == NENGL_COLOR_FORMAT_ARGB32) ? GL_RGBA : GL_RGB,
		            GL_UNSIGNED_BYTE,
		            texbuf
		            );
		        GL_CHECK(glTexImage2D);
                        free(texbuf);
		    }
		    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST); //GL_LINEAR);
		    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST); //GL_LINEAR);
		    curr_state.textureType[curr_state.num_textures] = NENGL_TEXTURE_2D;
	    }//2d texture
        //Go to the next texture
        obj = ++objArray;
        i++;
        curr_state.num_textures++;
    }
    return 0;
}

int nengl_core::setup_fbo_as_texture_data(NENGL_TEXTURE_OBJ* objArray, unsigned int textureId)
{
    int i = 0;
    int loc;
    NENGL_TEXTURE_OBJ* obj = objArray;

	restore_attribs();
	loc = glGetUniformLocation(curr_state.program, obj->sampler_name);
	if (i > GL_MAX_TEXTURE_IMAGE_UNITS)
	{
	    D_PRINTF("Texture requirements exceed available HW texture units"); 
	    return -1;
	}
	D_PRINTF("Sampler loc = %d for name %s\n", loc, obj->sampler_name);
	GL_CHECK(glGetUniformLocation);
	curr_state.textureID[i] = textureId;
	
    glUniform1i(loc, 0);
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, curr_state.textureID[i]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST); //GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST); //GL_LINEAR);
	curr_state.textureType[curr_state.num_textures] = NENGL_TEXTURE_2D;
    curr_state.num_textures++;
    return 0;
}

int nengl_core::setup_shaders(const char** s, int len, GLenum type, GLuint * obj)
{
    int bShaderCompiled;
    *obj = 0;
    // Loads the vertex shader in the same way
    GLuint shaderObj = glCreateShader(type);
    GL_CHECK(glCreateShader);
    glShaderSource(shaderObj, len, s, NULL);

    glCompileShader(shaderObj);
    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &bShaderCompiled);

    if (GL_FALSE == bShaderCompiled)
    {
        D_PRINTF("Error: compiling shader (type = %d)\n", type);
        char infolog[1000] = {0};
        int length = 999;
        glGetShaderInfoLog(shaderObj, length, &length, infolog);
        D_PRINTF("%s\n", infolog);
        return -1;
    }
    *obj = shaderObj;
    return 0;
}

int nengl_core::setup_shaders_array(const char** v, int lenv, const char** f, int lenf)
{
    GLuint fragshaderObj, vertshaderObj;
    int ret;
    ret = setup_shaders(f, lenf, GL_FRAGMENT_SHADER, &fragshaderObj);
    if(ret) return ret;
    ret=setup_shaders(v, lenv, GL_VERTEX_SHADER, &vertshaderObj);
    if(ret) return ret;

    // Create the shader program
    curr_state.program = glCreateProgram();

    // Attach the fragment and vertex shaders to it
    glAttachShader(curr_state.program, fragshaderObj);
    glAttachShader(curr_state.program, vertshaderObj);

    // Link the program
    glLinkProgram(curr_state.program);

    // Check if linking succeeded in the same way we checked for compilation success
    GLint bLinked;
    glGetProgramiv(curr_state.program, GL_LINK_STATUS, &bLinked);

    if (!bLinked)
    {
        D_PRINTF("Error: linking prog\n");
        char infolog[1000];
        int length = 999;
        glGetProgramInfoLog(curr_state.program, length, &length, infolog);
        D_PRINTF("%s\n", infolog);
        return -1;
    }
    GL_CHECK(glGetProgramiv);
    // Actually use the created program
    glUseProgram(curr_state.program);
    GL_CHECK(glUseProgram);
    //delete unwanted
    glDeleteShader(vertshaderObj);
    GL_CHECK(glDeleteShader);
    glDeleteShader(fragshaderObj);
    GL_CHECK(glDeleteShader);

    return 0;
}
int nengl_core::rotate(float x, float y, float z, const char* matrix)
{
    restore_attribs();
    int loc = glGetUniformLocation(curr_state.program, matrix);
    GL_CHECK(glGetUniformLocation);
    curr_xfm = glm::rotate(curr_xfm, x, glm::vec3(1.0f, 0.0, 0.0));
    curr_xfm = glm::rotate(curr_xfm, y, glm::vec3(0.0, 1.0, 0.0));
    curr_xfm = glm::rotate(curr_xfm, z, glm::vec3(0.0, 0.0, 1.0));
    //Set transform
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(curr_xfm));
    GL_CHECK(glUniformMatrix4fv);
    return 0;
}
int nengl_core::scale(float x, float y, float z, const char* matrix)
{
    restore_attribs();
    curr_xfm = glm::scale(curr_xfm, glm::vec3(x, y, z));
    //Set transform
    glUniformMatrix4fv(glGetUniformLocation(curr_state.program, matrix), 1, GL_FALSE, glm::value_ptr(curr_xfm));
    GL_CHECK(glUniformMatrix4fv);
    return 0;
}
int nengl_core::translate(float x, float y, float z, const char *matrix)
{
    restore_attribs();
    curr_xfm = glm::translate(curr_xfm, glm::vec3(x, y, -z));
    //Set transform
    glUniformMatrix4fv(glGetUniformLocation(curr_state.program, matrix), 1, GL_FALSE, glm::value_ptr(curr_xfm));
    GL_CHECK(glUniformMatrix4fv);
    return 0;
}

int nengl_core::default_transform(NENGL_VIEW_TYPE type)
{
    if (type == NENGL_VIEW_PERSPECTIVE)
        curr_xfm = glm::perspective(30.0f, 4.0f / 3.0f, 0.1f, 50.f);
    else
        curr_xfm = glm::ortho(-10, 10, -10, 10);
    return 0;
}

int nengl_core::alpha(float a)
{
    return 0;
}

int nengl_core::restore_attribs()
{
    int i;
    GL_CHECK("pre"restore_attribs);
    glUseProgram(curr_state.program);
    D_PRINTF("program=%d\n", curr_state.program);
    GL_CHECK(glUseProgram);
    D_PRINTF("num_attribs (not including index) = %d\n", curr_state.num_attribs);
    for (i = 0; i < curr_state.num_attribs; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, curr_state.vboID[i]);
        GL_CHECK(glBindBuffer);
        glVertexAttribPointer(curr_state.attrib_loc[i],
            curr_state.attrib_num_elems[i],
            curr_state.attrib_type[i],
            curr_state.attrib_normalised[i],
            curr_state.attrib_stride[i],
            curr_state.attrib_elem_offset[i]);
        GL_CHECK(glVertexAttribPointer);
        glEnableVertexAttribArray(curr_state.attrib_loc[i]);
	    GL_CHECK(glEnableVertexAttribArray);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, curr_state.vboID[i]);
    GL_CHECK(glBindBuffer);
    for (i = 0; i < curr_state.num_textures; i++)
    {
	    glActiveTexture(GL_TEXTURE0 + i);
	    if(curr_state.textureType[i] == NENGL_TEXTURE_2D)
            glBindTexture(GL_TEXTURE_2D, curr_state.textureID[i]);
	    else
		    glBindTexture(GL_TEXTURE_3D_OES, curr_state.textureID[i]);
    }
    GL_CHECK(restore_attribs);
    return 0;
}

int nengl_core::draw()
{
    restore_attribs();
    if (draw_mode_indexed == true)
    {
        if (curr_state.index_mode == 0) 
            glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_SHORT, 0);
        else 
            glDrawElements(GL_TRIANGLES, numindices, GL_UNSIGNED_INT, 0);
    }
    else //Unsupported currently
        return -1;
    GL_CHECK(glDrawElements);
    return 0;
}
 
int nengl_core::setup_uniform1f(const char* name, float f)
{
    restore_attribs();
    int loc = glGetUniformLocation(curr_state.program, name);
    GL_CHECK(glGetUniformLocation);
    glUniform1f(loc, f);
    GL_CHECK(glUniform1f);
    return 0;
}

int nengl_core::setup_uniform3fv(const char* name, float* f)
{
    restore_attribs();
    int loc = glGetUniformLocation(curr_state.program, name);
    GL_CHECK(glGetUniformLocation);
    glUniform3fv(loc, 3, f);
    GL_CHECK(glUniform3fv);
    return 0;
}