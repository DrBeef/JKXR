#ifndef GL_WRAP_H
#define GL_WRAP_H

#include "../gl4es.h"
//#include <GLES/glext.h>


// misc naive wrappers

void gl4es_glClearDepth(GLdouble depth);
void gl4es_glClipPlane(GLenum plane, const GLdouble *equation);
void gl4es_glDepthRange(GLdouble nearVal, GLdouble farVal);
void gl4es_glFogi(GLenum pname, GLint param);
void gl4es_glFogiv(GLenum pname, GLint *params);
void gl4es_glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void gl4es_glGetDoublev(GLenum pname, GLdouble *params);
void gl4es_glLighti(GLenum light, GLenum pname, GLint param);
void gl4es_glLightiv(GLenum light, GLenum pname, GLint *iparams);
void gl4es_glLightModeli(GLenum pname, GLint param);
void gl4es_glLightModeliv(GLenum pname, GLint *iparams);
void gl4es_glMateriali(GLenum face, GLenum pname, GLint param);
void gl4es_glMaterialiv(GLenum face, GLenum pname, GLint *param);
void gl4es_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
void gl4es_glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void gl4es_glGetMaterialiv(GLenum face, GLenum pname, GLint * params);
void gl4es_glGetLightiv(GLenum light, GLenum pname, GLint * params);
void gl4es_glGetClipPlane(GLenum plane, GLdouble *equation);

void gl4es_glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void *indices);	
// color
void gl4es_glColor3f(GLfloat r, GLfloat g, GLfloat b);
void gl4es_glColor3fv(GLfloat *c);
void gl4es_glColor4fv(GLfloat *c);
void gl4es_glIndexfv(const GLfloat *c);
void gl4es_glSecondaryColor3fv(const GLfloat *v);

// raster
void gl4es_glRasterPos2f(GLfloat x, GLfloat y);
void gl4es_glRasterPos2fv(const GLfloat *v);
void gl4es_glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
void gl4es_glRasterPos3fv(const GLfloat *v);
void gl4es_glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void gl4es_glRasterPos4fv(const GLfloat *v);
void gl4es_glWindowPos2f(GLfloat x, GLfloat y);
void gl4es_glWindowPos2fv(const GLfloat *v);
void gl4es_glWindowPos3f(GLfloat x, GLfloat y, GLfloat z);
void gl4es_glWindowPos3fv(const GLfloat *v);

void gl4es_glPixelStoref(GLenum pname, GLfloat param);
void gl4es_glGetTexGendv(GLenum coord,GLenum pname,GLdouble *params);
void gl4es_glGetTexGeniv(GLenum coord,GLenum pname,GLint *params);
void gl4es_glPixelTransferi(GLenum pname, GLint param);
void gl4es_glPixelTransferf(GLenum pname, GLfloat param);	

// eval
void gl4es_glEvalCoord1d(GLdouble u);
void gl4es_glEvalCoord1dv(GLdouble *v);
void gl4es_glEvalCoord1fv(GLfloat *v);
void gl4es_glEvalCoord2d(GLdouble u, GLdouble v);
void gl4es_glEvalCoord2dv(GLdouble *v);
void gl4es_glEvalCoord2fv(GLfloat *v);
void gl4es_glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
void gl4es_glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);

// matrix
void gl4es_glLoadMatrixd(const GLdouble *m);
void gl4es_glMultMatrixd(const GLdouble *m);

// normal
void gl4es_glNormal3fv(GLfloat *v);

// fog (defined in fog.c)
void gl4es_glFogCoordd(GLdouble coord);
void gl4es_glFogCoorddv(const GLdouble *coord);

// rect
#define GL_RECT(suffix, type)                                \
    void gl4es_glRect##suffix(type x1, type y1, type x2, type y2); \
    void gl4es_glRect##suffix##v(const type *v);

GL_RECT(d, GLdouble)
GL_RECT(f, GLfloat)
GL_RECT(i, GLint)
GL_RECT(s, GLshort)
#undef GL_RECT

// textures

void gl4es_glTexCoord1f(GLfloat s);
void gl4es_glTexCoord1fv(GLfloat *t);
void gl4es_glTexCoord2f(GLfloat s, GLfloat t);
void gl4es_glTexCoord2fv(GLfloat *t);
void gl4es_glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
void gl4es_glTexCoord3fv(GLfloat *t);
void gl4es_glTexCoord4fv(GLfloat *t);

void gl4es_glMultiTexCoord1f(GLenum target, GLfloat s);
void gl4es_glMultiTexCoord1fv(GLenum target, GLfloat *t);
void gl4es_glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
void gl4es_glMultiTexCoord2fv(GLenum target, GLfloat *t);
void gl4es_glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
void gl4es_glMultiTexCoord3fv(GLenum target, GLfloat *t);
void gl4es_glMultiTexCoord4fv(GLenum target, GLfloat *t);

void gl4es_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);

void gl4es_glPolygonMode(GLenum face, GLenum mode);

