#ifndef RENDERTOOL_H
#define RENDERTOOL_H

#include <stdio.h>
#include "../../renderer/qgl.h"


class RenderTool
{
public:
    struct FrameBufferInfo
    {
        GLuint  Fbo;
        GLuint  DepthBuffer;
        GLuint  ColorBuffer;
        int Width;
        int Height;
    };


    static bool CreateFrameBuffer(FrameBufferInfo& rInfo, int width, int height);
    static bool CreateFrameBufferWithoutTextures(FrameBufferInfo& rInfo, int width, int height);
    static void ClearFBO(FrameBufferInfo info);
    static void Clear(int width, int height);
    static void DrawFbos(FrameBufferInfo* pFbos, int fboCount, int windowWidth, int windowHeight, GLhandleARB shaderProg = 0);
    static void SetVSync(bool active);

    static GLhandleARB CreateShaderProgram(const char* pVertexShader, const char* pFragmentShader);

};

#endif
