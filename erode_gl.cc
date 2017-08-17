#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
 
#include "erode_gl.h"

#ifndef RED_EXT
#define RED_EXT                 0x1903
#endif

                       // positions        texture coords
GLfloat vVertices[] = {-1.0f,  3.0f, 0.0f, 0.0f, 2.0f,
                       -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                        3.0f, -1.0f, 0.0f, 2.0f, 0.0f};

const char *vertexSource = "\n" \
"attribute vec4 aPosition;  \n" \
"attribute vec2 aTexture;   \n" \
"varying vec2 vTexture;     \n" \
"void main()                \n" \
"{                          \n" \
"  vTexture = aTexture;     \n" \
"  gl_Position = aPosition; \n" \
"}";

const char *fragmentSource =               "\n" \
"precision mediump float;                   \n" \
"varying vec2 vTexture;                     \n" \
"uniform sampler2D sTexture;                \n" \
"uniform float offset_x;                    \n" \
"uniform float offset_y;                    \n" \
"void main()                                \n" \
"{                                          \n" \
"  float color = texture2D(sTexture, vTexture).r; \n" \
"  float c; \n" \
"  c = texture2D(sTexture, vTexture + vec2(-offset_x, 0.0)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(offset_x, 0.0)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(-offset_x, offset_y)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(0.0, offset_y)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(offset_x, offset_y)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(-offset_x, -offset_y)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(0.0, -offset_y)).r; \n" \
"  color = min(c, color); \n" \
"  c = texture2D(sTexture, vTexture + vec2(offset_x, -offset_y)).r; \n" \
"  color = min(c, color); \n" \
"  gl_FragColor = vec4(color, 0.0, 0.0, 1.0); \n" \
"}";

#define CHECK_GL_ERROR() { GLenum err (glGetError()); if (err != GL_NO_ERROR) {std::cout << "glError @ " << __LINE__ << ": " << err << std::endl; exit (1);}}

static EGLDisplay display = EGL_NO_DISPLAY;
static EGLConfig config;
static EGLContext context;
static EGLSurface surface;
static GLuint framebuffer;
static GLuint renderedTexture;
static GLuint inputTexture;
static GLuint programObject;
static bool redOnlyTextureSupport;

static GLint uniformOffsetX;
static GLint uniformOffsetY;

static GLint attribPos;
static GLint attribTex;

///
// Create a shader object, load the shader source, and
// compile the shader.
//
static GLuint
LoadShader(const char *shaderSrc, GLenum type)
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);
    if (shader == 0)
        return 0;
    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);
    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            std::cout << "Error compiling shader: " << std::endl;
            std::cout << infoLog << std::endl;
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static void
initGLProgram()
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = LoadShader(vertexSource, GL_VERTEX_SHADER);
    fragmentShader = LoadShader(fragmentSource, GL_FRAGMENT_SHADER);

    // Create the program object
    programObject = glCreateProgram();
    if (programObject == 0) {
        std::cout << "Cannot create program" << std::endl;
        exit(1);
    }
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    // Bind vPosition to attribute 0
    glBindAttribLocation(programObject, 0, "vPosition");
    // Link the program
    glLinkProgram(programObject);
    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        std::cout << "Error linking program:" << std::endl;
        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            std::cout << infoLog << std::endl;

            free(infoLog);
        }
        glDeleteProgram(programObject);
        exit(1);
    }

    uniformOffsetX = glGetUniformLocation(programObject, "offset_x");
    uniformOffsetY = glGetUniformLocation(programObject, "offset_y");

    attribPos = glGetAttribLocation(programObject, "aPosition");
    attribTex = glGetAttribLocation(programObject, "aTexture");
}

