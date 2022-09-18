#include "gl4es.h"

#if defined(AMIGAOS4) || (defined(NOX11) && defined(NOEGL))
#include <sys/time.h>
#endif // defined(AMIGAOS4) || (defined(NOX11) && defined(NOEGL))

#include "../config.h"
#include "../glx/hardext.h"
#include "wrap/gl4es.h"
#include "array.h"
#include "debug.h"
#include "enum_info.h"
#include "fpe.h"
#include "framebuffers.h"
#include "glstate.h"
#include "init.h"
#include "loader.h"
#include "matrix.h"

int adjust_vertices(GLenum mode, int nb) {
    switch (mode) {
        case GL_POINTS:
            return nb;
        case GL_LINES: // 2 points per elements
            return nb-(nb%2);
        case GL_LINE_STRIP: // at least 2 points
        case GL_LINE_LOOP:
            return (nb>1)?nb:0;
        case GL_TRIANGLES:  // 3 points per elements
            return nb-(nb%3);
        case GL_TRIANGLE_FAN:
        case GL_TRIANGLE_STRIP: // at least 3 points
            return (nb>2)?nb:0;
        case GL_QUADS:  // 4 points per elements
            return nb-(nb%4);
        case GL_QUAD_STRIP: // at least 4, the 2 per elements
            return (nb>4)?(nb-(nb%2)):0;
        case GL_POLYGON:   // at least 3
            return (nb>2)?nb:0;
        default:
            return nb;  // meh?
    }
}

#undef client_state
#define clone_gl_pointer(t, s)\
    t.size = s; t.type = type; t.stride = stride; t.pointer = pointer + (uintptr_t)((glstate->vao->vertex)?glstate->vao->vertex->data:0)
void gl4es_glVertexPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    if(size<1 || size>4) {
        errorShim(GL_INVALID_VALUE);
		return;
    }
    noerrorShim();
    clone_gl_pointer(glstate->vao->pointers[ATT_VERTEX], size);
}
void gl4es_glColorPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
	if (!((size>0 && size<=4) || (size==GL_BGRA && type==GL_UNSIGNED_BYTE))) {
        errorShim(GL_INVALID_VALUE);
		return;
    }
    noerrorShim();
    clone_gl_pointer(glstate->vao->pointers[ATT_COLOR], size);
}
void gl4es_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {
    noerrorShim();
    clone_gl_pointer(glstate->vao->pointers[ATT_NORMAL], 3);
}
void gl4es_glTexCoordPointer(GLint size, GLenum type,
                     GLsizei stride, const GLvoid *pointer) {
    if(size<1 || size>4) {
        errorShim(GL_INVALID_VALUE);
		return;
    }
    noerrorShim();
    clone_gl_pointer(glstate->vao->pointers[ATT_MULTITEXCOORD0+glstate->texture.client], size);
}
void gl4es_glSecondaryColorPointer(GLint size, GLenum type, 
					GLsizei stride, const GLvoid *pointer) {
	if (!(size==3 || (size==GL_BGRA && type==GL_UNSIGNED_BYTE))) {
        errorShim(GL_INVALID_VALUE);
		return;		// Size must be 3...
    }
    clone_gl_pointer(glstate->vao->pointers[ATT_SECONDARY], size);
    noerrorShim();
}
void gl4es_glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer) {
    if(type==1 && stride==GL_FLOAT) {
        type = GL_FLOAT;
        stride = 0; // mistake found in some version of openglide...
    }
    clone_gl_pointer(glstate->vao->pointers[ATT_FOGCOORD], 1);
    noerrorShim();
}

