/* sigc++config.h.  Generated from sigc++config.h.in by configure.  */

#ifndef _SIGCXX_CONFIG_H
#define _SIGCXX_CONFIG_H

// Detect common platforms
#if defined(_WIN32)
  // Win32 compilers have a lot of variation
  #if defined(_MSC_VER)
    #define SIGC_MSC
    #define SIGC_WIN32
    #define SIGC_DLL
  #elif defined(__CYGWIN__)
    #define SIGC_CONFIGURE
  #elif defined(__MINGW32__)
    #define SIGC_WIN32
    #define SIGC_CONFIGURE
  #else
    //The Tru64 compiler complains about this "unrecognized preprocessing directive", but it should never get this far anyway.
    //#warning "libsigc++ config: Unknown win32 architecture (send me gcc --dumpspecs or equiv)"
  #endif
#else
  #define SIGC_CONFIGURE
#endif /* _WIN32 */

#ifdef SIGC_CONFIGURE
  // configure checks
  #define SIGC_GCC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD 1
  #define SIGC_MSVC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD 1
  #define SIGC_SELF_REFERENCE_IN_MEMBER_INITIALIZATION 1

  #define SIGC_HAVE_NAMESPACE_STD 1
/*   #undef SIGC_HAVE_SUN_REVERSE_ITERATOR */
/*   #undef SIGC_TYPEDEF_REDEFINE_ALLOWED */

  // platform specific macros
  // #define LIBSIGC_DISABLE_DEPRECATED
  // #define SIGC_NEW_DELETE_IN_LIBRARY_ONLY
#endif /* SIGC_CONFIGURE */

#ifdef SIGC_MSC

  // MS VC7 Warning 4251 says that the classes to any member objects in an
  // exported class must be also be exported.  Some of the libsigc++
  // template classes contain std::list members.  MS KB article 168958 says
  // that it's not possible to export a std::list instantiation due to some
  // wacky class nesting issues, so our only options are to ignore the
  // warning or to modify libsigc++ to remove the std::list dependency.
  // AFAICT, the std::list members are used internally by the library code
  // and don't need to be used from the outside, and ignoring the warning
  // seems to have no adverse effects, so that seems like a good enough
  // solution for now.
  //
  #pragma warning(disable:4251)

  #define SIGC_MSVC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD
  #define SIGC_NEW_DELETE_IN_LIBRARY_ONLY // To keep ABI compatibility
  #define SIGC_HAVE_NAMESPACE_STD 1

#endif /* SIGC_MSC */

//Put things in the std namespace, if they should be there.
#ifndef SIGC_HAVE_NAMESPACE_STD
  # define SIGC_USING_STD(Symbol) namespace std { using ::Symbol; }
#else
  #  define SIGC_USING_STD(Symbol) /* empty */
#endif

#ifdef SIGC_DLL
  #if defined(SIGC_BUILD) && defined(_WINDLL)
    #define SIGC_API __declspec(dllexport) 
  #elif !defined(SIGC_BUILD)
    #define SIGC_API __declspec(dllimport)
  #else
    #define SIGC_API
  #endif /* SIGC_BUILD - _WINDLL */
#else
  #define SIGC_API
#endif /* SIGC_DLL */

#endif /* _SIGCXX_CONFIG_H */
