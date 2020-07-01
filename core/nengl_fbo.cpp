/*******************************************************************
* 
* NENGL - Learn OpenGL ESx in a modular way
*
* Simple class to encapsulate a 32bit FBO
* 
*******************************************************************/
#include "nengl_fbo.h"

nengl_fbo::nengl_fbo(int width, int height)
{
	fbo_id = fbo_texture_id = 0;
	this->height = height ;
	this->width = width ;
	//create fbo
	glGenFramebuffers(1, &fbo_id);
	//create empty texture
	glGenTextures(1, &fbo_texture_id);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, fbo_texture_id);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		0
	);
	GL_CHECK(glTexImage2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	set();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture_id, 0);
	GL_CHECK(glFramebufferTexture2D);
	//check completeness
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) 
	{
		D_PRINTF("ERROR: FBO incomplete %x\n", status);
	}
}

nengl_fbo::~nengl_fbo()
{
	unset();
	glDeleteTextures(1, &fbo_texture_id);
	glDeleteFramebuffers(1, &fbo_id);
}

//Set as target
int nengl_fbo::set()
{
	if(fbo_id==0) return -1;
	if(fbo_texture_id==0) return -1;
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
	GL_CHECK(glBindFramebuffer);
	return 0;
}
//Ubset as target
int nengl_fbo::unset()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GL_CHECK(glBindFramebuffer);
	return 0;
}
//get the content drawn
int nengl_fbo::get_reference_id()
{
	return fbo_texture_id;
}

int nengl_fbo::get_width()
{
	return width;
}
int nengl_fbo::get_height()
{
	return height;
}
