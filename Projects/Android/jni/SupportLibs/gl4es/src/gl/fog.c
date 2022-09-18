#include "fog.h"

#include "../glx/hardext.h"
#include "fpe.h"
#include "gl4es.h"
#include "glstate.h"
#include "loader.h"
#include "matrix.h"
#include "matvec.h"

void gl4es_glFogfv(GLenum pname, const GLfloat* params) {

    if (glstate->list.active)
        if (glstate->list.compiling) {
                NewStage(glstate->list.active, STAGE_FOG);
                rlFogOp(glstate->list.active, pname, params);
                return;
            }
        else flush();
    noerrorShim();
    #define GO(A,name, size) if(memcmp(A glstate->fog.name, params, size)==0) return; else memcpy(A glstate->fog.name, params, size);
    switch (pname) {
        case GL_FOG_MODE:
            GO(&, mode, sizeof(GLfloat))
            break;
        case GL_FOG_DENSITY:
            if(*params<0.f) {
                errorShim(GL_INVALID_VALUE);
                return;
            }
            GO(&, density, sizeof(GLfloat))
            break;
        case GL_FOG_START:
            GO(&, start, sizeof(GLfloat))
            break;
        case GL_FOG_END:
            GO(&, end, sizeof(GLfloat))
            break;
        case GL_FOG_INDEX:
            GO(&, index, sizeof(GLfloat))
            return; // unsupported for now
        case GL_FOG_COLOR:
#if defined(__GNUC__) && (__GNUC__ > 7)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
            // GCC 8.1 warn about reading 16 bytes from a 4 bytes value, but params is not (in that case) a 4 bytes value
            GO(, color, 4*sizeof(GLfloat))
#if defined(__GNUC__) && (__GNUC__ > 7)
#pragma GCC diagnostic pop
#endif
            break;
        case GL_FOG_COORD_SRC:
            GO(&, coord_src, sizeof(GLfloat))
            if(hardext.esversion==1)
                return; // unsupported on GLES1.1
            break;
        default:
            errorShim(GL_INVALID_ENUM);
            return;
    }
    #undef GO
    LOAD_GLES_FPE(glFogfv);
    gles_glFogfv(pname, params);
    errorGL();
}

void gl4es_glFogf(GLenum pname, GLfloat param) {
    gl4es_glFogfv(pname, &param);
}

void gl4es_glFogCoordd(GLdouble coord) {
    gl4es_glFogCoordf(coord);
}
void gl4es_glFogCoorddv(const GLdouble *coord) {
    gl4es_glFogCoordf(*coord);
}
void gl4es_glFogCoordfv(const GLfloat *coord) {
    gl4es_glFogCoordf(*coord);
}
void gl4es_glFogCoordf(GLfloat coord) {
    if (glstate->list.active) {
        if(glstate->list.pending)
            flush();
        else
        {
            rlFogCoordf(glstate->list.active, coord);
            glstate->list.active->lastFogCoord = coord;
        }
        noerrorShim();
    } else {
        noerrorShim();
    }
    // change the state last thing
    glstate->fogcoord = coord;
}


void glFogfv(GLenum pname, const GLfloat* params) AliasExport("gl4es_glFogfv");
void glFogf(GLenum pname, GLfloat param) AliasExport("gl4es_glFogf");
void glFogCoordd(GLdouble coord) AliasExport("gl4es_glFogCoordd");
void glFogCoorddv(const GLdouble *coord) AliasExport("gl4es_glFogCoorddv");
void glFogCoordf(GLfloat coord) AliasExport("gl4es_glFogCoordf");
void glFogCoordfv(const GLfloat *coord) AliasExport("gl4es_glFogCoordfv");

void glFogfvEXT(GLenum pname, const GLfloat* params) AliasExport("gl4es_glFogfv");
void glFogfEXT(GLenum pname, GLfloat param) AliasExport("gl4es_glFogf");
void glFogCoorddEXT(GLdouble coord) AliasExport("gl4es_glFogCoordd");
void glFogCoorddvEXT(const GLdouble *coord) AliasExport("gl4es_glFogCoorddv");
void glFogCoordfEXT(GLfloat coord) AliasExport("gl4es_glFogCoordf");
void glFogCoordfvEXT(const GLfloat *coord) AliasExport("gl4es_glFogCoordfv");