// -*- c++ -*-
/* Do not edit! -- generated file */
#ifndef _SIGC_FUNCTORS_MACROS_FUNCTOR_TRAITHM4_
#define _SIGC_FUNCTORS_MACROS_FUNCTOR_TRAITHM4_
#include <sigc++/type_traits.h>


namespace sigc {

/** nil struct type.
 * The nil struct type is used as default template argument in the
 * unnumbered sigc::signal and sigc::slot templates.
 *
 * @ingroup signal
 * @ingroup slot
 */
struct nil;


/** @defgroup sigcfunctors Functors
 * Functors are copyable types that define operator()().
 *
 * Types that define operator()() overloads with different return types are referred to
 * as multi-type functors. Multi-type functors are only partially supported in libsigc++.
 *
 * Closures are functors that store all information needed to invoke a callback from operator()().
 *
 * Adaptors are functors that alter the signature of a functor's operator()().
 *
 * libsigc++ defines numerous functors, closures and adaptors.
 * Since libsigc++ is a callback library, most functors are also closures.
 * The documentation doesn't distinguish between functors and closures.
 *
 * The basic functor types libsigc++ provides are created with ptr_fun() and mem_fun()
 * and can be converted into slots implicitly.
 * The set of adaptors that ships with libsigc++ is documented in the @ref adaptors module.
 *
 * If you want to mix user-defined and third party functors with libsigc++,
 * and you want them to be implicitly convertible into slots, libsigc++ must know
 * the result type of your functors. There are different ways to achieve that.
 *
 * - Derive your functors from sigc::functor_base and place
 *   <tt>typedef T_return result_type;</tt> in the class definition.
 * - Use the macro SIGC_FUNCTOR_TRAIT(T_functor,T_return) in namespace sigc.
 *   Multi-type functors are only partly supported.
 * - Use the macro #SIGC_FUNCTORS_HAVE_RESULT_TYPE, if you want libsigc++ to assume
 *   that result_type is defined in all user-defined or third party functors,
 *   except those for which you specify a return type explicitly with SIGC_FUNCTOR_TRAIT().
 * - Use the macro #SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE, if your
 *   compiler makes it possible. Functors with overloaded operator()() are not
 *   supported.
 *
 * The last alterative makes it possible to construct a slot from a C++11 lambda
 * expression with any return type. Example:
 * @code
 * namespace sigc {
 *   SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
 * }
 * sigc::slot<bool, int> slot1 = [](int n)-> bool
 *                               {
 *                                 return n == 42;
 *                               };
 * @endcode
 */

/** A hint to the compiler.
 * All functors which define @p result_type should publically inherit from this hint.
 *
 * @ingroup sigcfunctors
 */
struct functor_base {};

/** Trait that specifies the return type of any type.
 * Template specializations for functors derived from sigc::functor_base,
 * for function pointers and for class methods are provided.
 *
 * @tparam T_functor Functor type.
 * @tparam I_derives_functor_base Whether @p T_functor inherits from sigc::functor_base.
 *
 * @ingroup sigcfunctors
 */
template <class T_functor, bool I_derives_functor_base=is_base_and_derived<functor_base,T_functor>::value>
struct functor_trait
{
  typedef void result_type;
  typedef T_functor functor_type;
};

template <class T_functor>
struct functor_trait<T_functor,true>
{
  typedef typename T_functor::result_type result_type;
  typedef T_functor functor_type;
};

/** Helper macro, if you want to mix user-defined and third party functors with libsigc++.
 *
 * If you want to mix functors not derived from sigc::functor_base with libsigc++, and
 * these functors define @p result_type, use this macro inside namespace sigc like so:
 * @code
 * namespace sigc { SIGC_FUNCTORS_HAVE_RESULT_TYPE }
 * @endcode
 *
 * You can't use both SIGC_FUNCTORS_HAVE_RESULT_TYPE and
 * SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE in the same compilation unit.
 *
 * @ingroup sigcfunctors
 */
#define SIGC_FUNCTORS_HAVE_RESULT_TYPE                 \
template <class T_functor>                             \
struct functor_trait<T_functor,false>                  \
{                                                      \
  typedef typename T_functor::result_type result_type; \
  typedef T_functor functor_type;                      \
};

/** Helper macro, if you want to mix user-defined and third party functors with libsigc++.
 *
 * If you want to mix functors not derived from sigc::functor_base with libsigc++, and
 * these functors don't define @p result_type, use this macro inside namespace sigc
 * to expose the return type of the functors like so:
 * @code
 * namespace sigc {
 *   SIGC_FUNCTOR_TRAIT(first_functor_type, return_type_of_first_functor_type)
 *   SIGC_FUNCTOR_TRAIT(second_functor_type, return_type_of_second_functor_type)
 *   ...
 * }
 * @endcode
 *
 * @ingroup sigcfunctors
 */
#define SIGC_FUNCTOR_TRAIT(T_functor,T_return) \
template <>                                    \
struct functor_trait<T_functor,false>          \
{                                              \
  typedef T_return result_type;                \
  typedef T_functor functor_type;              \
};

/** Helper macro, if you want to mix user-defined and third party functors with libsigc++.
 *
 * If you want to mix functors not derived from sigc::functor_base with libsigc++,
 * and your compiler can deduce the result type of the functor with the C++11
 * keyword <tt>decltype</tt>, use this macro inside namespace sigc like so:
 * @code
 * namespace sigc {
 *   SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
 * }
 * @endcode
 *
 * @newin{2,2,11}
 *
 * You can't use both SIGC_FUNCTORS_HAVE_RESULT_TYPE and
 * SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE in the same compilation unit.
 *
 * @ingroup sigcfunctors
 */
#define SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE \
template <typename T_functor>          \
struct functor_trait<T_functor, false> \
{                                      \
  typedef typename functor_trait<decltype(&T_functor::operator()), false>::result_type result_type; \
  typedef T_functor functor_type;      \
};

// detect the return type and the functor version of non-functor types.
template <class T_return> class pointer_functor0;
template <class T_return>
struct functor_trait<T_return (*)(), false>
{
  typedef T_return result_type;
  typedef pointer_functor0<T_return> functor_type;
};

template <class T_arg1, class T_return> class pointer_functor1;
template <class T_arg1, class T_return>
struct functor_trait<T_return (*)(T_arg1), false>
{
  typedef T_return result_type;
  typedef pointer_functor1<T_arg1, T_return> functor_type;
};

template <class T_arg1, class T_arg2, class T_return> class pointer_functor2;
template <class T_arg1, class T_arg2, class T_return>
struct functor_trait<T_return (*)(T_arg1, T_arg2), false>
{
  typedef T_return result_type;
  typedef pointer_functor2<T_arg1, T_arg2, T_return> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_return> class pointer_functor3;
template <class T_arg1, class T_arg2, class T_arg3, class T_return>
struct functor_trait<T_return (*)(T_arg1, T_arg2, T_arg3), false>
{
  typedef T_return result_type;
  typedef pointer_functor3<T_arg1, T_arg2, T_arg3, T_return> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_return> class pointer_functor4;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_return>
struct functor_trait<T_return (*)(T_arg1, T_arg2, T_arg3, T_arg4), false>
{
  typedef T_return result_type;
  typedef pointer_functor4<T_arg1, T_arg2, T_arg3, T_arg4, T_return> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_return> class pointer_functor5;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_return>
struct functor_trait<T_return (*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5), false>
{
  typedef T_return result_type;
  typedef pointer_functor5<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_return> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_return> class pointer_functor6;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_return>
struct functor_trait<T_return (*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6), false>
{
  typedef T_return result_type;
  typedef pointer_functor6<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_return> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_arg7, class T_return> class pointer_functor7;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_arg7, class T_return>
struct functor_trait<T_return (*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7), false>
{
  typedef T_return result_type;
  typedef pointer_functor7<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7, T_return> functor_type;
};


template <class T_return, class T_obj> class mem_functor0;
template <class T_return, class T_obj> class const_mem_functor0;
template <class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(), false>
{
  typedef T_return result_type;
  typedef mem_functor0<T_return, T_obj> functor_type;
};
template <class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)() const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor0<T_return, T_obj> functor_type;
};

template <class T_arg1, class T_return, class T_obj> class mem_functor1;
template <class T_arg1, class T_return, class T_obj> class const_mem_functor1;
template <class T_arg1, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1), false>
{
  typedef T_return result_type;
  typedef mem_functor1<T_arg1, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor1<T_arg1, T_return, T_obj> functor_type;
};

template <class T_arg1, class T_arg2, class T_return, class T_obj> class mem_functor2;
template <class T_arg1, class T_arg2, class T_return, class T_obj> class const_mem_functor2;
template <class T_arg1, class T_arg2, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2), false>
{
  typedef T_return result_type;
  typedef mem_functor2<T_arg1, T_arg2, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_arg2, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor2<T_arg1, T_arg2, T_return, T_obj> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_return, class T_obj> class mem_functor3;
template <class T_arg1, class T_arg2, class T_arg3, class T_return, class T_obj> class const_mem_functor3;
template <class T_arg1, class T_arg2, class T_arg3, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3), false>
{
  typedef T_return result_type;
  typedef mem_functor3<T_arg1, T_arg2, T_arg3, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_arg2, class T_arg3, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor3<T_arg1, T_arg2, T_arg3, T_return, T_obj> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_return, class T_obj> class mem_functor4;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_return, class T_obj> class const_mem_functor4;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4), false>
{
  typedef T_return result_type;
  typedef mem_functor4<T_arg1, T_arg2, T_arg3, T_arg4, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor4<T_arg1, T_arg2, T_arg3, T_arg4, T_return, T_obj> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_return, class T_obj> class mem_functor5;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_return, class T_obj> class const_mem_functor5;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5), false>
{
  typedef T_return result_type;
  typedef mem_functor5<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor5<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_return, T_obj> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_return, class T_obj> class mem_functor6;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_return, class T_obj> class const_mem_functor6;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6), false>
{
  typedef T_return result_type;
  typedef mem_functor6<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor6<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_return, T_obj> functor_type;
};

template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_arg7, class T_return, class T_obj> class mem_functor7;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_arg7, class T_return, class T_obj> class const_mem_functor7;
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_arg7, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7), false>
{
  typedef T_return result_type;
  typedef mem_functor7<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7, T_return, T_obj> functor_type;
};
template <class T_arg1, class T_arg2, class T_arg3, class T_arg4, class T_arg5, class T_arg6, class T_arg7, class T_return, class T_obj>
struct functor_trait<T_return (T_obj::*)(T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7) const, false>
{
  typedef T_return result_type;
  typedef const_mem_functor7<T_arg1, T_arg2, T_arg3, T_arg4, T_arg5, T_arg6, T_arg7, T_return, T_obj> functor_type;
};



} /* namespace sigc */
#endif /* _SIGC_FUNCTORS_MACROS_FUNCTOR_TRAITHM4_ */
