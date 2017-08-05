#include "RenderTool.h"
#include "../../renderer/tr_local.h"

#ifdef USE_SDL2
#if defined(LINUX) || defined(__APPLE__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#endif

bool RenderTool::CreateFrameBuffer(FrameBufferInfo& rInfo, int width, int height)
{
    qglGenTextures(1, &rInfo.DepthBuffer);
    qglBindTexture(GL_TEXTURE_2D, rInfo.DepthBuffer);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    //qglTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    qglGenTextures(1, &rInfo.ColorBuffer);
    qglBindTexture(GL_TEXTURE_2D, rInfo.ColorBuffer);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    qglGenFramebuffers(1, &rInfo.Fbo);
    //glw_state.fbo = 1;
    qglBindFramebuffer(GL_FRAMEBUFFER, rInfo.Fbo);
    qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rInfo.ColorBuffer, 0);
    qglFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, rInfo.DepthBuffer, 0);

    GLenum status = qglCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("...framebuffer initialization failed\n");
        return false;
    }

    rInfo.Width = width;
    rInfo.Height = height;

    ClearFBO(rInfo);

    return true;
}

bool RenderTool::CreateFrameBufferWithoutTextures(FrameBufferInfo& rInfo, int width, int height)
{
    qglGenFramebuffers(1, &rInfo.Fbo);

    rInfo.Width = width;
    rInfo.Height = height;

    return true;
}

void RenderTool::ClearFBO(FrameBufferInfo info)
{
    Clear(info.Width, info.Height);
}

void RenderTool::Clear(int width, int height)
{
    qglViewport(0, 0, width, height);
    qglScissor(0, 0, width, height);
    qglClear(GL_COLOR_BUFFER_BIT);
}


void RenderTool::DrawFbos(FrameBufferInfo* pFbos, int fboCount, int windowWidth, int windowHeight, GLhandleARB shaderProg)
{
    if (pFbos == NULL)
    {
        return;
    }

    // backup the current state
    GLboolean depth_test = qglIsEnabled(GL_DEPTH_TEST);
    GLboolean blend = qglIsEnabled(GL_BLEND);
    GLboolean texture_2d = qglIsEnabled(GL_TEXTURE_2D);
    GLboolean texture_coord_array = qglIsEnabled(GL_TEXTURE_COORD_ARRAY);
    GLboolean color_array = qglIsEnabled(GL_COLOR_ARRAY);
    GLint viewport[4];
    GLint scissor[4];
    GLint texture;
    qglGetIntegerv(GL_VIEWPORT, viewport);
    qglGetIntegerv(GL_SCISSOR_BOX, scissor);
    qglGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);

    // set state
    qglBindFramebuffer(GL_FRAMEBUFFER, 0);

    qglViewport(0,0, windowWidth, windowHeight);
    qglScissor(0,0, windowWidth, windowHeight);

    qglDisable(GL_DEPTH_TEST);
    qglDisable(GL_BLEND);
    qglEnable(GL_TEXTURE_2D);
    qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
    qglDisableClientState(GL_COLOR_ARRAY);

    qglUseProgramObjectARB(shaderProg);

    qglMatrixMode(GL_PROJECTION);
    qglPushMatrix();
    qglLoadIdentity();

    qglMatrixMode(GL_MODELVIEW);
    qglPushMatrix();
    qglLoadIdentity();

    float verts[] =
    {
        -1.f, 1.f, 0.f, 1.f,
        0.f, 1.f, 1.f, 1.f,
        0.f,-1.f, 1.f, 0.f,
        -1.f,-1.f, 0.f, 0.f,
    };

    if (windowHeight > windowWidth)
    {
        // display is rotated
        qglRotatef(90, 0, 0, 1);
    }

    if (fboCount == 1)
    {
        qglScalef(2.0f, 1.0f, 1.0f);
        qglTranslatef(0.5f, 0.f, 0.f);
    }

    qglBindTexture(GL_TEXTURE_2D, pFbos[0].ColorBuffer);
    qglColor4ub(255,255,255,255);
    qglTexCoordPointer(2, GL_FLOAT, 16, verts + 2);
    qglVertexPointer(2, GL_FLOAT, 16, verts);
    qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    if (fboCount >= 2)
    {
        qglTranslatef(1.0f, 0.f, 0.f);
        qglBindTexture(GL_TEXTURE_2D, pFbos[1].ColorBuffer);
        qglColor4ub(255,255,255,255);
        qglTexCoordPointer(2, GL_FLOAT, 16, verts + 2);
        qglVertexPointer(2, GL_FLOAT, 16, verts);
        qglDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }



    qglMatrixMode(GL_PROJECTION);
    qglPopMatrix();

    qglMatrixMode(GL_MODELVIEW);
    qglPopMatrix();


    // restore the old state
    qglUseProgramObjectARB(0);

    if (depth_test)
    {
        qglEnable(GL_DEPTH_TEST);
    }
    if (blend)
    {
        qglEnable(GL_BLEND);
    }
    if (!texture_2d)
    {
        qglDisable(GL_TEXTURE_2D);
    }
    if (!texture_coord_array)
    {
        qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (color_array)
    {
        qglEnableClientState(GL_COLOR_ARRAY);
    }
    qglViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
    qglScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
    qglBindTexture(GL_TEXTURE_2D, texture);
}