#undef clone_gl_pointer

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glVertexPointer");
void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glColorPointer");
void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glNormalPointer");
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glTexCoordPointer");
void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glSecondaryColorPointer");
void glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glFogCoordPointer");
void glSecondaryColorPointerEXT(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glSecondaryColorPointer");
void glFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glFogCoordPointer");


void gl4es_glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer) {
    uintptr_t ptr = (uintptr_t)pointer;
    // element lengths
    GLsizei tex=0, color=0, normal=0, vert=0;
    // element formats
    GLenum tf, cf, nf, vf;
    tf = cf = nf = vf = GL_FLOAT;
    noerrorShim();
    switch (format) {
        case GL_V2F: vert = 2; break;
        case GL_V3F: vert = 3; break;
        case GL_C4UB_V2F:
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 2;
            break;
        case GL_C4UB_V3F:
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 3;
            break;
        case GL_C3F_V3F:
            color = 3;
            vert = 4;
            break;
        case GL_N3F_V3F:
            normal = 3;
            vert = 3;
            break;
        case GL_C4F_N3F_V3F:
            color = 4;
            normal = 3;
            vert = 3;
            break;
        case GL_T2F_V3F:
            tex = 2;
            vert = 3;
            break;
        case GL_T4F_V4F:
            tex = 4;
            vert = 4;
            break;
        case GL_T2F_C4UB_V3F:
            tex = 2;
            color = 4; cf = GL_UNSIGNED_BYTE;
            vert = 3;
            break;
        case GL_T2F_C3F_V3F:
            tex = 2;
            color = 3;
            vert = 3;
            break;
        case GL_T2F_N3F_V3F:
            tex = 2;
            normal = 3;
            vert = 3;
            break;
        case GL_T2F_C4F_N3F_V3F:
            tex = 2;
            color = 4;
            normal = 3;
            vert = 3;
            break;
        case GL_T4F_C4F_N3F_V4F:
            tex = 4;
            color = 4;
            normal = 3;
            vert = 4;
            break;
        default:
            errorShim(GL_INVALID_ENUM);
            return;
    }
    if (!stride)
        stride = tex * gl_sizeof(tf) +
                 color * gl_sizeof(cf) +
                 normal * gl_sizeof(nf) +
                 vert * gl_sizeof(vf);
    if (tex) {
		gl4es_glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        gl4es_glTexCoordPointer(tex, tf, stride, (GLvoid *)ptr);
        ptr += tex * gl_sizeof(tf);
    }
    if (color) {
		gl4es_glEnableClientState(GL_COLOR_ARRAY);
        gl4es_glColorPointer(color, cf, stride, (GLvoid *)ptr);
        ptr += color * gl_sizeof(cf);
    }
    if (normal) {
		gl4es_glEnableClientState(GL_NORMAL_ARRAY);
        gl4es_glNormalPointer(nf, stride, (GLvoid *)ptr);
        ptr += normal * gl_sizeof(nf);
    }
    if (vert) {
		gl4es_glEnableClientState(GL_VERTEX_ARRAY);
        gl4es_glVertexPointer(vert, vf, stride, (GLvoid *)ptr);
    }
}
void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer) AliasExport("gl4es_glInterleavedArrays");

// immediate mode functions
void gl4es_glBegin(GLenum mode) {
    glstate->list.begin = 1;
    if (!glstate->list.active)
        glstate->list.active = alloc_renderlist();
    // small optim... continue a render command if possible
    glstate->list.active = NewDrawStage(glstate->list.active, mode);
    glstate->list.pending = 0;
    noerrorShim();	// TODO, check Enum validity
}
void glBegin(GLenum mode) AliasExport("gl4es_glBegin");

void gl4es_glEnd() {
    if (!glstate->list.active) return;
    glstate->list.begin = 0;
    // check if TEXTUREx is activate and no TexCoord (or texgen), in that case, create a dummy one base on glstate->..
    for (int a=0; a<hardext.maxtex; a++)
		if ((hardext.esversion==1) && glstate->enable.texture[a] && ((glstate->list.active->tex[a]==0) && !(glstate->enable.texgen_s[a] || glstate->texture.pscoordreplace[a])))
			rlMultiTexCoord4f(glstate->list.active, GL_TEXTURE0+a, glstate->texcoord[a][0], glstate->texcoord[a][1], glstate->texcoord[a][2], glstate->texcoord[a][3]);
    rlEnd(glstate->list.active); // end the list now
    // render if we're not in a display list
    int withColor = 0;
    if(glstate->list.compiling) {
        glstate->list.active = extend_renderlist(glstate->list.active);
    } else {
        if (!globals4es.beginend /*|| (glstate->polygon_mode==GL_LINE)*/) {
            renderlist_t *mylist = glstate->list.active;
            withColor = (mylist->color!=NULL);
            glstate->list.active = NULL;
            mylist = end_renderlist(mylist);
            draw_renderlist(mylist);
            free_renderlist(mylist);
        } else {
            withColor = (glstate->list.active->color!=NULL);
            glstate->list.pending = 1;
            NewStage(glstate->list.active, STAGE_POSTDRAW);
        }
    }
    if(withColor)
        gl4es_glColor4f(glstate->color[0], glstate->color[1], glstate->color[2], glstate->color[3]);
    noerrorShim();
}
void glEnd() AliasExport("gl4es_glEnd");

void gl4es_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
    if (glstate->list.active) {
        if (glstate->list.active->stage != STAGE_DRAW) {
            if (glstate->list.compiling && glstate->list.active) {
                glstate->list.active->lastNormal[0] = nx; glstate->list.active->lastNormal[1] = ny; glstate->list.active->lastNormal[2] = nz;
            } else if (glstate->list.pending && glstate->list.active->stage==STAGE_POSTDRAW) {
                glstate->list.active->post_normals[0] = nx; glstate->list.active->post_normals[1] = ny;
                glstate->list.active->post_normals[2] = nz;
                glstate->list.active->post_normal = 1;
                return;                
            }

            PUSH_IF_COMPILING(glNormal3f);
        } else {
            rlNormal3f(glstate->list.active, nx, ny, nz);
            glstate->list.active->lastNormal[0] = nx; glstate->list.active->lastNormal[1] = ny; glstate->list.active->lastNormal[2] = nz;
            noerrorShim();
        }
    }
    else {
        LOAD_GLES_FPE(glNormal3f);
        errorGL();
        gles_glNormal3f(nx, ny, nz);
    }
    glstate->normal[0] = nx; glstate->normal[1] = ny; glstate->normal[2] = nz;
}
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) AliasExport("gl4es_glNormal3f");

