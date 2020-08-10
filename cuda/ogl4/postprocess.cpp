// Taken originally from
// https://www.3dgep.com/opengl-interoperability-with-cuda/
// by Jeremiah van Oosten, 2011
// https://drive.google.com/file/d/0B0ND0J8HHfaXT0p1N3ZkSW5kTVU/edit?usp=sharing

// Updates over the original version:
// - Updated to CUDA 11, 64b, removed unsupported dependencies
// - Moved to use OpenGL 4
// - 2-pass rendering shader controls
// - Separated the GL rendering and CUDA code-base
// - Added debug reads for debugging offscreen rendering
// - Compilable with Visual Studio 2019 142 toolsets


// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;


// CUDA stuff
// CUDA headers
#include <cuda_runtime_api.h>
#include <cuda_gl_interop.h>
#include "cuda-postprocess.h"

#include "debug-gl.h"

using namespace std;


// Prototypes
// Render a texture object to the current framebuffer
void DisplayImage(GLuint texture, GLuint pass1or2Loc);


// Create a framebuffer object that is used for offscreen rendering.
void CreateFramebuffer(GLuint& framebuffer, GLuint colorAttachment0, GLuint depthAttachment);
void DeleteFramebuffer(GLuint& framebuffer);
void CreateTexture(GLuint& texture, unsigned int width, unsigned int height);
void DeleteTexture(GLuint& texture);
void CreateDepthBuffer(GLuint& depthBuffer, unsigned int width, unsigned int height);
void DeleteDepthBuffer(GLuint& depthBuffer);
// Links a OpenGL texture object to a CUDA resource that can be used in the CUDA kernel.
void CreateCUDAResource(cudaGraphicsResource_t& cudaResource, GLuint GLtexture, cudaGraphicsMapFlags mapFlags);
void DeleteCUDAResource(cudaGraphicsResource_t& cudaResource);

#define SRC_BUFFER  0
#define DST_BUFFER  1

bool g_bPostProcess = true;
GLuint g_GLPostprocessTexture = 0;
GLuint g_iImageWidth = 1920;
GLuint g_iImageHeight = 1080;
GLuint g_GLFramebuffer = 0;                  // Frame buffer object for off-screen rendering.
GLuint g_GLColorAttachment0 = 0;            // Color texture to attach to frame buffer object.
GLuint g_GLDepthAttachment = 0;             // Depth buffer to attach to frame buffer object.

// The CUDA Graphics Resource is used to map the OpenGL texture to a CUDA
// buffer that can be used in a CUDA kernel.
// We need 2 resource: One will be used to map to the color attachment of the
//   framebuffer and used read-only from the CUDA kernel (SRC_BUFFER), 
//   the second is used to write the postprocess effect to (DST_BUFFER).
cudaGraphicsResource_t g_CUDAGraphicsResource[2] = { 0, 0 };

// Define a few default filters to apply to the image
// Filter definitions found on http://ian-albert.com/old/custom_filters/

#define SCALE_INDEX 25  // The index of the scale value in the filter
#define OFFSET_INDEX 26 // The index of the offset value in the filter

// Filter matrices are defined by a series of 25 values which represent the weights of the 
// each neighboring pixel followed by the default scale and the offset that is applied to each pixel.
float g_Unfiltered[] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 0
};

float g_BlurFilter[] = {
    1, 1, 1, 1, 1,
    1, 2, 2, 2, 1,
    1, 2, 3, 2, 1,
    1, 2, 2, 2, 2,
    1, 1, 1, 1, 1,
    35, 0
};

float g_SharpeningFilter[] = {
    0,  0,  0,  0,  0,
    0,  0, -1,  0,  0,
    0, -1,  5, -1,  0,
    0,  0, -1,  0,  0,
    0,  0,  0,  0,  0,
    1, 0
};

float g_EmbossFilter[] = {
    0, 0, 0,  0, 0,
    0, 0, 0,  0, 0,
    0, 0, 1,  0, 0,
    0, 0, 0, -1, 0,
    0, 0, 0,  0, 0,
    1, 128
};

