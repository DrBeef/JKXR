#ifndef _GL4ES_DIRECTSTATE_H_
#define _GL4ES_DIRECTSTATE_H_

#include "gles.h"

void gl4es_glClientAttribDefault(GLbitfield mask);
void gl4es_glPushClientAttribDefault(GLbitfield mask);

void gl4es_glMatrixLoadf(GLenum matrixMode, const GLfloat *m);
void gl4es_glMatrixLoadd(GLenum matrixMode, const GLdouble *m);
void gl4es_glMatrixMultf(GLenum matrixMode, const GLfloat *m);
void gl4es_glMatrixMultd(GLenum matrixMode, const GLdouble *m);
void gl4es_glMatrixLoadIdentity(GLenum matrixMode);
void gl4es_glMatrixRotatef(GLenum matrixMode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void gl4es_glMatrixRotated(GLenum matrixMode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void gl4es_glMatrixScalef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
void gl4es_glMatrixScaled(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
void gl4es_glMatrixTranslatef(GLenum matrixMode, GLfloat x, GLfloat y, GLfloat z);
void gl4es_glMatrixTranslated(GLenum matrixMode, GLdouble x, GLdouble y, GLdouble z);
void gl4es_glMatrixOrtho(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void gl4es_glMatrixFrustum(GLenum matrixMode, GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void gl4es_glMatrixPush(GLenum matrixMode);
void gl4es_glMatrixPop(GLenum matrixMode);

void gl4es_glTextureParameteri(GLuint texture, GLenum target, GLenum pname, GLint param);
void gl4es_glTextureParameteriv(GLuint texture, GLenum target, GLenum pname, const GLint *param);
void gl4es_glTextureParameterf(GLuint texture, GLenum target, GLenum pname, GLfloat param);
void gl4es_glTextureParameterfv(GLuint texture, GLenum target, GLenum pname, const GLfloat *param);
void gl4es_glTextureImage1D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glTextureImage2D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glCopyTextureImage1D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void gl4es_glCopyTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void gl4es_glCopyTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void gl4es_glCopyTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void gl4es_glGetTextureImage(GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void gl4es_glGetTextureParameterfv(GLuint texture, GLenum target, GLenum pname, GLfloat *params);
void gl4es_glGetTextureParameteriv(GLuint texture, GLenum target, GLenum pname, GLint *params);
void gl4es_glGetTextureLevelParameterfv(GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params);
void gl4es_glGetTextureLevelParameteriv(GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params);

void gl4es_glTextureImage3D(GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glCopyTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void gl4es_glBindMultiTexture(GLenum texunit, GLenum target, GLuint texture);
void gl4es_glMultiTexCoordPointer(GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void gl4es_glMultiTexEnvf(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
void gl4es_glMultiTexEnvfv(GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
void gl4es_glMultiTexEnvi(GLenum texunit, GLenum target, GLenum pname, GLint param);
void gl4es_glMultiTexEnviv(GLenum texunit, GLenum target, GLenum pname, const GLint *params);
void gl4es_glMultiTexGend(GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
void gl4es_glMultiTexGendv(GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params);
void gl4es_glMultiTexGenf(GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
void gl4es_glMultiTexGenfv(GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params);
void gl4es_glMultiTexGeni(GLenum texunit, GLenum coord, GLenum pname, GLint param);
void gl4es_glMultiTexGeniv(GLenum texunit, GLenum coord, GLenum pname, const GLint *params);
void gl4es_glGetMultiTexEnvfv(GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
void gl4es_glGetMultiTexEnviv(GLenum texunit, GLenum target, GLenum pname, GLint *params);
void gl4es_glGetMultiTexGendv(GLenum texunit, GLenum coord, GLenum pname, GLdouble *params);
void gl4es_glGetMultiTexGenfv(GLenum texunit, GLenum coord, GLenum pname, GLfloat *params);
void gl4es_glGetMultiTexGeniv(GLenum texunit, GLenum coord, GLenum pname, GLint *params);
void gl4es_glMultiTexParameteri(GLenum texunit, GLenum target, GLenum pname, GLint param);
void gl4es_glMultiTexParameteriv(GLenum texunit, GLenum target, GLenum pname, const GLint *param);
void gl4es_glMultiTexParameterf(GLenum texunit, GLenum target, GLenum pname, GLfloat param);
void gl4es_glMultiTexParameterfv(GLenum texunit, GLenum target, GLenum pname, const GLfloat *param);
void gl4es_glMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glCopyMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
void gl4es_glCopyMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void gl4es_glCopyMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void gl4es_glCopyMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void gl4es_glGetMultiTexImage(GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void gl4es_glGetMultiTexParameterfv(GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
void gl4es_glGetMultiTexParameteriv(GLenum texunit, GLenum target, GLenum pname, GLint *params);
void gl4es_glGetMultiTexLevelParameterfv(GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params);
void gl4es_glGetMultiTexLevelParameteriv(GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params);
void gl4es_glMultiTexImage3D(GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
void gl4es_glCopyMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

void gl4es_glCompressedTextureImage3D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedTextureImage2D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedTextureImage1D(GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedTextureSubImage3D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedTextureSubImage2D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedTextureSubImage1D(GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
void gl4es_glGetCompressedTextureImage(GLuint texture, GLenum target, GLint level, GLvoid *img);

void gl4es_glCompressedMultiTexImage3D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedMultiTexImage2D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedMultiTexImage1D(GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedMultiTexSubImage3D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedMultiTexSubImage2D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
void gl4es_glCompressedMultiTexSubImage1D(GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
void gl4es_glGetCompressedMultiTexImage(GLenum texunit, GLenum target, GLint level, GLvoid *img);

void gl4es_glMatrixLoadTransposef(GLenum matrixMode, const GLfloat *m);
void gl4es_glMatrixLoadTransposed(GLenum matrixMode, const GLdouble *m);
void gl4es_glMatrixMultTransposef(GLenum matrixMode, const GLfloat *m);
void gl4es_glMatrixMultTransposed(GLenum matrixMode, const GLdouble *m);

void gl4es_glEnableClientStateIndexedEXT(GLenum array, GLuint index);
void gl4es_glDisableClientStateIndexedEXT(GLenum array, GLuint index);

void gl4es_glGetFloatIndexedvEXT(GLenum target, GLuint index, GLfloat *data);
void gl4es_glGetDoubleIndexedvEXT(GLenum target, GLuint index, GLdouble *data);
void gl4es_glGetIntegerIndexedvEXT(GLenum target, GLuint index, GLint *data);
void gl4es_glGetBooleanIndexedvEXT(GLenum target, GLuint index, GLboolean *data);

void gl4es_glGetPointerIndexedvEXT(GLenum pname, GLuint index, GLvoid **params);

void gl4es_glEnableIndexedEXT(GLenum cap, GLuint index);
void gl4es_glDisableIndexedEXT(GLenum cap, GLuint index);
GLboolean gl4es_glIsEnabledIndexedEXT(GLenum cap, GLuint index);

#endif // _GL4ES_DIRECTSTATE_H_
