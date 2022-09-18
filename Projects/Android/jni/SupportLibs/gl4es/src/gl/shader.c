#include "shader.h"

#include "../glx/hardext.h"
#include "debug.h"
#include "init.h"
#include "gl4es.h"
#include "glstate.h"
#include "loader.h"
#include "shaderconv.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

KHASH_MAP_IMPL_INT(shaderlist, shader_t *);

GLuint gl4es_glCreateShader(GLenum shaderType) {
    DBG(printf("glCreateShader(%s)\n", PrintEnum(shaderType));)
    // sanity check
    if (shaderType!=GL_VERTEX_SHADER && shaderType!=GL_FRAGMENT_SHADER) {
        DBG(printf("Invalid shader type\n");)
        errorShim(GL_INVALID_ENUM);
        return 0;
    }
    static GLuint lastshader = 0;
    GLuint shader;
    // create the shader
    LOAD_GLES2(glCreateShader);
    if(gles_glCreateShader) {
        shader = gles_glCreateShader(shaderType);
        if(!shader) {
            DBG(printf("Failed to create shader\n");)
            errorGL();
            return 0;
        }
    } else {
        shader = ++lastshader;
        noerrorShim();
    }
    // store the new empty shader in the list for later use
   	khint_t k;
   	int ret;
	khash_t(shaderlist) *shaders = glstate->glsl->shaders;
    k = kh_get(shaderlist, shaders, shader);
    shader_t *glshader = NULL;
    if (k == kh_end(shaders)){
        k = kh_put(shaderlist, shaders, shader, &ret);
        glshader = kh_value(shaders, k) = malloc(sizeof(shader_t));
        memset(glshader, 0, sizeof(shader_t));
    } else {
        glshader = kh_value(shaders, k);
    }
    glshader->id = shader;
    glshader->type = shaderType;
    if(glshader->source) {
        free(glshader->source);
        glshader->source = NULL;
    }
    glshader->need.need_texcoord = -1;

    // all done
    return shader;
}

void actualy_deleteshader(GLuint shader) {
    khint_t k;
    int ret;
    khash_t(shaderlist) *shaders = glstate->glsl->shaders;
    k = kh_get(shaderlist, shaders, shader);
    if (k != kh_end(shaders)) {
        shader_t *glshader = kh_value(shaders, k);
        if(glshader->deleted && !glshader->attached) {
            kh_del(shaderlist, shaders, k);
            if(glshader->source)
                free(glshader->source);
            if(glshader->converted)
                free(glshader->converted);
            free(glshader);
        }
    }
}

void actualy_detachshader(GLuint shader) {
    khint_t k;
    int ret;
    khash_t(shaderlist) *shaders = glstate->glsl->shaders;
    k = kh_get(shaderlist, shaders, shader);
    if (k != kh_end(shaders)) {
        shader_t *glshader = kh_value(shaders, k);
        if((--glshader->attached)<1 && glshader->deleted)
            actualy_deleteshader(shader); 
    }
}

void gl4es_glDeleteShader(GLuint shader) {
    DBG(printf("glDeleteShader(%d)\n", shader);)
    // sanity check...
    CHECK_SHADER(void, shader)
    // delete the shader from the list
    if(!glshader) {
        noerrorShim();
        return;
    }
    glshader->deleted = 1;
    noerrorShim();
    if(!glshader->attached) {
        actualy_deleteshader(shader);

        // delete the shader in GLES2 hardware (if any)
        LOAD_GLES2(glDeleteShader);
        if(gles_glDeleteShader) {
            errorGL();
            gles_glDeleteShader(shader);
        }   
    }
}

void gl4es_glCompileShader(GLuint shader) {
    DBG(printf("glCompileShader(%d)\n", shader);)
    // look for the shader
    CHECK_SHADER(void, shader)

    glshader->compiled = 1;
    LOAD_GLES2(glCompileShader);
    if(gles_glCompileShader) {
        gles_glCompileShader(glshader->id);
        errorGL();
        if(globals4es.logshader) {
            // get compile status and print shaders sources if compile fail...
            LOAD_GLES2(glGetShaderiv);
            LOAD_GLES2(glGetShaderInfoLog);
            GLint status = 0;
            gles_glGetShaderiv(glshader->id, GL_COMPILE_STATUS, &status);
            if(status!=GL_TRUE) {
                printf("LIBGL: Error while compiling shader %d. Original source is:\n%s\n=======\n", glshader->id, glshader->source);
                printf("ShaderConv Source is:\n%s\n=======\n", glshader->converted);
                char tmp[500];
                GLint length;
                gles_glGetShaderInfoLog(glshader->id, 500, &length, tmp);
                printf("Compiler message is\n%s\nLIBGL: End of Error log\n", tmp);
            }
        }
    } else
        noerrorShim();
}

