#include "face.h"

#include "loader.h"
#include "gl4es.h"
#include "glstate.h"

void gl4es_glCullFace(GLenum mode) {
    if(!glstate->list.pending)
        PUSH_IF_COMPILING(glCullFace);
    if(mode!=GL_FRONT && mode!=GL_BACK && mode!=GL_FRONT_AND_BACK) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    if(glstate->face.cull == mode) {
        noerrorShim();
        return;
    }
    if(glstate->list.pending)
        flush();
    
    glstate->face.cull = mode;
    LOAD_GLES(glCullFace);
    gles_glCullFace(mode);
}

void gl4es_glFrontFace(GLenum mode) {
    if(!glstate->list.pending)
        PUSH_IF_COMPILING(glFrontFace);
    if(mode!=GL_CW && mode!=GL_CCW) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    if(glstate->face.front == mode) {
        noerrorShim();
        return;
    }
    if(glstate->list.pending)
        flush();
    
    glstate->face.front = mode;
    LOAD_GLES(glFrontFace);
    gles_glFrontFace(mode);
}


void glCullFace(GLenum mode) AliasExport("gl4es_glCullFace");
void glFrontFace(GLenum mode) AliasExport("gl4es_glFrontFace");
