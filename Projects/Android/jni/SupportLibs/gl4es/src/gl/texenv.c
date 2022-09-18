#include "texenv.h"

#include "../glx/hardext.h"
#include "debug.h"
#include "fpe.h"
#include "gl4es.h"
#include "glstate.h"
#include "loader.h"

//#define DEBUG
#ifdef DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif

void gl4es_glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
    DBG(printf("glTexEnvf(%s, %s, 0x%04X(%s)), tmu=%d, pending=%d, compiling=%d\n", PrintEnum(target), PrintEnum(pname), (GLenum)param, PrintEnum((GLenum)param), glstate->texture.active, glstate->list.pending, glstate->list.compiling);)
    if (!glstate->list.pending) {
        PUSH_IF_COMPILING(glTexEnvf);
    }
    // Handling GL_EXT_DOT3, wrapping to standard dot3 (???)
    if(param==GL_DOT3_RGB_EXT) param=GL_DOT3_RGB;
    if(param==GL_DOT3_RGBA_EXT) param=GL_DOT3_RGBA;
    const int tmu = glstate->texture.active;
    noerrorShim();
    switch(target) {
        case GL_POINT_SPRITE:
            if(pname==GL_COORD_REPLACE) {
                int p = (param!=0.0f)?1:0;
                if (glstate->texture.pscoordreplace[tmu] == p)
                    return;
                if (glstate->list.pending) flush();
                glstate->texture.pscoordreplace[tmu] = p;
                if (glstate->fpe_state)
                    glstate->fpe_state->pointsprite_coord = p;
            } else {
                errorShim(GL_INVALID_ENUM);
                return;
            }
            break;
        case GL_TEXTURE_FILTER_CONTROL:
            if(pname==GL_TEXTURE_LOD_BIAS) {
                if(glstate->texenv[tmu].filter.lod_bias == param)
                    return;
                if (glstate->list.pending) flush();
                glstate->texenv[tmu].filter.lod_bias = param;
            } else {
                errorShim(GL_INVALID_ENUM);
                return;
            }
            break;
        case GL_TEXTURE_ENV:
            {
            texenv_t *t = &glstate->texenv[tmu].env;
            switch(pname) {
                case GL_TEXTURE_ENV_MODE:
                    if(t->mode == param)
                        return;
                    if(param==GL_COMBINE4) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_ADD && param!=GL_MODULATE && param!=GL_DECAL && param!=GL_BLEND && param!=GL_REPLACE && param!=GL_COMBINE) {
                        errorShim(GL_INVALID_ENUM);
                        return;
                    }
                    if (glstate->list.pending) flush();
                    t->mode = param;
                    if(glstate->fpe_state) {
                        int state = FPE_MODULATE;
                        switch(t->mode) {
                            case GL_ADD: state=FPE_ADD; break;
                            case GL_DECAL: state=FPE_DECAL; break;
                            case GL_BLEND: state=FPE_BLEND; break;
                            case GL_REPLACE: state=FPE_REPLACE; break;
                            case GL_COMBINE: state=FPE_COMBINE; break;
                            case GL_COMBINE4: state=FPE_COMBINE4; break;
                        }
                        glstate->fpe_state->texenv &= ~ (7<<(tmu*3));
                        glstate->fpe_state->texenv |= state<<(tmu*3);
                    }
                    break;
                case GL_COMBINE_RGB:
                    if(t->combine_rgb == param)
                        return;
                    if((param==GL_MODULATE_ADD_ATI || param==GL_MODULATE_SIGNED_ADD_ATI || param==GL_MODULATE_SUBTRACT_ATI)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_REPLACE && param!=GL_MODULATE && param!=GL_ADD && param!=GL_ADD_SIGNED 
                        && param!=GL_INTERPOLATE && param!=GL_SUBTRACT && param!=GL_DOT3_RGB && param!=GL_DOT3_RGBA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->combine_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_CR_REPLACE;
                        switch(t->combine_rgb) {
                            case GL_MODULATE: state=FPE_CR_MODULATE; break;
                            case GL_ADD: state=FPE_CR_ADD; break;
                            case GL_ADD_SIGNED: state=FPE_CR_ADD_SIGNED; break;
                            case GL_INTERPOLATE: state=FPE_CR_INTERPOLATE; break;
                            case GL_SUBTRACT: state=FPE_CR_SUBTRACT; break;
                            case GL_DOT3_RGB: state=FPE_CR_DOT3_RGB; break;
                            case GL_DOT3_RGBA: state=FPE_CR_DOT3_RGBA; break;
                            case GL_MODULATE_ADD_ATI: state=FPE_CR_MOD_ADD; break;
                            case GL_MODULATE_SIGNED_ADD_ATI: state=FPE_CR_MOD_ADD_SIGNED; break;
                            case GL_MODULATE_SUBTRACT_ATI: state=FPE_CR_MOD_SUB; break;
                        }
                        glstate->fpe_state->texcombine[tmu] &= 0xf0;
                        glstate->fpe_state->texcombine[tmu] |= state;
                    }
                    break;
                case GL_COMBINE_ALPHA:
                    if(t->combine_alpha == param)
                        return;
                    if((param==GL_MODULATE_ADD_ATI || param==GL_MODULATE_SIGNED_ADD_ATI || param==GL_MODULATE_SUBTRACT_ATI)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_REPLACE && param!=GL_MODULATE && param!=GL_ADD && param!=GL_ADD_SIGNED 
                        && param!=GL_INTERPOLATE && param!=GL_SUBTRACT) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->combine_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_CR_REPLACE;
                        switch(t->combine_alpha) {
                            case GL_MODULATE: state=FPE_CR_MODULATE; break;
                            case GL_ADD: state=FPE_CR_ADD; break;
                            case GL_ADD_SIGNED: state=FPE_CR_ADD_SIGNED; break;
                            case GL_INTERPOLATE: state=FPE_CR_INTERPOLATE; break;
                            case GL_SUBTRACT: state=FPE_CR_SUBTRACT; break;
                            case GL_MODULATE_ADD_ATI: state=FPE_CR_MOD_ADD; break;
                            case GL_MODULATE_SIGNED_ADD_ATI: state=FPE_CR_MOD_ADD_SIGNED; break;
                            case GL_MODULATE_SUBTRACT_ATI: state=FPE_CR_MOD_SUB; break;
                        }
                        glstate->fpe_state->texcombine[tmu] &= 0x0f;
                        glstate->fpe_state->texcombine[tmu] |= (state<<4);
                    }
                    break;
                case GL_SRC0_RGB:
                    if(t->src0_rgb == param)
                        return;
                    if((param==GL_ZERO || param==GL_ONE 
                        || param==GL_SECONDARY_COLOR_ATIX || param==GL_TEXTURE_OUTPUT_RGB_ATIX)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src0_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(t->src0_rgb>=GL_TEXTURE0 && t->src0_rgb<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (t->src0_rgb-GL_TEXTURE0);
                        } else
                            switch(t->src0_rgb) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                            }
                        glstate->fpe_state->texsrcrgb[0] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcrgb[0] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC1_RGB:
                    if(t->src1_rgb == param)
                        return;
                    if((param==GL_ZERO || param==GL_ONE 
                        || param==GL_SECONDARY_COLOR_ATIX || param==GL_TEXTURE_OUTPUT_RGB_ATIX)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src1_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(t->src1_rgb>=GL_TEXTURE0 && t->src1_rgb<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (t->src1_rgb-GL_TEXTURE0);
                        } else
                            switch(t->src1_rgb) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                            }
                        glstate->fpe_state->texsrcrgb[1] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcrgb[1] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC2_RGB:
                    if(t->src2_rgb == param)
                        return;
                    if((param==GL_ZERO || param==GL_ONE 
                        || param==GL_SECONDARY_COLOR_ATIX || param==GL_TEXTURE_OUTPUT_RGB_ATIX)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src2_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(t->src2_rgb>=GL_TEXTURE0 && t->src2_rgb<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (t->src2_rgb-GL_TEXTURE0);
                        } else
                            switch(t->src2_rgb) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                                case GL_ONE: state=FPE_SRC_ONE; break;
                                case GL_ZERO: state=FPE_SRC_ZERO; break;
                                case GL_SECONDARY_COLOR_ATIX: state=FPE_SRC_SECONDARY_COLOR; break;
                                //case GL_TEXTUTRE_OUTPUT: unknown, so fall back to texture...
                            }
                        glstate->fpe_state->texsrcrgb[2] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcrgb[2] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC3_RGB:
                    if(t->src3_rgb == param)
                        return;
                    if(hardext.esversion==1) {
                        errorShim(GL_INVALID_ENUM);
                        return;
                    }
                    if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS && param!=GL_ZERO) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src3_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(param>=GL_TEXTURE0 && param<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (param-GL_TEXTURE0);
                        } else
                            switch(t->src3_rgb) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                                case GL_ZERO: state=FPE_SRC_ZERO; break;
                            }
                        glstate->fpe_state->texsrcrgb[3] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcrgb[3] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC0_ALPHA:
                    if(t->src0_alpha == param)
                        return;
                    if((param==GL_ZERO || param==GL_ONE 
                        || param==GL_TEXTURE_OUTPUT_ALPHA_ATIX)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src0_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(t->src0_alpha>=GL_TEXTURE0 && t->src0_alpha<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (t->src0_alpha-GL_TEXTURE0);
                        } else
                            switch(t->src0_alpha) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                                case GL_ONE: state=FPE_SRC_ONE; break;
                                case GL_ZERO: state=FPE_SRC_ZERO; break;
                                //case GL_SECONDARY_COLOR_ATIX: state=FPE_SRC_SECONDARY_COLOR; break;
                                //case GL_TEXTUTRE_OUTPUT: unknown, so fall back to texture...
                            }
                        glstate->fpe_state->texsrcalpha[0] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcalpha[0] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC1_ALPHA:
                    if(t->src1_alpha == param)
                        return;
                    if((param==GL_ZERO || param==GL_ONE 
                        || param==GL_TEXTURE_OUTPUT_ALPHA_ATIX)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src1_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(t->src1_alpha>=GL_TEXTURE0 && t->src1_alpha<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (t->src1_alpha-GL_TEXTURE0);
                        } else
                            switch(t->src1_alpha) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                                case GL_ONE: state=FPE_SRC_ONE; break;
                                case GL_ZERO: state=FPE_SRC_ZERO; break;
                                //case GL_SECONDARY_COLOR_ATIX: state=FPE_SRC_SECONDARY_COLOR; break;
                                //case GL_TEXTUTRE_OUTPUT: unknown, so fall back to texture...
                            }
                        glstate->fpe_state->texsrcalpha[1] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcalpha[1] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC2_ALPHA:
                    if(t->src2_alpha == param)
                        return;
                    if((param==GL_ZERO || param==GL_ONE 
                        || param==GL_TEXTURE_OUTPUT_ALPHA_ATIX)) {
                        if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    } else if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src2_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(t->src2_alpha>=GL_TEXTURE0 && t->src2_alpha<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (t->src2_alpha-GL_TEXTURE0);
                        } else
                            switch(t->src2_alpha) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                                case GL_ONE: state=FPE_SRC_ONE; break;
                                case GL_ZERO: state=FPE_SRC_ZERO; break;
                                //case GL_SECONDARY_COLOR_ATIX: state=FPE_SRC_SECONDARY_COLOR; break;
                                //case GL_TEXTUTRE_OUTPUT: unknown, so fall back to texture...
                            }
                        glstate->fpe_state->texsrcalpha[2] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcalpha[2] |= state<<(tmu*4);
                    }
                    break;
                case GL_SRC3_ALPHA:
                    if(t->src3_alpha == param)
                        return;
                    if(hardext.esversion==1) {
                        errorShim(GL_INVALID_ENUM);
                        return;
                        }
                    if (param!=GL_TEXTURE && !(param>=GL_TEXTURE0 && param<GL_TEXTURE0+hardext.maxtex) 
                        && param!=GL_CONSTANT && param!=GL_PRIMARY_COLOR && param!=GL_PREVIOUS && param!=GL_ZERO) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->src3_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_SRC_TEXTURE;
                        if(param>=GL_TEXTURE0 && param<=GL_TEXTURE8) {
                            state = FPE_SRC_TEXTURE0 + (param-GL_TEXTURE0);
                        } else
                            switch(t->src3_alpha) {
                                case GL_CONSTANT: state=FPE_SRC_CONSTANT; break;
                                case GL_PRIMARY_COLOR: state=FPE_SRC_PRIMARY_COLOR; break;
                                case GL_PREVIOUS: state=FPE_SRC_PREVIOUS; break;
                                case GL_ZERO: state=FPE_SRC_ZERO; break;
                            }
                        glstate->fpe_state->texsrcalpha[3] &= ~ (0xf<<(tmu*4));
                        glstate->fpe_state->texsrcalpha[3] |= state<<(tmu*4);
                    }
                    break;
                case GL_OPERAND0_RGB:
                    if(t->op0_rgb == param)
                        return;
                    if (param!=GL_SRC_COLOR && param!=GL_ONE_MINUS_SRC_COLOR
                         && param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op0_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        switch(t->op0_rgb) {
                            case GL_SRC_COLOR: state=FPE_OP_SRCCOLOR; break;
                            case GL_ONE_MINUS_SRC_COLOR: state=FPE_OP_MINUSCOLOR; break;
                            case GL_ONE_MINUS_SRC_ALPHA: state=FPE_OP_MINUSALPHA; break;
                        }
                        glstate->fpe_state->texoprgb[0] &= ~ (0x3<<(tmu*2));
                        glstate->fpe_state->texoprgb[0] |= state<<(tmu*2);
                    }
                    break;
                case GL_OPERAND1_RGB:
                    if(t->op1_rgb == param)
                        return;
                    if (param!=GL_SRC_COLOR && param!=GL_ONE_MINUS_SRC_COLOR
                         && param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op1_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        switch(t->op1_rgb) {
                            case GL_SRC_COLOR: state=FPE_OP_SRCCOLOR; break;
                            case GL_ONE_MINUS_SRC_COLOR: state=FPE_OP_MINUSCOLOR; break;
                            case GL_ONE_MINUS_SRC_ALPHA: state=FPE_OP_MINUSALPHA; break;
                        }
                        glstate->fpe_state->texoprgb[1] &= ~ (0x3<<(tmu*2));
                        glstate->fpe_state->texoprgb[1] |= state<<(tmu*2);
                    }
                    break;
                case GL_OPERAND2_RGB:
                    if(t->op2_rgb == param)
                        return;
                    if (param!=GL_SRC_COLOR && param!=GL_ONE_MINUS_SRC_COLOR
                         && param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op2_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        switch(t->op2_rgb) {
                            case GL_SRC_COLOR: state=FPE_OP_SRCCOLOR; break;
                            case GL_ONE_MINUS_SRC_COLOR: state=FPE_OP_MINUSCOLOR; break;
                            case GL_ONE_MINUS_SRC_ALPHA: state=FPE_OP_MINUSALPHA; break;
                        }
                        glstate->fpe_state->texoprgb[2] &= ~ (0x3<<(tmu*2));
                        glstate->fpe_state->texoprgb[2] |= state<<(tmu*2);
                    }
                    break;
                case GL_OPERAND3_RGB:
                    if(t->op3_rgb == param)
                        return;
                    if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                    }
                    if (param!=GL_SRC_COLOR && param!=GL_ONE_MINUS_SRC_COLOR
                         && param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op3_rgb = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        switch(t->op3_rgb) {
                            case GL_SRC_COLOR: state=FPE_OP_SRCCOLOR; break;
                            case GL_ONE_MINUS_SRC_COLOR: state=FPE_OP_MINUSCOLOR; break;
                            case GL_ONE_MINUS_SRC_ALPHA: state=FPE_OP_MINUSALPHA; break;
                        }
                        glstate->fpe_state->texoprgb[3] &= ~ (0x3<<(tmu*2));
                        glstate->fpe_state->texoprgb[3] |= state<<(tmu*2);
                    }
                    break;
                case GL_OPERAND0_ALPHA:
                    if(t->op0_alpha == param)
                        return;
                    if (param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op0_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        if(t->op0_alpha==GL_ONE_MINUS_SRC_ALPHA) state=FPE_OP_MINUSALPHA;

                        glstate->fpe_state->texopalpha[0] &= ~ (0x1<<tmu);
                        glstate->fpe_state->texopalpha[0] |= state<<tmu;
                    }
                    break;
                case GL_OPERAND1_ALPHA:
                    if(t->op1_alpha == param)
                        return;
                    if (param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op1_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        if(t->op1_alpha==GL_ONE_MINUS_SRC_ALPHA) state=FPE_OP_MINUSALPHA;

                        glstate->fpe_state->texopalpha[1] &= ~ (0x1<<tmu);
                        glstate->fpe_state->texopalpha[1] |= state<<tmu;
                    }
                    break;
                case GL_OPERAND2_ALPHA:
                    if(t->op2_alpha == param)
                        return;
                    if (param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op2_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        if(t->op2_alpha==GL_ONE_MINUS_SRC_ALPHA) state=FPE_OP_MINUSALPHA;

                        glstate->fpe_state->texopalpha[2] &= ~ (0x1<<tmu);
                        glstate->fpe_state->texopalpha[2] |= state<<tmu;
                    }
                    break;
                case GL_OPERAND3_ALPHA:
                    if(t->op3_alpha == param)
                        return;
                    if(hardext.esversion==1) {
                            errorShim(GL_INVALID_ENUM);
                            return;                        
                    }
                    if (param!=GL_SRC_ALPHA && param!=GL_ONE_MINUS_SRC_ALPHA) {
                            errorShim(GL_INVALID_ENUM);
                            return;
                        }
                    if (glstate->list.pending) flush();
                    t->op3_alpha = param;
                    if(glstate->fpe_state) {
                        int state = FPE_OP_ALPHA;
                        if(t->op3_alpha==GL_ONE_MINUS_SRC_ALPHA) state=FPE_OP_MINUSALPHA;

                        glstate->fpe_state->texopalpha[3] &= ~ (0x1<<tmu);
                        glstate->fpe_state->texopalpha[3] |= state<<tmu;
                    }
                    break;
                case GL_RGB_SCALE:
                    if(t->rgb_scale == param)
                        return;
                    if(param!=1.0 && param!=2.0 && param!=4.0) {
                        errorShim(GL_INVALID_VALUE);
                        return;
                    }
                    if (glstate->list.pending) flush();
                    t->rgb_scale = param;
                    if(glstate->fpe_state) {
                        if(param==1.0f)
                            glstate->fpe_state->texrgbscale &= ~(1<<tmu);
                        else
                        glstate->fpe_state->texrgbscale |= 1<<tmu;
                    }
                    break;
                case GL_ALPHA_SCALE:
                    if(t->alpha_scale == param)
                        return;
                    if(param!=1.0 && param!=2.0 && param!=4.0) {
                        errorShim(GL_INVALID_VALUE);
                        return;
                    }
                    if (glstate->list.pending) flush();
                    t->alpha_scale = param;
                    if(glstate->fpe_state) {
                        if(param==1.0f)
                            glstate->fpe_state->texalphascale &= ~(1<<tmu);
                        else
                        glstate->fpe_state->texalphascale |= 1<<tmu;
                    }
                    break;
                default:
                    errorShim(GL_INVALID_ENUM);
                    return;
            }
            }
            break;
        default:
            errorShim(GL_INVALID_ENUM);
            return;
    }
    errorGL();
    if(hardext.esversion==1) {
        LOAD_GLES2(glTexEnvf);
        realize_active();
        gles_glTexEnvf(target, pname, param);
    }
}