void gl4es_glShaderSource(GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length) {
    DBG(printf("glShaderSource(%d, %d, %p, %p)\n", shader, count, string, length);)
    // sanity check
    if(count<=0) {
        errorShim(GL_INVALID_VALUE);
        return;
    }
    CHECK_SHADER(void, shader)
    // get the size of the shader sources and than concatenate in a single string
    int l = 0;
    for (int i=0; i<count; i++) l+=(length && length[i] >= 0)?length[i]:strlen(string[i]);
    if(glshader->source) free(glshader->source);
    glshader->source = malloc(l+1);
    memset(glshader->source, 0, l+1);
    if(length) {
        for (int i=0; i<count; i++) {
            if(length[i] >= 0)
                strncat(glshader->source, string[i], length[i]);
            else
                strcat(glshader->source, string[i]);
        }
    } else {
        for (int i=0; i<count; i++)
            strcat(glshader->source, string[i]);
    }
    LOAD_GLES2(glShaderSource);
    if (gles_glShaderSource) {
        // adapt shader if needed (i.e. not an es2 context and shader is not #version 100)
        if(!glstate->glsl->es2 && strncmp(glshader->source, "#version 100", 12))
            glshader->converted = ConvertShader(glshader->source, glshader->type==GL_VERTEX_SHADER?1:0, &glshader->need);
        // send source to GLES2 hardware if any
        gles_glShaderSource(shader, 1, (const GLchar * const*)((glshader->converted)?(&glshader->converted):(&glshader->source)), NULL);
        errorGL();
    } else
        noerrorShim();
}

void accumShaderNeeds(GLuint shader, shaderconv_need_t *need) {
    CHECK_SHADER(void, shader)
    if(!glshader->converted) 
        return;
    #define GO(A) if(need->need_##A < glshader->need.need_##A) need->need_##A = glshader->need.need_##A
    GO(color);
    GO(secondary);
    GO(fogcoord);
    GO(texcoord);
    GO(normalmatrix);
    GO(mvmatrix);
    GO(mvpmatrix);
    #undef GO
}
int isShaderCompatible(GLuint shader, shaderconv_need_t *need) {
    CHECK_SHADER(int, shader)
    if(!glshader->converted)
        return 0;
    #define GO(A) if(need->need_##A > glshader->need.need_##A) return 0;
    GO(color);
    GO(secondary);
    GO(fogcoord);
    GO(texcoord);
    GO(normalmatrix);
    GO(mvmatrix);
    GO(mvpmatrix);
    #undef GO
    return 1;
}
void redoShader(GLuint shader, shaderconv_need_t *need) {
    LOAD_GLES2(glShaderSource);
    if(!gles_glShaderSource)
        return;
    CHECK_SHADER(void, shader)
    if(!glshader->converted)
        return;
    // test, if no changes, no need to reconvert & recompile...
    if (memcmp(&glshader->need, need, sizeof(shaderconv_need_t))==0)
        return;
    free(glshader->converted);
    memcpy(&glshader->need, need, sizeof(shaderconv_need_t));
    glshader->converted = ConvertShader(glshader->source, glshader->type==GL_VERTEX_SHADER?1:0, &glshader->need);
    // send source to GLES2 hardware if any
    gles_glShaderSource(shader, 1, (const GLchar * const*)((glshader->converted)?(&glshader->converted):(&glshader->source)), NULL);
    // recompile...
    gl4es_glCompileShader(glshader->id);
}

void gl4es_glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) {
    DBG(printf("glGetShaderSource(%d, %d, %p, %p)\n", shader, bufSize, length, source);)
    // find shader
    CHECK_SHADER(void, shader)
    if (bufSize<=0) {
        errorShim(GL_INVALID_OPERATION);
        return;
    }
    // if no source, then it's an empty string
    if(glshader->source==NULL) {
        noerrorShim();
        if(length) *length = 0;
        source[0] = '\0';
        return;
    }
    // copy concatenated sources
    GLsizei size = strlen(glshader->source);
    if (size+1>bufSize) size = bufSize-1;
    strncpy(source, glshader->source, size);
    source[size]='\0';
    if(length) *length=size;
    noerrorShim();
}

GLboolean gl4es_glIsShader(GLuint shader) {
    DBG(printf("glIsShader(%d)\n", shader);)
    // find shader
    shader_t *glshader = NULL;
    khint_t k;
    {
        int ret;
        khash_t(shaderlist) *shaders = glstate->glsl->shaders;
        k = kh_get(shaderlist, shaders, shader);
        if (k != kh_end(shaders))
            glshader = kh_value(shaders, k);
    }
    return (glshader)?GL_TRUE:GL_FALSE;
}