void gl4es_glNormal3fv(GLfloat* v) {
    if (glstate->list.active) {
        if (glstate->list.active->stage != STAGE_DRAW) {
            if (glstate->list.compiling && glstate->list.active) {
                memcpy(glstate->list.active->lastNormal, v, 3*sizeof(GLfloat));
            } else if (glstate->list.pending && glstate->list.active->stage==STAGE_POSTDRAW) {
                memcpy(glstate->list.active->post_normals, v, 3*sizeof(GLfloat));
                glstate->list.active->post_normal = 1;
                return;                
            }

            if (!glstate->list.pending)
                return gl4es_glNormal3f(v[0], v[1], v[2]);  // this will put the call on the stack in the current list
        } else {
            rlNormal3fv(glstate->list.active, v);
            memcpy(glstate->list.active->lastNormal, v, 3*sizeof(GLfloat));
            noerrorShim();
        }
    }
    else {
        LOAD_GLES_FPE(glNormal3f);
        errorGL();
        gles_glNormal3f(v[0], v[1], v[2]);
    }
    memcpy(glstate->normal, v, 3*sizeof(GLfloat));
}
void glNormal3fv(GLfloat* v) AliasExport("gl4es_glNormal3fv");

void gl4es_glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (glstate->list.active) {
        rlVertex4f(glstate->list.active, x, y, z, w);
        noerrorShim();
    } else {
        glstate->vertex[0]=x; glstate->vertex[1]=y; glstate->vertex[2]=z; glstate->vertex[3]=w;
    }
}
void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) AliasExport("gl4es_glVertex4f");

void gl4es_glVertex3fv(GLfloat* v) {
    if (glstate->list.active) {
        rlVertex3fv(glstate->list.active, v);
        noerrorShim();
    } else {
        memcpy(glstate->vertex, v, 3*sizeof(GLfloat));
        glstate->vertex[3]=1.f;
    }
}
void glVertex3fv(GLfloat* v) AliasExport("gl4es_glVertex3fv");

void gl4es_glVertex4fv(GLfloat* v) {
    if (glstate->list.active) {
        rlVertex4fv(glstate->list.active, v);
        noerrorShim();
    } else {
        memcpy(glstate->vertex, v, 3*sizeof(GLfloat));
        glstate->vertex[3]=1.f;
    }
}
void glVertex4fv(GLfloat* v) AliasExport("gl4es_glVertex4fv");

void gl4es_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    if (glstate->list.active) {
        if (glstate->list.active->stage != STAGE_DRAW) {
            if (glstate->list.compiling || glstate->list.active->stage<STAGE_DRAW) {
                glstate->list.active->lastColors[0] = red; glstate->list.active->lastColors[1] = green;
                glstate->list.active->lastColors[2] = blue; glstate->list.active->lastColors[3] = alpha;
                glstate->list.active->lastColorsSet = 1;
            }
            else if (glstate->list.pending && glstate->list.active->stage==STAGE_POSTDRAW) {
                glstate->list.active->post_colors[0] = red; glstate->list.active->post_colors[1] = green;
                glstate->list.active->post_colors[2] = blue; glstate->list.active->post_colors[3] = alpha;
                glstate->list.active->post_color = 1;
                return;
            }
            PUSH_IF_COMPILING(glColor4f);
        } else {
            rlColor4f(glstate->list.active, red, green, blue, alpha);
            noerrorShim();
        }
    } else {
        LOAD_GLES_FPE(glColor4f);
        errorGL();
        gles_glColor4f(red, green, blue, alpha);
    }
    // change the state last thing
    glstate->color[0] = red; glstate->color[1] = green;
    glstate->color[2] = blue; glstate->color[3] = alpha;
}
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) AliasExport("gl4es_glColor4f");

void gl4es_glColor4fv(GLfloat* v) {
    if (glstate->list.active) {
        if (glstate->list.active->stage != STAGE_DRAW) {
            if (glstate->list.compiling || glstate->list.active->stage<STAGE_DRAW) {
                memcpy(glstate->list.active->lastColors, v, 4*sizeof(GLfloat));
                glstate->list.active->lastColorsSet = 1;
            }
            else if (glstate->list.pending && glstate->list.active->stage==STAGE_POSTDRAW) {
                memcpy(glstate->list.active->post_colors, v, 4*sizeof(GLfloat));
                glstate->list.active->post_color = 1;
                return;
            }
            if (!glstate->list.pending)
                return gl4es_glColor4f(v[0], v[1], v[2], v[3]);
        } else {
            rlColor4fv(glstate->list.active, v);
            noerrorShim();
        }
    } else {
        LOAD_GLES_FPE(glColor4f);
        errorGL();
        gles_glColor4f(v[0], v[1], v[2], v[3]);
    }
    // change the state last thing
    memcpy(glstate->color, v, 4*sizeof(GLfloat));
}
void glColor4fv(GLfloat* v) AliasExport("gl4es_glColor4fv");

