// Simple render to texture class
//
// Author: Simon Green
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.

#ifndef RENDER_TEXTURE_FBO__H
#define RENDER_TEXTURE_FBO__H

// replacement of RenderTexture class in RenderTexture.h.
// this version of the class uses FBO instead of pbuffer for render
// to texture implementation.

class RenderTexture {
public:
	RenderTexture(int w, int h, GLenum target = GL_TEXTURE_RECTANGLE_NV, int samples=0, int coverageSamples=0):
        m_width(w), m_height(h), m_target(target), m_samples(samples), m_coverageSamples(coverageSamples),
        m_tex_depth(0), m_rb_depth(0)
	{
        int i;
        for(i = 0; i < num_col_buffers; i++) {
            m_rb_col[i] = 0;
            m_tex_col[i] = 0;
        }
        glGenFramebuffers(1, &m_fb);
    }

    ~RenderTexture()
	{
        int i;
        for(i = 0; i < num_col_buffers; i++) {
            if (m_tex_col[i]) { glDeleteTextures(1, &m_tex_col[i]); }
            if (m_rb_col[i]) { glDeleteRenderbuffers(1, &m_rb_col[i]); }
        }
        if(m_tex_depth) { glDeleteTextures(1, &m_tex_depth); }
		if(m_rb_depth) { glDeleteRenderbuffers(1, &m_rb_depth); }

		glDeleteFramebuffers(1, &m_fb);
	}

    // In order to use a color buffer, either
    // InitColor_RB or InitColor_Tex needs to be called.
    void InitColor_RB(int index = 0, GLenum iformat = GL_FLOAT_RGBA16_NV)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb); 
        {
            glGenRenderbuffers(1, &m_rb_col[index]);
            glBindRenderbuffer(GL_RENDERBUFFER, m_rb_col[index]);
			if (m_samples > 0) {
				if ((m_coverageSamples > 0) && GLEW_NV_framebuffer_multisample_coverage) {
                    glRenderbufferStorageMultisampleCoverageNV(
                        GL_RENDERBUFFER, m_coverageSamples, m_samples, iformat, m_width, m_height);
                } else {
			        glRenderbufferStorageMultisample(
                        GL_RENDERBUFFER, m_samples, iformat, m_width, m_height);
                }
			} else {
				glRenderbufferStorage(GL_RENDERBUFFER, iformat, m_width, m_height);
			}
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_COLOR_ATTACHMENT0 + index, GL_RENDERBUFFER, m_rb_col[index]);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb); 
    }

    void InitColor_Tex(int index = 0, GLenum iformat = GL_FLOAT_RGBA16_NV)
    {
		glGenTextures(1, &m_tex_col[index]);
		glBindTexture(m_target, m_tex_col[index]);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(m_target, 0, iformat, m_width, m_height, 0,
                GL_RGBA, GL_INT, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0 + index, m_target, m_tex_col[index], 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void InitColor_None()
    {
        // turn the color buffer off in case this is a z only fbo
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb); 
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }

    // In order to use a depth buffer, either
    // InitDepth_RB or InitDepth_Tex needs to be called.
    void InitDepth_RB(GLenum iformat = GL_DEPTH_COMPONENT24)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb); 
        {
            glGenRenderbuffers(1, &m_rb_depth);
            glBindRenderbuffer(GL_RENDERBUFFER, m_rb_depth);
			if (m_samples > 0) {
				if ((m_coverageSamples > 0) && GLEW_NV_framebuffer_multisample_coverage) {
					glRenderbufferStorageMultisampleCoverageNV(GL_RENDERBUFFER, m_coverageSamples, m_samples, iformat, m_width, m_height);
                } else {
				    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, iformat, m_width, m_height);
                }
			} else {
				glRenderbufferStorage(GL_RENDERBUFFER, iformat, m_width, m_height);
			}
            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                    GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rb_depth);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb); 
    }

    void InitDepth_Tex(GLenum iformat = GL_DEPTH_COMPONENT24)
    {
		glGenTextures(1, &m_tex_depth);
		glBindTexture(m_target, m_tex_depth);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(m_target, 0, iformat, m_width, m_height, 0,
                GL_DEPTH_COMPONENT, GL_INT, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT, m_target, m_tex_depth, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    // Activate / deactivate the FBO as a render target
    // The FBO needs to be deactivated when using the associated textures.
	void Activate()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb); 
#if _DEBUG
		CheckFramebufferStatus();
#endif
	}
	void Deactivate() 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Bind the internal textures
	void BindColor(int index = 0) { glBindTexture(m_target, m_tex_col[index]); }
	inline void Bind(int index = 0) { BindColor(index); }
        // aliased to BindColor.  this reduces app code changes while migrating
        // from the pbuffer implementation.
	void BindDepth() { glBindTexture(m_target, m_tex_depth); }
	void Release() { glBindTexture(m_target, 0); }

    // Get the dimention of the surface
    inline int GetWidth() { return m_width; }
    inline int GetHeight() { return m_height; }

    // Get the internal texutre object IDs.
    inline GLenum GetColorTex(int index = 0) { return m_tex_col[index]; }
    inline GLenum GetDepthTex() { return m_tex_depth; }

    // Get the target texture format (texture2d or texture_rectangle)
    inline GLenum GetTarget() { return m_target; }

    inline GLuint GetFramebuffer() { return m_fb; }

private:
    void CheckFramebufferStatus()
    {
        GLenum status;
        status = (GLenum) glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch(status) {
            case GL_FRAMEBUFFER_COMPLETE:
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                printf("Unsupported framebuffer format\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                printf("Framebuffer incomplete attachment\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                printf("Framebuffer incomplete, missing attachment\n");
                break;
            /*case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                printf("Framebuffer incomplete, attached images must have same dimensions\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
                printf("Framebuffer incomplete, attached images must have same format\n");
                break;*/
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                printf("Framebuffer incomplete, missing draw buffer\n");
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                printf("Framebuffer incomplete, missing read buffer\n");
                break;
            default:
                printf("Unknown error %d\n", status);
        }
    }

    const static int num_col_buffers = 16;

    int m_width, m_height;
	GLenum m_target;
	int m_samples;	        // 0 if not multisampled
    int m_coverageSamples;  // for CSAA
    GLuint m_fb;
    GLuint m_tex_col[num_col_buffers], m_rb_col[num_col_buffers];
    GLuint m_tex_depth, m_rb_depth;
};

#endif // RENDER_TEXTURE_FBO__H
