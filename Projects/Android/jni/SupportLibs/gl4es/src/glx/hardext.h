#ifndef _GLX_HARDEXT_H_
#define _GLX_HARDEXT_H_

#define VEND_IMGTEC     0x0100
#define VEND_ARM        0x0200

typedef struct _hardext {
    int npot;           // 0 = no npot capability, 1 = limited npot, 2 = full npot
    int maxtex;         // maximum number of texture
    int maxlights;      // maximum number of light
    int maxsize;        // maximum texture size
    int maxplanes;      // maximum clip planes
    int blendsub;       // GL_OES_blend_subtract
    int blendfunc;      // GL_OES_blend_func_separate
    int blendeq;        // GL_OES_blend_equation_separate
    int blendminmax;    // GL_EXT_blend_minmax
    int blendcolor;     // GL_EXT_blend_color
    int pointsprite;    // GL_OES_point_sprite
    int pointsize;      // GL_OES_point_size_array
    int elementuint;    // GL_OES_element_index_uint
    int fbo;            // GL_OES_framebuffer_object
    int depthstencil;   // GL_OES_packed_depth_stencil
    int depth24;        // GL_OES_depth24
    int rgba8;          // GL_OES_rgb8_rgba8
    int multidraw;      // GL_EXT_multi_draw_arrays
    int bgra8888;       // GL_EXT_texture_format_BGRA8888
    int depthtex;       // GL_OES_depth_texture
    int stenciltex;     // GL_OES_texture_stencil8
    int cubemap;        // GL_OES_texture_cube_map
    int drawtex;        // GL_OES_draw_texture
    int rgtex;          // GL_EXT_texture_rg
    int floattex;       // GL_OES_texture_float
    int halffloattex;   // GL_OES_texture_half_float
    int floatfbo;       // GL_EXT_color_buffer_float
    int halffloatfbo;   // GL_EXT_color_buffer_half_float
    int mirrored;       // GL_OES_texture_mirrored_repeat
    int aniso;          // Max ANISOTROPIC filter available (0 if not)
    int srgb;           // EGL_KHR_gl_colorspace
    int mapbuffer;      // GL_OES_mapbuffer
    // es2 stuffs
    int esversion;      // 1 is ES1.1 backend, 2 is ES2
    int maxvattrib;     // GL_MAX_VERTEX_ATTRIBS (or 0 if not using es2)
    int maxteximage;    // GL_MAX_TEXTURE_IMAGE_UNITS for es2
    int maxvarying;     // GL_MAX_VARYING_VECTORS for es2
    int highp;          // GL_OES_fragment_precision_high
    int fragdepth;      // GL_EXT_frag_depth
    int derivatives;    // GL_OES_standard_derivatives
    int gbm;            // EGL_KHR_platform_gbm
    int vendor;         // which vendor (to apply workaround)
    int eglnoalpha;     // EGL surface doesn't seems to have any alpha channel (auto detect)
} hardext_t;

extern hardext_t hardext;

void GetHardwareExtensions(int test);

#endif