float g_InvertFilter[] = {
    0, 0,  0, 0, 0,
    0, 0,  0, 0, 0,
    0, 0, -1, 0, 0,
    0, 0,  0, 0, 0,
    0, 0,  0, 0, 0,
    1, 255
};

float g_EdgeFilter[] = {
    0,  0,  0,  0,  0,
    0, -1, -1, -1,  0,
    0, -1,  8, -1,  0,
    0, -1, -1, -1,  0,
    0,  0,  0,  0,  0,
    1, 0
};

// The current scale
float g_Scale = 1.0f;
// The current offset
float g_Offset = 0.0f;
// The currently selected filter
float* g_CurrentFilter = g_EdgeFilter; // g_Unfiltered;

// Create a texture resource for rendering to.
void CreateTexture(GLuint& texture, unsigned int width, unsigned int height)
{
    // Make sure we don't already have a texture defined here
    DeleteTexture(texture);


    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set basic parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create texture data (4-component unsigned byte)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DeleteTexture(GLuint& texture)
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}

void CreateDepthBuffer(GLuint& depthBuffer, unsigned int width, unsigned int height)
{
    // Delete the existing depth buffer if there is one.
    DeleteDepthBuffer(depthBuffer);

    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // Unbind the depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void DeleteDepthBuffer(GLuint& depthBuffer)
{
    if (depthBuffer != 0)
    {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }
}

void CreateFramebuffer(GLuint& framebuffer, GLuint colorAttachment0, GLuint depthAttachment)
{
    // Delete the existing framebuffer if it exists.
    DeleteFramebuffer(framebuffer);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment0, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);

    // Check to see if the frame buffer is valid
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: Incomplete framebuffer status." << std::endl;
    }

    // Unbind the frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeleteFramebuffer(GLuint& framebuffer)
{
    if (framebuffer != 0)
    {
        glDeleteFramebuffers(1, &framebuffer);
        framebuffer = 0;
    }
}

void CreateCUDAResource(cudaGraphicsResource_t& cudaResource, GLuint GLtexture, cudaGraphicsMapFlags mapFlags)
{
    // Map the GL texture resource with the CUDA resource
    cudaGraphicsGLRegisterImage(&cudaResource, GLtexture, GL_TEXTURE_2D, mapFlags);
}

void DeleteCUDAResource(cudaGraphicsResource_t& cudaResource)
{
    if (cudaResource != 0)
    {
        cudaGraphicsUnregisterResource(cudaResource);
        cudaResource = 0;
    }
}


// To be called atleast once
void ReInitCUDAGL(int w, int h)
{
    h = h > 1 ? h : 1;

    g_iImageWidth = w;
    g_iImageHeight = h;

    // Create a surface texture to render the scene to.
    CreateTexture(g_GLColorAttachment0, g_iImageWidth, g_iImageHeight);
    // Create a depth buffer for the frame buffer object.
    CreateDepthBuffer(g_GLDepthAttachment, g_iImageWidth, g_iImageHeight);
    // Attach the color and depth textures to the framebuffer.
    CreateFramebuffer(g_GLFramebuffer, g_GLColorAttachment0, g_GLDepthAttachment);

    // Create a texture to render the post-process effect to.
    CreateTexture(g_GLPostprocessTexture, g_iImageWidth, g_iImageHeight);

    if (g_bPostProcess)
    {
        // DEPRECATED - Not required anymore - beyond CUDA 5!!
        // We have to call cudaGLSetGLDevice if we want to use OpenGL interoperability.
        cudaGLSetGLDevice(0);
        // Map the color attachment to a CUDA graphics resource so we can read it in a CUDA a kernel.
        CreateCUDAResource(g_CUDAGraphicsResource[SRC_BUFFER],
            g_GLColorAttachment0, cudaGraphicsMapFlagsReadOnly);
        // Map the post-process texture to the CUDA resource so it can be 
        // written in the kernel.
        CreateCUDAResource(g_CUDAGraphicsResource[DST_BUFFER],
            g_GLPostprocessTexture, cudaGraphicsMapFlagsWriteDiscard);
    }
}


void DrawSceneType(int type, GLuint pass1or2Loc, int pass);