static const char* GLES_NoGLSLSupport = "No Shader support with current backend";

void gl4es_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {
    DBG(printf("glGetShaderInfoLog(%d, %d, %p, %p)\n", shader, maxLength, length, infoLog);)
    // find shader
    CHECK_SHADER(void, shader)
    if(maxLength<=0) {
        errorShim(GL_INVALID_OPERATION);
        return;
    }
    LOAD_GLES2(glGetShaderInfoLog);
    if(gles_glGetShaderInfoLog) {
        gles_glGetShaderInfoLog(glshader->id, maxLength, length, infoLog);
        errorGL();
    } else {
        strncpy(infoLog, GLES_NoGLSLSupport, maxLength);
        if(length) *length = strlen(infoLog);
    }
}

void gl4es_glGetShaderiv(GLuint shader, GLenum pname, GLint *params) {
    DBG(printf("glGetShaderiv(%d, %s, %p)\n", shader, PrintEnum(pname), params);)
    // find shader
    CHECK_SHADER(void, shader)
    LOAD_GLES2(glGetShaderiv);
    noerrorShim();
    switch (pname) {
        case GL_SHADER_TYPE:
            *params = glshader->type;
            break;
        case GL_DELETE_STATUS:
            *params = (glshader->deleted)?GL_TRUE:GL_FALSE;
            break;
        case GL_COMPILE_STATUS:
            if(gles_glGetShaderiv) {
                gles_glGetShaderiv(glshader->id, pname, params);
                errorGL();
            } else {
                *params = GL_FALSE; // stub, compile always fail
            }
            break;
        case GL_INFO_LOG_LENGTH:
            if(gles_glGetShaderiv) {
                gles_glGetShaderiv(glshader->id, pname, params);
                errorGL();
            } else {
                *params = strlen(GLES_NoGLSLSupport); // stub, compile always fail
            }
            break;
        case GL_SHADER_SOURCE_LENGTH:
            if(glshader->source)
                *params = strlen(glshader->source)+1;
            else
                *params = 0;
            break;
        default:
            errorShim(GL_INVALID_ENUM);
    }
}

void gl4es_glGetShaderPrecisionFormat(GLenum shaderType, GLenum precisionType, GLint *range, GLint *precision) {
    LOAD_GLES2(glGetShaderPrecisionFormat);
    if(gles_glGetShaderPrecisionFormat) {
        gles_glGetShaderPrecisionFormat(shaderType, precisionType, range, precision);
        errorGL();
    } else {
        errorShim(GL_INVALID_ENUM);
    }
}

void gl4es_glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length) {
    // TODO: check consistancy of "shaders" values
    LOAD_GLES2(glShaderBinary);
    if (gles_glShaderBinary) {
        gles_glShaderBinary(count, shaders, binaryFormat, binary, length);
        errorGL();
    } else {
        errorShim(GL_INVALID_ENUM);
    }
}

void gl4es_glReleaseShaderCompiler(void) {
    LOAD_GLES2(glReleaseShaderCompiler);
    if(gles_glReleaseShaderCompiler) {
        gles_glReleaseShaderCompiler();
        errorGL();
    } else
        noerrorShim();
}

// ========== GL_ARB_shader_objects ==============

GLuint glCreateShader(GLenum shaderType) AliasExport("gl4es_glCreateShader");
void glDeleteShader(GLuint shader) AliasExport("gl4es_glDeleteShader");
void glCompileShader(GLuint shader) AliasExport("gl4es_glCompileShader");
void glShaderSource(GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length) AliasExport("gl4es_glShaderSource");
void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) AliasExport("gl4es_glGetShaderSource");
GLboolean glIsShader(GLuint shader) AliasExport("gl4es_glIsShader");
void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) AliasExport("gl4es_glGetShaderInfoLog");
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params) AliasExport("gl4es_glGetShaderiv");
void glGetShaderPrecisionFormat(GLenum shaderType, GLenum precisionType, GLint *range, GLint *precision) AliasExport("gl4es_glGetShaderPrecisionFormat");
void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length) AliasExport("gl4es_glShaderBinary");
void glReleaseShaderCompiler(void) AliasExport("gl4es_glReleaseShaderCompiler");


GLhandleARB gl4es_glCreateShaderObject(GLenum shaderType) {
    return gl4es_glCreateShader(shaderType);
}

GLhandleARB glCreateShaderObjectARB(GLenum shaderType) AliasExport("gl4es_glCreateShaderObject");
GLvoid glShaderSourceARB(GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length) AliasExport("gl4es_glShaderSource");
GLvoid glCompileShaderARB(GLhandleARB shaderObj) AliasExport("gl4es_glCompileShader");
GLvoid glGetShaderSourceARB(GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source) AliasExport("gl4es_glGetShaderSource");