// texgen
void gl4es_glTexGend(GLenum coord, GLenum pname, GLdouble param);
void gl4es_glTexGeni(GLenum coord, GLenum pname, GLint param);
void gl4es_glTexGenf(GLenum coord, GLenum pname, GLfloat param);
void gl4es_glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);
void gl4es_glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
void gl4es_glTexGeniv(GLenum coord, GLenum pname, const GLint *params);

// transforms
void gl4es_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void gl4es_glScaled(GLdouble x, GLdouble y, GLdouble z);
void gl4es_glTranslated(GLdouble x, GLdouble y, GLdouble z);

// vertex
void gl4es_glVertex2f(GLfloat x, GLfloat y);
void gl4es_glVertex2fv(GLfloat *v);
void gl4es_glVertex3fv(GLfloat *v);
void gl4es_glVertex3f(GLfloat r, GLfloat g, GLfloat b);
void gl4es_glVertex4fv(GLfloat *v);

// OES wrappers

void glClearDepthfOES(GLfloat depth);
void glClipPlanefOES(GLenum plane, const GLfloat *equation);
void glDepthRangefOES(GLclampf near, GLclampf far);
void glFrustumfOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
void glGetClipPlanefOES(GLenum plane, GLfloat equation[4]);
void glOrthofOES(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

// basic thunking

#define THUNK(suffix, type)                                \
void gl4es_glColor3##suffix##v(const type *v);                   \
void gl4es_glColor3##suffix(type r, type g, type b);             \
void gl4es_glColor4##suffix##v(const type *v);                   \
void gl4es_glColor4##suffix(type r, type g, type b, type a);     \
void gl4es_glSecondaryColor3##suffix##v(const type *v);          \
void gl4es_glSecondaryColor3##suffix(type r, type g, type b);    \
void gl4es_glIndex##suffix##v(const type *c);                    \
void gl4es_glIndex##suffix(type c);                              \
void gl4es_glNormal3##suffix##v(const type *v);                  \
void gl4es_glNormal3##suffix(type x, type y, type z);            \
void gl4es_glRasterPos2##suffix##v(type *v);                     \
void gl4es_glRasterPos2##suffix(type x, type y);                 \
void gl4es_glRasterPos3##suffix##v(type *v);                     \
void gl4es_glRasterPos3##suffix(type x, type y, type z);         \
void gl4es_glRasterPos4##suffix##v(type *v);                     \
void gl4es_glRasterPos4##suffix(type x, type y, type z, type w); \
void gl4es_glWindowPos2##suffix##v(type *v);                     \
void gl4es_glWindowPos2##suffix(type x, type y);                 \
void gl4es_glWindowPos3##suffix##v(type *v);                     \
void gl4es_glWindowPos3##suffix(type x, type y, type z);         \
void gl4es_glVertex2##suffix##v(type *v);                        \
void gl4es_glVertex2##suffix(type x, type y);                    \
void gl4es_glVertex3##suffix##v(type *v);                        \
void gl4es_glVertex3##suffix(type x, type y, type z);            \
void gl4es_glVertex4##suffix(type x, type y, type z, type w);    \
void gl4es_glVertex4##suffix##v(type *v);                        \
void gl4es_glTexCoord1##suffix(type s);                          \
void gl4es_glTexCoord1##suffix##v(type *t);                      \
void gl4es_glTexCoord2##suffix(type s, type t);                  \
void gl4es_glTexCoord2##suffix##v(type *t);                      \
void gl4es_glTexCoord3##suffix(type s, type t, type r);          \
void gl4es_glTexCoord3##suffix##v(type *t);                      \
void gl4es_glTexCoord4##suffix(type s, type t, type r, type q);  \
void gl4es_glTexCoord4##suffix##v(type *t);					   \
void gl4es_glMultiTexCoord1##suffix(GLenum target, type s);                          \
void gl4es_glMultiTexCoord1##suffix##v(GLenum target, type *t);                      \
void gl4es_glMultiTexCoord2##suffix(GLenum target, type s, type t);                  \
void gl4es_glMultiTexCoord2##suffix##v(GLenum target, type *t);                      \
void gl4es_glMultiTexCoord3##suffix(GLenum target, type s, type t, type r);          \
void gl4es_glMultiTexCoord3##suffix##v(GLenum target, type *t);                      \
void gl4es_glMultiTexCoord4##suffix(GLenum target, type s, type t, type r, type q);  \
void gl4es_glMultiTexCoord4##suffix##v(GLenum target, type *t);					   \
void gl4es_glMultiTexCoord1##suffix##ARB(GLenum target, type s);                          \
void gl4es_glMultiTexCoord1##suffix##vARB(GLenum target, type *t);                      \
void gl4es_glMultiTexCoord2##suffix##ARB(GLenum target, type s, type t);                  \
void gl4es_glMultiTexCoord2##suffix##vARB(GLenum target, type *t);                      \
void gl4es_glMultiTexCoord3##suffix##ARB(GLenum target, type s, type t, type r);          \
void gl4es_glMultiTexCoord3##suffix##vARB(GLenum target, type *t);                      \
void gl4es_glMultiTexCoord4##suffix##ARB(GLenum target, type s, type t, type r, type q);  \
void gl4es_glMultiTexCoord4##suffix##vARB(GLenum target, type *t);

THUNK(b, GLbyte)
THUNK(d, GLdouble)
THUNK(i, GLint)
THUNK(s, GLshort)
THUNK(ub, GLubyte)
THUNK(ui, GLuint)
THUNK(us, GLushort)
#undef THUNK

#define THUNK(suffix, type) \
void glGet##suffix##v(GLenum pname, type *params);

THUNK(Double, GLdouble)
THUNK(Integer, GLint)
THUNK(Float, GLfloat)
#undef THUNK

#define THUNK(suffix, type) \
void gl4es_glVertexAttrib1##suffix(GLuint index, type v0); \
void gl4es_glVertexAttrib2##suffix(GLuint index, type v0, type v1); \
void gl4es_glVertexAttrib3##suffix(GLuint index, type v0, type v1, type v2); \
void gl4es_glVertexAttrib4##suffix(GLuint index, type v0, type v1, type v2, type v3); \
void gl4es_glVertexAttrib1##suffix##v(GLuint index, const type *v); \
void gl4es_glVertexAttrib2##suffix##v(GLuint index, const type *v); \
void gl4es_glVertexAttrib3##suffix##v(GLuint index, const type *v);
THUNK(s, GLshort);
THUNK(d, GLdouble);
#undef THUNK
void gl4es_glVertexAttrib4dv(GLuint index, const GLdouble *v);
#define THUNK(suffix, type) \
void gl4es_glVertexAttrib4##suffix##v (GLuint index, const type *v); \
void gl4es_glVertexAttrib4N##suffix##v (GLuint index, const type *v)
THUNK(b, GLbyte);
THUNK(ub, GLubyte);
THUNK(s, GLshort);
THUNK(us, GLushort);
THUNK(i, GLint);
THUNK(ui, GLuint);
#undef THUNK
void gl4es_glVertexAttrib4Nub(GLuint index, GLubyte v0, GLubyte v1, GLubyte v2, GLubyte v3);

// ============= GL_ARB_vertex_shader =================
GLvoid glVertexAttrib1fARB(GLuint index, GLfloat v0);
GLvoid glVertexAttrib1sARB(GLuint index, GLshort v0);
GLvoid glVertexAttrib1dARB(GLuint index, GLdouble v0);
GLvoid glVertexAttrib2fARB(GLuint index, GLfloat v0, GLfloat v1);
GLvoid glVertexAttrib2sARB(GLuint index, GLshort v0, GLshort v1);
GLvoid glVertexAttrib2dARB(GLuint index, GLdouble v0, GLdouble v1);
GLvoid glVertexAttrib3fARB(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
GLvoid glVertexAttrib3sARB(GLuint index, GLshort v0, GLshort v1, GLshort v2);
GLvoid glVertexAttrib3dARB(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2);
GLvoid glVertexAttrib4sARB(GLuint index, GLshort v0, GLshort v1, GLshort v2, GLshort v3);
GLvoid glVertexAttrib4dARB(GLuint index, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
GLvoid glVertexAttrib4NubARB(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GLvoid glVertexAttrib1fvARB(GLuint index, const GLfloat *v);
GLvoid glVertexAttrib1svARB(GLuint index, const GLshort *v);
GLvoid glVertexAttrib1dvARB(GLuint index, const GLdouble *v);
GLvoid glVertexAttrib2fvARB(GLuint index, const GLfloat *v);
GLvoid glVertexAttrib2svARB(GLuint index, const GLshort *v);
GLvoid glVertexAttrib2dvARB(GLuint index, const GLdouble *v);
GLvoid glVertexAttrib3fvARB(GLuint index, const GLfloat *v);
GLvoid glVertexAttrib3svARB(GLuint index, const GLshort *v);
GLvoid glVertexAttrib3dvARB(GLuint index, const GLdouble *v);
GLvoid glVertexAttrib4svARB(GLuint index, const GLshort *v);
GLvoid glVertexAttrib4dvARB(GLuint index, const GLdouble *v);
GLvoid glVertexAttrib4ivARB(GLuint index, const GLint *v);
GLvoid glVertexAttrib4bvARB(GLuint index, const GLbyte *v);
GLvoid glVertexAttrib4ubvARB(GLuint index, const GLubyte *v);
GLvoid glVertexAttrib4usvARB(GLuint index, const GLushort *v);
GLvoid glVertexAttrib4uivARB(GLuint index, const GLuint *v);
GLvoid glVertexAttrib4NbvARB(GLuint index, const GLbyte *v);
GLvoid glVertexAttrib4NsvARB(GLuint index, const GLshort *v);
GLvoid glVertexAttrib4NivARB(GLuint index, const GLint *v);
GLvoid glVertexAttrib4NubvARB(GLuint index, const GLubyte *v);
GLvoid glVertexAttrib4NusvARB(GLuint index, const GLushort *v);
GLvoid glVertexAttrib4NuivARB(GLuint index, const GLuint *v);


#endif