int
erode3x3_gl_init(int w, int h)
{
    EGLint majorVersion, minorVersion;
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, &majorVersion, &minorVersion);

    std::cout << "EGL initialized. Version: " << majorVersion << "." << minorVersion << std::endl;

    EGLint numConfigs = -1;
    EGLint const configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    eglChooseConfig(display, configAttribs, 0, 0, &numConfigs);
    eglBindAPI(EGL_OPENGL_ES_API);

    if (numConfigs <= 0) {
        std::cout << "No config available" << std::endl;
        exit(1);
    }

    // just use the first available config
    EGLint n;
    EGLConfig* const configs = new EGLConfig[numConfigs];
    eglChooseConfig(display, configAttribs, configs, numConfigs, &n);

    std::cout << "Num of configs matching: " << n << std::endl;
    // list out all config
    for (int i = 0; i < n; i++) {
        EGLint bufferSize, redSize, greenSize, blueSize, luminanceSize, alphaSize, alphaMaskSize;
        EGLint bindToTextureRGB, bindToTextureRGBA, colorBufferType, configCaveat, configId, conformant;
        EGLint depthSize, level, maxSwapInterval, minSwapInterval, nativeRenderable;
        EGLint nativeVisualType, renderableType, sampleBuffers, samples, stencilSize, surfaceType;
        EGLint transparentType, transparentRedValue, transparentGreenValue, transparentBlueValue;

        eglGetConfigAttrib(display, configs[i], EGL_BUFFER_SIZE, &bufferSize);
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &redSize);
        eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &greenSize);
        eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &blueSize);
        eglGetConfigAttrib(display, configs[i], EGL_LUMINANCE_SIZE, &luminanceSize);
        eglGetConfigAttrib(display, configs[i], EGL_ALPHA_SIZE, &alphaSize);
        eglGetConfigAttrib(display, configs[i], EGL_ALPHA_MASK_SIZE, &alphaMaskSize);
        eglGetConfigAttrib(display, configs[i], EGL_BIND_TO_TEXTURE_RGB, &bindToTextureRGB);
        eglGetConfigAttrib(display, configs[i], EGL_BIND_TO_TEXTURE_RGBA, &bindToTextureRGBA);
        eglGetConfigAttrib(display, configs[i], EGL_COLOR_BUFFER_TYPE, &colorBufferType);
        eglGetConfigAttrib(display, configs[i], EGL_CONFIG_CAVEAT, &configCaveat);
        eglGetConfigAttrib(display, configs[i], EGL_CONFIG_ID, &configId);
        eglGetConfigAttrib(display, configs[i], EGL_CONFORMANT, &conformant);
        eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depthSize);
        eglGetConfigAttrib(display, configs[i], EGL_LEVEL, &level);
        eglGetConfigAttrib(display, configs[i], EGL_MAX_SWAP_INTERVAL, &maxSwapInterval);
        eglGetConfigAttrib(display, configs[i], EGL_MIN_SWAP_INTERVAL, &minSwapInterval);
        eglGetConfigAttrib(display, configs[i], EGL_NATIVE_RENDERABLE, &nativeRenderable);
        eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_TYPE, &nativeVisualType);
        eglGetConfigAttrib(display, configs[i], EGL_RENDERABLE_TYPE, &renderableType);
        eglGetConfigAttrib(display, configs[i], EGL_SAMPLE_BUFFERS, &sampleBuffers);
        eglGetConfigAttrib(display, configs[i], EGL_SAMPLES, &samples);
        eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &stencilSize);
        eglGetConfigAttrib(display, configs[i], EGL_SURFACE_TYPE, &surfaceType);
        eglGetConfigAttrib(display, configs[i], EGL_TRANSPARENT_TYPE, &transparentType);
        eglGetConfigAttrib(display, configs[i], EGL_TRANSPARENT_RED_VALUE, &transparentRedValue);
        eglGetConfigAttrib(display, configs[i], EGL_TRANSPARENT_GREEN_VALUE, &transparentGreenValue);
        eglGetConfigAttrib(display, configs[i], EGL_TRANSPARENT_BLUE_VALUE, &transparentBlueValue);

        std::cout << "[" << i << "] Config:" << std::endl;
        std::cout << "  Sizes: " << bufferSize << "[" << redSize << "," << greenSize << "," << blueSize << "]";
        std::cout << "[" << luminanceSize << "," << alphaSize << "," << alphaMaskSize << "]" << std::endl;
        std::cout << "  Bind: [" << bindToTextureRGB << "," << bindToTextureRGBA << "] BufferType: [" << colorBufferType << "] ";
        std::cout << "Config: [" << configCaveat << "," << configId << "," << conformant << "]" << std::endl;
        std::cout << "  Depth: [" << depthSize << "]" << "Level: [" << level << "]" << std::endl;
        std::cout << "  Swap interval: [" << maxSwapInterval << "," << minSwapInterval << "] ";
        std::cout << "Native: [" << nativeRenderable << "," << nativeVisualType << "]" << std::endl;
        std::cout << "  Renderable: [" << renderableType << "] Samples: [" << sampleBuffers << "," << samples << "]" << std::endl;
        std::cout << "  Stencil: [" << stencilSize << "], Surface: [" << surfaceType << "]" << std::endl;
        std::cout << "  Tranparent: " << transparentType << "[" << transparentRedValue << "," << transparentGreenValue << "," << transparentBlueValue << "]" << std::endl;
    }

    config = configs[0];
    delete[] configs;

    EGLint context_attribs2[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs2);

    EGLint pbuffer_attribs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    surface = eglCreatePbufferSurface(display, config, pbuffer_attribs);

    if (surface == EGL_NO_SURFACE) {
        std::cout << "No surface available" << std::endl;
        exit(1);
    }

    eglMakeCurrent(display, surface, surface, context);

    const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    std::cout << "Extension support: " << extensions << std::endl;

    // Create a texture for processing
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // The texture we're going to render to
    glGenTextures(1, &renderedTexture);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    
    // Give an empty image to OpenGL ( the last "0" )
    redOnlyTextureSupport = std::strstr(extensions, "GL_EXT_texture_rg");
    if (redOnlyTextureSupport) {
        glTexImage2D(GL_TEXTURE_2D, 0, RED_EXT, w, h, 0, RED_EXT, GL_UNSIGNED_BYTE, 0);
    } else {
        std::cout << "Warning no red component only framebuffer support, opengl cannot use" << std::endl;
        // TODO return error
        exit(1);
    }

    if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT ) {
        std::cout << "attach texture failed" << std::endl;
        exit(1);
    }
    
    CHECK_GL_ERROR();

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    CHECK_GL_ERROR();

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

    CHECK_GL_ERROR();

    // Generate input texture for upload input data to gpu
    glGenTextures(1, &inputTexture);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
    
    // We dont need filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Repeat edge values
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    initGLProgram();

    return 0; 
}

void
erode3x3_gl(uint8_t *in_data, uint8_t *out_data, int w, int h)
{
    // upload the data to the gpu texture (TODO if ES3.0 can use mapbuffer?)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_LUMINANCE, GL_UNSIGNED_BYTE, in_data);

    // Set the viewport
    glViewport(0, 0, w, h);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(programObject);

    // Load the vertex data
    glVertexAttribPointer(attribPos, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, vVertices);
    glEnableVertexAttribArray(attribPos);

    // Load the texture coords
    glVertexAttribPointer(attribTex, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, vVertices+3);
    glEnableVertexAttribArray(attribTex);

    // set the pixel offset
    glUniform1f(uniformOffsetX, 1.0f / w );
    glUniform1f(uniformOffsetY, 1.0f / h );

    glDrawArrays(GL_TRIANGLES, 0, 3);
    CHECK_GL_ERROR();

    // read back buffer from GPU framebuffer
    glReadPixels(0, 0, w, h, RED_EXT, GL_UNSIGNED_BYTE, out_data);

    CHECK_GL_ERROR();
}

void
erode3x3_gl_destroy()
{
    // TODO
}