void gl4es_glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b) {
    if (glstate->list.active) {
        if(glstate->list.pending)
            flush();
        else
        {
            rlSecondary3f(glstate->list.active, r, g, b);
            glstate->list.active->lastSecondaryColors[0] = r; glstate->list.active->lastSecondaryColors[1] = g;
            glstate->list.active->lastSecondaryColors[2] = b;
        }
        noerrorShim();
    } else {
        noerrorShim();
    }
    // change the state last thing
    glstate->secondary[0] = r; glstate->secondary[1] = g;
    glstate->secondary[2] = b;
}
void glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b) AliasExport("gl4es_glSecondaryColor3f");
void glSecondaryColor3fEXT(GLfloat r, GLfloat g, GLfloat b) AliasExport("gl4es_glSecondaryColor3f");


void gl4es_glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
    if (glstate->list.active) {
        if(glstate->list.pending)
            flush();
        else {
            // test if called between glBegin / glEnd but Texture is not active and not using a program. In that case, ignore the call
            if(hardext.esversion==1 || glstate->glsl->program || (glstate->list.begin && (glstate->list.compiling || glstate->enable.texture[0])))
                rlMultiTexCoord4f(glstate->list.active, GL_TEXTURE0, s, t, r, q);
        }
    }
    noerrorShim();
    glstate->texcoord[0][0] = s; glstate->texcoord[0][1] = t;
    glstate->texcoord[0][2] = r; glstate->texcoord[0][3] = q;
}
void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q) AliasExport("gl4es_glTexCoord4f");

void gl4es_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) {
	// TODO, error if target is unsuported texture....
    if (glstate->list.active) {
        if(glstate->list.pending)
            flush();
        else {
            // test if called between glBegin / glEnd but Texture is not active. In that case, ignore the call
            if(hardext.esversion==1 || (glstate->list.begin && (glstate->list.compiling || glstate->enable.texture[target-GL_TEXTURE0])))
                rlMultiTexCoord4f(glstate->list.active, target, s, t, r, q);
        }
    }
    noerrorShim();
    glstate->texcoord[target-GL_TEXTURE0][0] = s; glstate->texcoord[target-GL_TEXTURE0][1] = t;
    glstate->texcoord[target-GL_TEXTURE0][2] = r; glstate->texcoord[target-GL_TEXTURE0][3] = q;
}
void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) AliasExport("gl4es_glMultiTexCoord4f");
void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) AliasExport("gl4es_glMultiTexCoord4f");

void gl4es_glMultiTexCoord2fv(GLenum target, GLfloat* v) {
	// TODO, error if target is unsuported texture....
    if (glstate->list.active) {
        if(glstate->list.pending)
            flush();
        else {
            // test if called between glBegin / glEnd but Texture is not active. In that case, ignore the call
            if(hardext.esversion==1 || (glstate->list.begin && (glstate->list.compiling || glstate->enable.texture[target-GL_TEXTURE0])))
                rlMultiTexCoord2fv(glstate->list.active, target, v);
        }
    }
    noerrorShim();
    memcpy(glstate->texcoord[target-GL_TEXTURE0], v, 2*sizeof(GLfloat));
    glstate->texcoord[target-GL_TEXTURE0][2] = 0.f; glstate->texcoord[target-GL_TEXTURE0][3] = 1.f;
}
void glMultiTexCoord2fv(GLenum target, GLfloat* v) AliasExport("gl4es_glMultiTexCoord2fv");
void glMultiTexCoord2fvARB(GLenum target, GLfloat* v) AliasExport("gl4es_glMultiTexCoord2fv");

void gl4es_glMultiTexCoord4fv(GLenum target, GLfloat* v) {
	// TODO, error if target is unsuported texture....
    if (glstate->list.active) {
        if(glstate->list.pending)
            flush();
        else {
            // test if called between glBegin / glEnd but Texture is not active. In that case, ignore the call
            if(hardext.esversion==1 || (glstate->list.begin && (glstate->list.compiling || glstate->enable.texture[target-GL_TEXTURE0])))
                rlMultiTexCoord4fv(glstate->list.active, target, v);
        }
    }
    noerrorShim();
    memcpy(glstate->texcoord[target-GL_TEXTURE0], v, 4*sizeof(GLfloat));
}
void glMultiTexCoord4fv(GLenum target, GLfloat* v) AliasExport("gl4es_glMultiTexCoord4fv");
void glMultiTexCoord4fvARB(GLenum target, GLfloat* v) AliasExport("gl4es_glMultiTexCoord4fv");