GLhandleARB RenderTool::CreateShaderProgram(const char* pVertexShader, const char* pFragmentShader)
{
    if (pVertexShader == NULL || pFragmentShader == NULL)
    {
        return 0;
    }

    GLhandleARB v = qglCreateShaderObjectARB(GL_VERTEX_SHADER);
    GLhandleARB f = qglCreateShaderObjectARB(GL_FRAGMENT_SHADER);


    qglShaderSourceARB(v, 1, &pVertexShader, NULL);
    qglShaderSourceARB(f, 1, &pFragmentShader, NULL);

    qglCompileShaderARB(v);
    // Check Vertex Shader
    //    int result;
    //    glGetShaderiv(v, GL_COMPILE_STATUS, &result);
    //    if (result == GL_FALSE)
    //    {
    //        glGetShaderiv(v, GL_INFO_LOG_LENGTH, &infoLogLength);
    //        std::vector<char> VertexShaderErrorMessage(infoLogLength);
    //        glGetShaderInfoLog(v, infoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //        fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    //    }

    qglCompileShaderARB(f);
    // Check Fragment Shader
    //    glGetShaderiv(f, GL_COMPILE_STATUS, &result);
    //    if (result == GL_FALSE)
    //    {
    //        glGetShaderiv(f, GL_INFO_LOG_LENGTH, &infoLogLength);
    //        std::vector<char> fragmentShaderErrorMessage(infoLogLength);
    //        glGetShaderInfoLog(f, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
    //        fprintf(stdout, "%s\n", &fragmentShaderErrorMessage[0]);
    //    }


    GLhandleARB program = qglCreateProgramObjectARB();
    qglAttachObjectARB(program, f);
    qglAttachObjectARB(program, v);

    qglLinkProgramARB(program);
    // Check the program
    //    glGetProgramiv(mOculusProgram, GL_LINK_STATUS, &result);
    //    if (result == GL_FALSE)
    //    {
    //        glGetProgramiv(mOculusProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
    //        std::vector<char> programErrorMessage( std::max(infoLogLength, int(1)) );
    //        glGetProgramInfoLog(mOculusProgram, infoLogLength, NULL, &programErrorMessage[0]);
    //        fprintf(stdout, "%s\n", &programErrorMessage[0]);
    //    }

    //    flush(stdout);

    return program;
}

void RenderTool::SetVSync(bool active)
{
#ifdef USE_SDL2
    int interval = active ? 1 : 0;
    SDL_GL_SetSwapInterval(interval);
#endif
}