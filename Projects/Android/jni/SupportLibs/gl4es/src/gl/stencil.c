#include "stencil.h"

#include "../glx/hardext.h"
#include "debug.h"
#include "gl4es.h"
#include "glstate.h"
#include "loader.h"

void gl4es_glStencilMask(GLuint mask) {
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glStencilMask);
    LOAD_GLES(glStencilMask);
    if(glstate->stencil.mask[0]==glstate->stencil.mask[1] && glstate->stencil.mask[0]==mask) {
        noerrorShim();
        return;
    }
    if(glstate->list.pending)
        flush();
    glstate->stencil.mask[0] = glstate->stencil.mask[1] = mask;
    errorGL();
    gles_glStencilMask(mask);
}
void glStencilMask(GLuint mask) AliasExport("gl4es_glStencilMask");

void gl4es_glStencilMaskSeparate(GLenum face, GLuint mask) {
    if(face!=GL_FRONT && face!=GL_BACK && face!=GL_FRONT_AND_BACK) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    if(face==GL_FRONT_AND_BACK) {
        gl4es_glStencilMask(mask);
        return;
    }
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glStencilMaskSeparate);
    if((face==GL_FRONT && glstate->stencil.mask[0]==mask) || (face==GL_BACK && glstate->stencil.mask[1]==mask)) {
        noerrorShim();
        return;
    }
    LOAD_GLES2_OR_OES(glStencilMaskSeparate);
    if(glstate->list.pending)
        flush();
    glstate->stencil.mask[(face==GL_FRONT)?0:1] = mask;

    errorGL();
    if(gles_glStencilMaskSeparate) {
        gles_glStencilMaskSeparate(face, mask);
    } else {
        // fake function..., call it only for front or front_and_back, just ignore back (crappy, I know)
        if (face==GL_FRONT)
            gl4es_glStencilMask(mask);
        else
            noerrorShim();
    }
}
void glStencilMaskSeparate(GLenum face, GLuint mask) AliasExport("gl4es_glStencilMaskSeparate");

void gl4es_glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glStencilFunc);
    if(  glstate->stencil.func[0]==glstate->stencil.func[1] && glstate->stencil.func[0]==func
      && glstate->stencil.f_ref[0]==glstate->stencil.f_ref[1] && glstate->stencil.f_ref[0]==ref
      && glstate->stencil.f_mask[0]==glstate->stencil.f_mask[1] && glstate->stencil.f_mask[0]==mask ) {
          noerrorShim();
          return;
      }
    LOAD_GLES(glStencilFunc);
    errorGL();
    if(glstate->list.pending)
        flush();
    glstate->stencil.func[0] = glstate->stencil.func[1] = func;
    glstate->stencil.f_ref[0] = glstate->stencil.f_ref[1] = ref;
    glstate->stencil.f_mask[0] = glstate->stencil.f_mask[1] = mask;
    gles_glStencilFunc(func, ref, mask);
}
void glStencilFunc(GLenum func, GLint ref, GLuint mask) AliasExport("gl4es_glStencilFunc");

void gl4es_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
    if(face!=GL_FRONT && face!=GL_BACK && face!=GL_FRONT_AND_BACK) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    if(face==GL_FRONT_AND_BACK) {
        glStencilFunc(func, ref, mask);
        return;
    }

    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glStencilMaskSeparate);
    int idx = (face==GL_FRONT)?0:1;
    if(glstate->stencil.func[idx]==func && glstate->stencil.f_ref[idx]==ref && glstate->stencil.f_mask[idx]==mask) {
        noerrorShim();
        return;
    }
    LOAD_GLES2_OR_OES(glStencilFuncSeparate);
    errorGL();
    if(glstate->list.pending)
        flush();
    glstate->stencil.func[idx]=func;
    glstate->stencil.f_ref[idx]=ref;
    glstate->stencil.f_mask[idx]=mask;
    if(gles_glStencilFuncSeparate) {
        gles_glStencilFuncSeparate(face, func, ref, mask);
    } else {
        // fake function..., call it only for front or front_and_back, just ignore back (crappy, I know)
        if (face==GL_FRONT)
            gl4es_glStencilFunc(func, ref, mask);
        else
            noerrorShim();
    }
}
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) AliasExport("gl4es_glStencilFuncSeparate");

void gl4es_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glStencilOp);
    if(  glstate->stencil.sfail[0]==glstate->stencil.sfail[1] && glstate->stencil.sfail[0]==fail
      && glstate->stencil.dpfail[0]==glstate->stencil.dpfail[1] && glstate->stencil.dpfail[0]==zfail
      && glstate->stencil.dppass[0]==glstate->stencil.dppass[1] && glstate->stencil.dppass[0]==zpass ) {
          noerrorShim();
          return;
      }
    LOAD_GLES(glStencilOp);
    if(glstate->list.pending)
        flush();
    glstate->stencil.sfail[0] = glstate->stencil.sfail[1] = fail;
    glstate->stencil.dpfail[0] = glstate->stencil.dpfail[1] = zfail;
    glstate->stencil.dppass[0] = glstate->stencil.dppass[1] = zpass;
    errorGL();
    gles_glStencilOp(fail, zfail, zpass);
}
void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) AliasExport("gl4es_glStencilOp");

void gl4es_glStencilOpSeparate(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass) {
    if(face!=GL_FRONT && face!=GL_BACK && face!=GL_FRONT_AND_BACK) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    if(face==GL_FRONT_AND_BACK) {
        glStencilOp(sfail, zfail, zpass);
        return;
    }

    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glStencilOpSeparate);
    int idx = (face==GL_FRONT)?0:1;
    if(glstate->stencil.sfail[idx]==sfail && glstate->stencil.dpfail[idx]==zfail && glstate->stencil.dppass[idx]==zpass) {
        noerrorShim();
        return;
    }
    LOAD_GLES2_OR_OES(glStencilOpSeparate);
    errorGL();
    glstate->stencil.sfail[idx] = sfail;
    glstate->stencil.dpfail[idx] = zfail;
    glstate->stencil.dppass[idx] = zpass;
    if(gles_glStencilOpSeparate) {
        gles_glStencilOpSeparate(face, sfail, zfail, zpass);
    } else {
        //fake, again
        if (face==GL_FRONT)
            gl4es_glStencilOp(sfail, zfail, zpass);
        else
            noerrorShim();
    }
}
void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass) AliasExport("gl4es_glStencilOpSeparate");

void gl4es_glClearStencil(GLint s) {
    if(!glstate->list.pending) 
        PUSH_IF_COMPILING(glClearStencil);
    if(  glstate->stencil.clear==s) {
          noerrorShim();
          return;
      }
    LOAD_GLES(glClearStencil);
    if(glstate->list.pending)
        flush();
    glstate->stencil.clear = s;
    errorGL();
    gles_glClearStencil(s);
}
void glClearStencil(GLint s) AliasExport("gl4es_glClearStencil");