void gl4es_glArrayElement(GLint i) {
    GLfloat *v;
    pointer_state_t *p;
    glvao_t* vao = glstate->vao;
    int stride, size;
    p = &vao->pointers[ATT_COLOR];
    if (p->enabled) {
        size = p->size; stride = p->stride;
        // special fast case for easy stuff...
        if(p->type==GL_FLOAT) {
            if(stride)
                v = (GLfloat*)(((uintptr_t)p->pointer)+i*stride);
            else
                v = ((GLfloat*)p->pointer)+i*size;
            if(size==3)
                gl4es_glColor3fv(v);
            else
                gl4es_glColor4fv(v);
        } else if(p->type==GL_UNSIGNED_BYTE) {
            GLubyte *b;
            if(stride)
                b = (GLubyte*)(((uintptr_t)p->pointer)+i*stride);
            else
                b = ((GLubyte*)p->pointer)+i*size;
            if(size==3)
                gl4es_glColor3ubv(b);
            else
                gl4es_glColor4ubv(b);
        } else {
            v = gl_pointer_index(p, i);
            GLfloat scale = 1.0f/gl_max_value(p->type);
            // color[3] defaults to 1.0f
            if (size < 4)
                v[3] = 1.0f;

            // scale color coordinates to a 0 - 1.0 range
            for (int i = 0; i < size; i++) {
                v[i] *= scale;
            }
            gl4es_glColor4fv(v);
        }
    }
    p = &vao->pointers[ATT_SECONDARY];
    if (p->enabled) {
        v = gl_pointer_index(p, i);
        GLfloat scale = 1.0f/gl_max_value(p->type);

        // scale color coordinates to a 0 - 1.0 range
        for (int i = 0; i < p->size; i++) {
            v[i] *= scale;
        }
        gl4es_glSecondaryColor3fv(v);
    }
    p = &vao->pointers[ATT_NORMAL];
    if (p->enabled) {
        // special fast case for easy stuff...
        if(p->type==GL_FLOAT) {
            size = p->size; stride = p->stride;
            if(stride)
                v = (GLfloat*)(((uintptr_t)p->pointer)+i*stride);
            else
                v = ((GLfloat*)p->pointer)+i*size;
        } else {
            v = gl_pointer_index(p, i);
        }
        gl4es_glNormal3fv(v);
    }
    p = &vao->pointers[ATT_MULTITEXCOORD0];
    if (p->enabled) {
        size = p->size; stride = p->stride;
        // special fast case for easy stuff...
        if(p->type==GL_FLOAT) {
            if(stride)
                v = (GLfloat*)(((uintptr_t)p->pointer)+i*stride);
            else
                v = ((GLfloat*)p->pointer)+i*size;
        } else {
            v = gl_pointer_index(p, i);
        }
        if (size<4)
            gl4es_glTexCoord2fv(v);
        else
            gl4es_glTexCoord4fv(v);
    }
    for (int a=1; a<vao->maxtex; a++) {
        p = &vao->pointers[ATT_MULTITEXCOORD0+a];
	    if (p->enabled) {
            size = p->size; stride = p->stride;
            // special fast case for easy stuff...
            if(p->type==GL_FLOAT) {
            if(p->stride)
                v = (GLfloat*)(((uintptr_t)p->pointer)+i*p->stride);
            else
                v = ((GLfloat*)p->pointer)+i*p->size;
            } else {
                v = gl_pointer_index(p, i);
            }
            if (p->size<4)
                gl4es_glMultiTexCoord2fv(GL_TEXTURE0+a, v);
            else
                gl4es_glMultiTexCoord4fv(GL_TEXTURE0+a, v);
	    }
    }
    p = &vao->pointers[ATT_VERTEX];
    if (p->enabled) {
        // special fast case for easy stuff...
        if(p->type==GL_FLOAT) {
            if(p->stride)
                v = (GLfloat*)(((uintptr_t)p->pointer)+i*p->stride);
            else
                v = ((GLfloat*)p->pointer)+i*p->size;
        } else {
            v = gl_pointer_index(p, i);
        }
        if (p->size == 4) {
            gl4es_glVertex4fv(v);
        } else if (p->size == 3) {
            gl4es_glVertex3fv(v);
        } else {
            gl4es_glVertex2fv(v);
        }
    }
}
void glArrayElement(GLint i) AliasExport("gl4es_glArrayElement");
void glArrayElementEXT(GLint i) AliasExport("gl4es_glArrayElement");

// TODO: between a lock and unlock, I can assume the array pointers are unchanged
// so I can build a renderlist_t on the first call and hold onto it
// maybe I need a way to call a renderlist_t with (first, count)
void gl4es_glLockArrays(GLint first, GLsizei count) {
    glstate->vao->locked = true;
    glstate->vao->first = first;
    glstate->vao->count = count;
    noerrorShim();
}
void glLockArraysEXT(GLint first, GLsizei count) AliasExport("gl4es_glLockArrays");
void gl4es_glUnlockArrays() {
    glstate->vao->locked = false;

    noerrorShim();
}
void glUnlockArraysEXT() AliasExport("gl4es_glUnlockArrays");
// display lists

static renderlist_t *gl4es_glGetList(GLuint list) {
    khint_t k;
    int ret;
    khash_t(gllisthead) *lists = glstate->headlists;
    k = kh_get(gllisthead, lists, list);
    if (k != kh_end(lists))
        return kh_value(lists, k);
    return NULL;
}

