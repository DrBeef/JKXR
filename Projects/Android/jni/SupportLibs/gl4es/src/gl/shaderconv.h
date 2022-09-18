#ifndef _GL4ES_SHADERCONV_H_
#define _GL4ES_SHADERCONV_H_

#include "gles.h"
#include "program.h"

char* ConvertShader(const char* pBuffer, int isVertex, shaderconv_need_t *need);

int isBuiltinAttrib(const char* name);
int isBuiltinMatrix(const char* name);

#endif // _GL4ES_SHADERCONV_H_