// Perform a post-process effect on the current framebuffer (back buffer)
void Postprocess()
{
    if (g_bPostProcess)
    {
        PostprocessCUDA(g_CUDAGraphicsResource[DST_BUFFER], 
            g_CUDAGraphicsResource[SRC_BUFFER], g_iImageWidth, g_iImageHeight, 
            g_CurrentFilter, g_Scale, g_Offset);
    }
    else
    {
        // No postprocess effect. Just copy the contents of the color buffer
        // from the framebuffer (where the scene was rendered) to the 
        // post-process texture.  The postprocess texture will be rendered to the screen
        // in the next step.
        glBindFramebuffer(GL_FRAMEBUFFER, g_GLFramebuffer);

        GL_CHECK(glBindFramebuffer);

#if 0 // This shows correct model is rendered !!
        int sizePixelBuf = g_iImageWidth * g_iImageHeight * 4;
        char* pixels = new char[sizePixelBuf];
        glReadPixels(0, 0, g_iImageWidth, g_iImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        GL_CHECK(glReadPixels);
        FILE* fp = fopen("readpixels.rgba", "wb");
        fwrite(pixels, sizePixelBuf, 1, fp);
        fclose(fp);
        if (pixels) delete[] pixels;
#endif


        glBindTexture(GL_TEXTURE_2D, g_GLPostprocessTexture);
        GL_CHECK(glBindTexture);
        int glrb = 0;
        glGetIntegerv(GL_READ_BUFFER, &glrb); //Returns GL_COLOR_ATTACHMENT0

        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, g_iImageWidth, g_iImageHeight, 0);

        GL_CHECK(glCopyTexImage2D);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GL_CHECK(glBindFramebuffer);
    }
}

void DisplayProcessedScene(GLuint pass1or2Loc)
{
    // Bind the framebuffer that we want to use as the render target.
    glBindFramebuffer(GL_FRAMEBUFFER, g_GLFramebuffer);
    GL_CHECK(glBindFramebuffer);

    glClearColor(0.4f, 0.1f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw original scene
    DrawSceneType(1, pass1or2Loc, 1);

    // Unbind the framebuffer so we render to the back buffer again.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Postprocess();

    // Blit the image full-screen
    DisplayImage(g_GLPostprocessTexture, pass1or2Loc);
}

void DisplayImage(GLuint texture, GLuint pass1or2Loc)
{
    // Clear the screen again for the current framebuffer
    glClearColor(0.2f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, texture);

    DrawSceneType(2, pass1or2Loc, 2);

    glBindTexture(GL_TEXTURE_2D, 0);
}


bool loadCubeObject(
    std::vector<unsigned short>& indices,
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec2>& uvs,
    std::vector<glm::vec3>& normals
) 
{
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

    short index_coords[6 * 2 * 3] = { 0, 1, 2, 0, 2, 3,
                3, 2, 6, 3, 6, 7,
                7, 6, 5, 7, 5, 4,
                4, 5, 1, 4, 1,0,
                0,3,7, 0, 7,4,
                1,2,6,1,6,5 };

    // Fill vertices positions
    int numVertices = 4 * 2;
    vertices.reserve(numVertices);
    for (unsigned int i = 0; i < numVertices; i++) {
        vertices.push_back(glm::vec3(vattribs[i*3], vattribs[i * 3 + 1], vattribs[i * 3 + 2]));
    }

    // Fill vertices texture coordinates
    uvs.reserve(numVertices);
    for (unsigned int i = 0; i < numVertices; i++) {
        uvs.push_back(glm::vec2(texture_coords_2d[i*2], texture_coords_2d[i*2+1]));
    }

    // Fill vertices normals
    normals.reserve(numVertices);
    for (unsigned int i = 0; i < numVertices; i++) {
        normals.push_back(glm::vec3(0,0,0));
    }


    // Fill face indices
    int numFaces = 6 * 2; //cube, each face consisting 2 triangles
    indices.reserve(numFaces * 3);
    for (unsigned int i = 0; i < numFaces; i++) {
        // Assume the model has only triangles.
        indices.push_back(index_coords[i*3]);
        indices.push_back(index_coords[i * 3 + 1]);
        indices.push_back(index_coords[i * 3 + 2]);
    }
    return true;
}