GLuint gl4es_glGenLists(GLsizei range) {
	if (range<0) {
		errorShim(GL_INVALID_VALUE);
		return 0;
	}
	noerrorShim();
   	khint_t k;
   	int ret;
	khash_t(gllisthead) *lists = glstate->headlists;
    int start = glstate->list.count;
    glstate->list.count += range;

    for (int i = 0; i < range; i++) {
        k = kh_get(gllisthead, lists, start+i);
        if (k == kh_end(lists)){
            k = kh_put(gllisthead, lists, start+i, &ret);
            kh_value(lists, k) = NULL;  // create an empty gllist
        }
    }
    return start + 1;
}
GLuint glGenLists(GLsizei range) AliasExport("gl4es_glGenLists");


void gl4es_glNewList(GLuint list, GLenum mode) {
	errorShim(GL_INVALID_VALUE);
	if (list==0)
		return;
    {
        khint_t k;
        int ret;
        khash_t(gllisthead) *lists = glstate->headlists;
        k = kh_get(gllisthead, lists, list);
        if (k == kh_end(lists)){
            k = kh_put(gllisthead, lists, list, &ret);
            kh_value(lists, k) = NULL;
        }
    }
    noerrorShim();

    glstate->list.name = list;
    glstate->list.mode = mode;
    // TODO: if glstate->list.active is already defined, we probably need to clean up here
    glstate->list.active = alloc_renderlist();
    glstate->list.compiling = true;
}
void glNewList(GLuint list, GLenum mode) AliasExport("gl4es_glNewList");

void gl4es_glEndList() {
	noerrorShim();
    GLuint list = glstate->list.name;
    khash_t(gllisthead) *lists = glstate->headlists;
    khint_t k;
    {
        int ret;
        k = kh_get(gllisthead, lists, list);
        if (k == kh_end(lists)){
            k = kh_put(gllisthead, lists, list, &ret);
            kh_value(lists, k) = NULL;
        }
    }
    if (glstate->list.compiling) {
	// Free the previous list if it exist...
        free_renderlist(kh_value(lists, k));
        kh_value(lists, k) = GetFirst(glstate->list.active);
        glstate->list.compiling = false;
        end_renderlist(glstate->list.active);
        glstate->list.active = NULL;

        if (glstate->list.mode == GL_COMPILE_AND_EXECUTE) {
            glCallList(list);
        }
    }
}
void glEndList() AliasExport("gl4es_glEndList");

renderlist_t* append_calllist(renderlist_t *list, renderlist_t *a);
void gl4es_glCallList(GLuint list) {
	noerrorShim();
    if (glstate->list.active) {
        glstate->list.active = append_calllist(glstate->list.active, gl4es_glGetList(list));
		return;
	}
    // TODO: the output of this call can be compiled into another display list
    renderlist_t *l = gl4es_glGetList(list);
    if (l)
        draw_renderlist(l);
}
void glCallList(GLuint list) AliasExport("gl4es_glCallList");

void glPushCall(void *call) {
    if (glstate->list.active) {
		NewStage(glstate->list.active, STAGE_GLCALL);
        rlPushCall(glstate->list.active, call);
    }
}

void gl4es_glCallLists(GLsizei n, GLenum type, const GLvoid *lists) {
    #define call(name, type) \
        case name: glCallList(((type *)lists)[i] + glstate->list.base); break

    // seriously wtf
    #define call_bytes(name, stride)                             \
        case name:                                               \
            l = (GLubyte *)lists;                                \
            list = 0;                                            \
            for (j = 0; j < stride; j++) {                       \
                list += *(l + (i * stride + j)) << (stride - j); \
            }                                                    \
            gl4es_glCallList(list + glstate->list.base);                  \
            break

    unsigned int i, j;
    GLuint list;
    GLubyte *l;
    for (i = 0; i < n; i++) {
        switch (type) {
            call(GL_BYTE, GLbyte);
            call(GL_UNSIGNED_BYTE, GLubyte);
            call(GL_SHORT, GLshort);
            call(GL_UNSIGNED_SHORT, GLushort);
            call(GL_INT, GLint);
            call(GL_UNSIGNED_INT, GLuint);
            call(GL_FLOAT, GLfloat);
            call_bytes(GL_2_BYTES, 2);
            call_bytes(GL_3_BYTES, 3);
            call_bytes(GL_4_BYTES, 4);
        }
    }
    #undef call
    #undef call_bytes
}
void glCallLists(GLsizei n, GLenum type, const GLvoid *lists) AliasExport("gl4es_glCallLists");

void gl4es_glDeleteList(GLuint list) {

    renderlist_t *gllist = NULL;
    {
        khint_t k;
        int ret;
        khash_t(gllisthead) *lists = glstate->headlists;
        k = kh_get(gllisthead, lists, list);
        renderlist_t *gllist = NULL;
        if (k != kh_end(lists)){
            gllist = kh_value(lists, k);
            free_renderlist(gllist);
            kh_del(gllisthead, lists, k);
        }
    }
}

void gl4es_glDeleteLists(GLuint list, GLsizei range) {
	noerrorShim();
    for (int i = 0; i < range; i++) {
        gl4es_glDeleteList(list+i);
    }
}
void glDeleteLists(GLuint list, GLsizei range) AliasExport("gl4es_glDeleteLists");