void gl4es_glTexEnvi(GLenum target, GLenum pname, GLint param) {
    DBG(printf("glTexEnvi(...)->");)
    gl4es_glTexEnvf(target, pname, param);
}

void gl4es_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *param) {
    DBG(printf("glTexEnvfv(%s, %s, %p)->", PrintEnum(target), PrintEnum(pname), param);)
    if (glstate->list.compiling && glstate->list.active && !glstate->list.pending) {
        DBG(printf("rlTexEnvfv(...)\n");)
		NewStage(glstate->list.active, STAGE_TEXENV);
		rlTexEnvfv(glstate->list.active, target, pname, param);
        noerrorShim();
		return;
	}
    if(target==GL_TEXTURE_ENV && pname==GL_TEXTURE_ENV_COLOR) {
        texenv_t *t = &glstate->texenv[glstate->texture.active].env;
        DBG(printf("Color=%f/%f/%f/%f\n", param[0], param[1], param[2], param[3]);)
        if(memcmp(t->color, param, 4*sizeof(GLfloat))==0) {
            noerrorShim();
            return;
        }
        if (glstate->list.pending) flush();
        memcpy(t->color, param, 4*sizeof(GLfloat));
        errorGL();
        if(hardext.esversion==1) {
            LOAD_GLES2(glTexEnvfv);
            realize_active();
            gles_glTexEnvfv(target, pname, param);
        }
    } else
        gl4es_glTexEnvf(target, pname, *param);
}
void gl4es_glTexEnviv(GLenum target, GLenum pname, const GLint *param) {
    DBG(printf("glTexEnviv(%s, %s, %p)->", PrintEnum(target), PrintEnum(pname), param);)
    if (glstate->list.compiling && glstate->list.active && !glstate->list.pending) {
        DBG(printf("rlTexEnviv(...)\n");)
		NewStage(glstate->list.active, STAGE_TEXENV);
		rlTexEnviv(glstate->list.active, target, pname, param);
        noerrorShim();
		return;
	}
    if(target==GL_TEXTURE_ENV && pname==GL_TEXTURE_ENV_COLOR) {
        GLfloat p[4];
        p[0] = param[0]; p[1] = param[1]; p[2] = param[2]; p[3] = param[3];
        DBG(printf("Color=%d/%d/%d/%d\n", param[0], param[1], param[2], param[3]);)
        gl4es_glTexEnvfv(target, pname, p);
    } else
        gl4es_glTexEnvf(target, pname, *param);
}
void gl4es_glGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params) {
    //if (glstate->list.pending) flush();
    DBG(printf("glGetTexEnvfv(%s, %s, %p)\n", PrintEnum(target), PrintEnum(pname), params);)
    noerrorShim();
    switch(target) {
        case GL_POINT_SPRITE:
            if(pname == GL_COORD_REPLACE) {
                *params = glstate->texture.pscoordreplace[glstate->texture.active];
                return;
            }
            break;
        case GL_TEXTURE_FILTER_CONTROL:
            if(pname == GL_TEXTURE_LOD_BIAS) {
                *params = glstate->texenv[glstate->texture.active].filter.lod_bias;
                return;
            }
            break;
        case GL_TEXTURE_ENV:
        {
            texenv_t *t = &glstate->texenv[glstate->texture.active].env;
            switch(pname) {
                case GL_TEXTURE_ENV_MODE:
                    *params = t->mode;
                    return;
                case GL_TEXTURE_ENV_COLOR:
                    memcpy(params, t->color, 4*sizeof(GLfloat));
                    return;
                case GL_COMBINE_RGB:
                    *params = t->combine_rgb;
                    return;
                case GL_COMBINE_ALPHA:
                    *params = t->combine_alpha;
                    return;
                case GL_SRC0_RGB:
                    *params = t->src0_rgb;
                    return;
                case GL_SRC1_RGB:
                    *params = t->src1_rgb;
                    return;
                case GL_SRC2_RGB:
                    *params = t->src2_rgb;
                    return;
                case GL_SRC0_ALPHA:
                    *params = t->src0_alpha;
                    return;
                case GL_SRC1_ALPHA:
                    *params = t->src1_alpha;
                    return;
                case GL_SRC2_ALPHA:
                    *params = t->src2_alpha;
                    return;
                case GL_OPERAND0_RGB:
                    *params = t->op0_rgb;
                    break;
                case GL_OPERAND1_RGB:
                    *params = t->op1_rgb;
                    break;
                case GL_OPERAND2_RGB:
                    *params = t->op2_rgb;
                    break;
                case GL_OPERAND0_ALPHA:
                    *params = t->op0_alpha;
                    break;
                case GL_OPERAND1_ALPHA:
                    *params = t->op1_alpha;
                    break;
                case GL_OPERAND2_ALPHA:
                    *params = t->op2_alpha;
                    break;
                case GL_RGB_SCALE:
                    *params = t->rgb_scale;
                    return;
                case GL_ALPHA_SCALE:
                    *params = t->alpha_scale;
                    return;
            }
        }
    }
    errorShim(GL_INVALID_ENUM);
    return;
}
void gl4es_glGetTexEnviv(GLenum target, GLenum pname, GLint * params) {
 //   LOAD_GLES(glGetTexEnviv);
    //if (glstate->list.pending) flush();
    noerrorShim();
    switch(target) {
        case GL_POINT_SPRITE:
            if(pname == GL_COORD_REPLACE) {
                *params = glstate->texture.pscoordreplace[glstate->texture.active];
                return;
            }
            break;
        case GL_TEXTURE_FILTER_CONTROL:
            if(pname == GL_TEXTURE_LOD_BIAS) {
                *params = glstate->texenv[glstate->texture.active].filter.lod_bias;
                return;
            }
            break;
        case GL_TEXTURE_ENV:
        {
            texenv_t *t = &glstate->texenv[glstate->texture.active].env;
            switch(pname) {
                case GL_TEXTURE_ENV_MODE:
                    *params = t->mode;
                    return;
                case GL_TEXTURE_ENV_COLOR:
                    memcpy(params, t->color, 4*sizeof(GLfloat));
                    return;
                case GL_COMBINE_RGB:
                    *params = t->combine_rgb;
                    return;
                case GL_COMBINE_ALPHA:
                    *params = t->combine_alpha;
                    return;
                case GL_SRC0_RGB:
                    *params = t->src0_rgb;
                    return;
                case GL_SRC1_RGB:
                    *params = t->src1_rgb;
                    return;
                case GL_SRC2_RGB:
                    *params = t->src2_rgb;
                    return;
                case GL_SRC0_ALPHA:
                    *params = t->src0_alpha;
                    return;
                case GL_SRC1_ALPHA:
                    *params = t->src1_alpha;
                    return;
                case GL_SRC2_ALPHA:
                    *params = t->src2_alpha;
                    return;
                case GL_OPERAND0_RGB:
                    *params = t->op0_rgb;
                    break;
                case GL_OPERAND1_RGB:
                    *params = t->op1_rgb;
                    break;
                case GL_OPERAND2_RGB:
                    *params = t->op2_rgb;
                    break;
                case GL_OPERAND0_ALPHA:
                    *params = t->op0_alpha;
                    break;
                case GL_OPERAND1_ALPHA:
                    *params = t->op1_alpha;
                    break;
                case GL_OPERAND2_ALPHA:
                    *params = t->op2_alpha;
                    break;
                case GL_RGB_SCALE:
                    *params = t->rgb_scale;
                    return;
                case GL_ALPHA_SCALE:
                    *params = t->alpha_scale;
                    return;
            }
        }
    }
    errorShim(GL_INVALID_ENUM);
    return;
}


void glTexEnvf(GLenum target, GLenum pname, GLfloat param) AliasExport("gl4es_glTexEnvf");
void glTexEnvi(GLenum target, GLenum pname, GLint param) AliasExport("gl4es_glTexEnvi");
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *param) AliasExport("gl4es_glTexEnvfv");
void glTexEnviv(GLenum target, GLenum pname, const GLint *param) AliasExport("gl4es_glTexEnviv");
void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params) AliasExport("gl4es_glGetTexEnvfv");
void glGetTexEnviv(GLenum target, GLenum pname, GLint * params) AliasExport("gl4es_glGetTexEnviv");