void gl4es_glListBase(GLuint base) {
	noerrorShim();
    glstate->list.base = base;
}
void glListBase(GLuint base) AliasExport("gl4es_glListBase");

GLboolean gl4es_glIsList(GLuint list) {
	noerrorShim();
    khint_t k;
    int ret;
    khash_t(gllisthead) *lists = glstate->headlists;
    k = kh_get(gllisthead, lists, list);
    if (k != kh_end(lists))
        return true;
    return false;
}
GLboolean glIsList(GLuint list) AliasExport("gl4es_glIsList");

void gl4es_glPolygonMode(GLenum face, GLenum mode) {
    ERROR_IN_BEGIN
	noerrorShim();
	if (face == GL_FRONT)
		face = GL_FRONT_AND_BACK;   //TODO, better handle all this
	if (face == GL_BACK)
		return;		//TODO, handle face enum for polygon mode != GL_FILL
    if (glstate->list.active)
        if (glstate->list.compiling) {
            NewStage(glstate->list.active, STAGE_POLYGON);
            glstate->list.active->polygon_mode = mode;
            return;
        }
        else flush();
	switch(mode) {
		case GL_LINE:
		case GL_POINT:
			glstate->polygon_mode = mode;
			break;
		case GL_FILL:
			glstate->polygon_mode = 0;
			break;
		default:
			glstate->polygon_mode = 0;
	}
}
void glPolygonMode(GLenum face, GLenum mode) AliasExport("gl4es_glPolygonMode");


void flush() {
    // flush internal list
    renderlist_t *mylist = glstate->list.active?extend_renderlist(glstate->list.active):NULL;
    if (mylist) {
        glstate->list.active = NULL;
        glstate->list.pending = 0;
        mylist = end_renderlist(mylist);
        draw_renderlist(mylist);
        free_renderlist(mylist);
    }
    glstate->list.active = NULL;
}

#ifndef NOX11
extern void BlitEmulatedPixmap();
#endif
void gl4es_glFlush() {
	LOAD_GLES(glFlush);
    
    realize_textures();
    if (glstate->list.pending) flush();
    if (glstate->raster.bm_drawing) bitmap_flush();
    
    gles_glFlush();
    errorGL();

#ifndef NOX11
    if(glstate->emulatedPixmap && !glstate->emulatedWin)
        BlitEmulatedPixmap();
#endif
}
void glFlush() AliasExport("gl4es_glFlush");

void gl4es_glFinish() {
	LOAD_GLES(glFinish);
    
    realize_textures();
    if (glstate->list.pending) flush();
    if (glstate->raster.bm_drawing) bitmap_flush();
    
    gles_glFinish();
    errorGL();
}
void glFinish() AliasExport("gl4es_glFinish");

void gl4es_glIndexPointer(GLenum type, GLsizei stride, const GLvoid * pointer) {
    static bool warning = false;
    if(!warning) {
        LOGD("Warning, stubbed glIndexPointer\n");
        warning = true;
    }
}
void glIndexPointer(GLenum type, GLsizei stride, const GLvoid * pointer) AliasExport("gl4es_glIndexPointer");

void gl4es_glEdgeFlagPointer(GLsizei stride, const GLvoid * pointer) {
    static bool warning = false;
    if(!warning) {
        LOGD("Warning, stubbed glEdgeFlagPointer\n");
        warning = true;
    }
}
void glEdgeFlagPointer(GLsizei stride, const GLvoid * pointer) AliasExport("gl4es_glEdgeFlagPointer");



void gl4es_glShadeModel(GLenum mode) {
    if(mode!=GL_SMOOTH && mode!=GL_FLAT) {
        errorShim(GL_INVALID_ENUM);
        return;
    }
    PUSH_IF_COMPILING(glShadeModel);
    noerrorShim();
    if(mode==glstate->shademodel)
        return;
    glstate->shademodel = mode;
    LOAD_GLES2(glShadeModel);
    if(gles_glShadeModel) {
        errorGL();
        gles_glShadeModel(mode);
    }
}
void glShadeModel(GLenum mode) AliasExport("gl4es_glShadeModel");

void gl4es_glAlphaFunc(GLenum func, GLclampf ref) {
    PUSH_IF_COMPILING(glAlphaFunc);
    noerrorShim();
    if(ref<0.0f) ref = 0.0f;
    if(ref>1.0f) ref = 1.0f;
    if(glstate->alphafunc==func && glstate->alpharef==ref)
        return;
    if(func!=GL_NEVER && func!=GL_LESS && func!=GL_EQUAL
        && func!=GL_LEQUAL && func!=GL_GREATER && func!=GL_NOTEQUAL
        && func!=GL_ALWAYS && func!=GL_GEQUAL) {
            errorShim(GL_INVALID_ENUM);
            return;
    }
    glstate->alphafunc = func;
    glstate->alpharef = ref;
    LOAD_GLES_FPE(glAlphaFunc);
    if(gles_glAlphaFunc) {
        errorGL();
        gles_glAlphaFunc(func, ref);
    }
}
void glAlphaFunc(GLenum func, GLclampf ref) AliasExport("gl4es_glAlphaFunc");

void gl4es_glLogicOp(GLenum opcode) {
    PUSH_IF_COMPILING(glLogicOp);
    noerrorShim();
    if(glstate->logicop==opcode)
        return;
    // TODO: test if opcode is valid
    glstate->logicop = opcode;
    LOAD_GLES2(glLogicOp);
    if(gles_glLogicOp) {
        errorGL();
        gles_glLogicOp(opcode);
    }
}
void glLogicOp(GLenum opcode) AliasExport("gl4es_glLogicOp");

void gl4es_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    PUSH_IF_COMPILING(glColorMask);
    if(glstate->colormask[0]==red && glstate->colormask[1]==green && glstate->colormask[2]==blue && glstate->colormask[3]==alpha) {
        noerrorShim();
        return;
    }
    glstate->colormask[0]=red;
    glstate->colormask[1]=green;
    glstate->colormask[2]=blue;
    glstate->colormask[3]=alpha;
    LOAD_GLES(glColorMask);
    gles_glColorMask(red, green, blue, alpha);
}
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) AliasExport("gl4es_glColorMask");

void gl4es_glClear(GLbitfield mask) {
    PUSH_IF_COMPILING(glClear);

    mask &= GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    LOAD_GLES(glClear);
    gles_glClear(mask);
}
void glClear(GLbitfield mask) AliasExport("gl4es_glClear");

void gl4es_scratch(int alloc) {
    if(glstate->scratch_alloc<alloc) {
        if(glstate->scratch)
            free(glstate->scratch);
        glstate->scratch = malloc(alloc);
        glstate->scratch_alloc = alloc;
    }
}

void gl4es_scratch_vertex(int alloc) {
    LOAD_GLES(glBufferData);
    LOAD_GLES(glBindBuffer);
    LOAD_GLES(glGenBuffers);
    if(!glstate->scratch_vertex) {
        glGenBuffers(1, &glstate->scratch_vertex);
    }
    gles_glBindBuffer(GL_ARRAY_BUFFER, glstate->scratch_vertex);
    if(glstate->scratch_vertex_size < alloc) {
        gles_glBufferData(GL_ARRAY_BUFFER, alloc, NULL, GL_DYNAMIC_DRAW);
        glstate->scratch_vertex_size = alloc;
    }
}

void gl4es_use_scratch_vertex(int use) {
    LOAD_GLES(glBindBuffer);
    gles_glBindBuffer(GL_ARRAY_BUFFER, use?glstate->scratch_vertex:0);
}

void gl4es_scratch_indices(int alloc) {
    LOAD_GLES(glBufferData);
    LOAD_GLES(glBindBuffer);
    LOAD_GLES(glGenBuffers);
    if(!glstate->scratch_indices) {
        glGenBuffers(1, &glstate->scratch_indices);
    }
    gles_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glstate->scratch_indices);
    if(glstate->scratch_indices_size < alloc) {
        gles_glBufferData(GL_ELEMENT_ARRAY_BUFFER, alloc, NULL, GL_DYNAMIC_DRAW);
        glstate->scratch_indices_size = alloc;
    }
}

void gl4es_use_scratch_indices(int use) {
    LOAD_GLES(glBindBuffer);
    gles_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, use?glstate->scratch_indices:0);
}

#if defined(AMIGAOS4) || (defined(NOX11) && defined(NOEGL))
#ifdef AMIGAOS4
void amiga_pre_swap()
#else
__attribute__((visibility("default"))) void gl4es_pre_swap()
#endif
{
    if (glstate->list.active){
        flush();
    }
    if (glstate->raster.bm_drawing)
        bitmap_flush();

    if (globals4es.usefbo) {
        unbindMainFBO();
        blitMainFBO(0, 0, 0, 0);
        // blit the main_fbo before swap
    }
}

#ifdef AMIGAOS4
void amiga_post_swap()
#else
__attribute__((visibility("default"))) void gl4es_post_swap()
#endif
{
    if (globals4es.showfps) 
    {
        // framerate counter
        static float avg, fps = 0;
        static int frame1, last_frame, frame, now, current_frames;
        struct timeval out;
        gettimeofday(&out, NULL);
        now = out.tv_sec;
        frame++;
        current_frames++;

        if (frame == 1) {
            frame1 = now;
        } else if (frame1 < now) {
            if (last_frame < now) {
                float change = current_frames / (float)(now - last_frame);
                float weight = 0.7;
                if (! fps) {
                    fps = change;
                } else {
                    fps = (1 - weight) * fps + weight * change;
                }
                current_frames = 0;

                avg = frame / (float)(now - frame1);
                printf("LIBGL: fps: %.2f, avg: %.2f\n", fps, avg);
            }
        }
        last_frame = now;
    }

    // If drawing in fbo, rebind it...
    if (globals4es.usefbo) {
        bindMainFBO();
    }
}
#endif
