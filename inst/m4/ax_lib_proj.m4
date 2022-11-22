#
# SYNOPSIS
#
#   AC_LIB_PROJ([keepvar])
#
#   Optional argument 'keepvar' that will indicate to the script that CPPFLAGS & LDFLAGS should be restored after
#   macro has been run.
#
# DESCRIPTION
#
#   Macro for identifying PROJ.4 (and/or) PROJ.6 library. 
#
#   The macro provides two options.
#
#   --with-proj=[yes | no | path] 
#                        where path can either be a proj-root or a commaseparated list of <root>/include,<other-root>/lib
#                        default is "yes" meaning that pkg-config first will be used to identify compiler flags. After that, if on mac and
#                        pkg-config couldn't be found, brew will be identified and used to specify generic include/library flags.
#        
#                        If path is specified, then this path will be used for an atempt to compile the code.
#
#   --enable-proj6       Will force building against PROJ >= 6. The default behaviour is however to always first try proj.4 and if that
#                        doesn't work out. Proj >= 6 will be tried instead.
#
#   The macro first checks if with-proj was specified with path or not. If path was specified, that include/library
#   paths will be used for setting CFLAGS & LIBS and try to compile the software. This branch of the macro will also
#   ensure that it can find proj_api.h/proj.h in the specified include-dir and libproj* in specified lib-dir. If that isn't possible
#   the macro will abort since there must be a manual error.
#
#   If path wasn't specified, the macro will instead use pkg-config to identify CFLAGS & LIBS if available. If that doesn't work if will keep
#   on by checking if homebrew is available and in that case use the standard prefix as include/lib.
#
#   As a final resort, the macro will go back to use standard paths.
#
# 
#
#   The following variables will be set after this function has been run.
#
#   PROJ_FOUND=yes|no         - If proj could be identified or not.
#   PROJ_SUPPRESSED=yes|no    - If proj actively was suppressed by user (--with-proj=no)
#   PROJ_VARIANT=4|6          - If proj variant 4 or >= 6 was identified.
#   PROJ_CFLAGS=..            - Compiler flags that should be used.
#   PROJ_LIBS=..              - Linker flags that should be used.
#
#   CPPFLAGS & LIBS will be updated with new flags unless AX_LIB_PROJ is called with [keepvar]. If not keepvar is specified
#   both CPPFLAGS & LIBS will be updated with flags used when successfully identifying the proj-version
#
# LICENSE
#   Copyright (c) 2022 Anders Henja (anders@henjab.se)
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.
AC_DEFUN([AX_LIB_PROJ], [

PROJ_CFLAGS=""
PROJ_LIBS=""
PROJ_VARIANT=X
PROJ_FOUND=no
PROJ_SUPPRESSED=no

ax_lib_proj_path=
ax_lib_proj_with_proj=yes
ax_lib_proj_useproj6=no
ax_lib_proj_proj4ok=yes
ax_lib_proj_proj6ok=no
ax_lib_proj_with_proj6=no
ax_lib_proj_pkg_config_identified=no
ax_lib_proj_fun_arg=$1 # Store argument when calling AX_LIB_PROJ([keepvar])

# Add a default --with-proj configuration option.
AC_ARG_WITH([proj],
  AS_HELP_STRING(
    [--with-proj=[yes|no|<path to proj>]],
            [location of <proj>-root or a comma separated list specifying include and library directory]
  ),
  [ if test "$withval" = "no" -o "$withval" = "yes"; then
      ax_lib_proj_with_proj="$withval"
    else
      ax_lib_proj_with_proj="yes"
      ax_lib_proj_path="$withval"
    fi
  ],
  [ax_lib_proj_with_proj="yes"]
)

AC_ARG_ENABLE([proj6],
  AS_HELP_STRING(
    --enable-proj6, 
    [use the new PROJ6 API even if PROJ4 API is available.]
  ),
  [ax_lib_proj_with_proj6=$enableval], 
  [ax_lib_proj_with_proj6=$ax_lib_proj_with_proj6]
)
  
# Keep track of LIBS before we start identifying PROJ
ax_lib_proj_save_LIBS=${LIBS}
ax_lib_proj_save_CPPFLAGS=${CPPFLAGS}

if [[ "$ax_lib_proj_with_proj" != "no" ]]; then
  if [[ "$ax_lib_proj_with_proj" = "yes" -a "$ax_lib_proj_path" = "" ]]; then
    AC_MSG_NOTICE([Trying to identify PROJ using pkg-config])

    AC_MSG_CHECKING([for proj using pkg-config])
    if pkg-config proj; then
      AC_MSG_RESULT([yes])

      ax_lib_proj_pkg_config_identified=yes

      AC_MSG_CHECKING([for proj CFLAGS])
      PROJ_CFLAGS=`pkg-config --cflags proj`
      AC_MSG_RESULT($PROJ_CFLAGS)

      # TODO: Replace PROJ_LIBS with --libs-only-l / --libs-only-L so that we
      # can set two different variables PROJ_LIBRARIES & PROJ_LDFLAGS instead of the
      # generic PROJ_LIBS that can be derived from the two above. Order of libraries
      # might get scrambled which in some cases is not wanted
      AC_MSG_CHECKING([for proj LIBS])
      PROJ_LIBS=`pkg-config --libs proj`
      AC_MSG_RESULT($PROJ_LIBS)

      AC_MSG_CHECKING([for static proj LIBS, if any])
      STATIC_PROJ_LIBS=`pkg-config --static --libs proj`
      AC_MSG_RESULT($STATIC_PROJ_LIBS)
  
      CPPFLAGS="${CPPFLAGS} ${PROJ_CFLAGS}"
      LIBS="${LIBS} ${PROJ_LIBS}"
    else
      AC_MSG_RESULT([no])
    fi
  fi

  if [[ "$ax_lib_proj_pkg_config_identified" = "no" -a "$ax_lib_proj_path" != "" ]]; then
    if [[ "`echo $ax_lib_proj_path | grep ','`" = "" ]]; then
      ax_lib_proj_inc=$ax_lib_proj_path/include
      ax_lib_proj_lib=$ax_lib_proj_path/lib
    else
      ax_lib_proj_inc="`echo $ax_lib_proj_path |cut -f1 -d,`"
      ax_lib_proj_lib="`echo $ax_lib_proj_path |cut -f2 -d,`"
    fi
  
    AC_MSG_CHECKING([Checking if proj.h or proj_api.h can be found in proj-path])
    if [[ ! -f "$ax_lib_proj_inc/proj.h" -o ! -f "$ax_lib_proj_inc/proj_api.h" ]]; then
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([Could not identify proj.h or proj_api in include directory $ax_lib_proj_inc, aborting!])
    else
      AC_MSG_RESULT([yes])
    fi
    AC_MSG_CHECKING([Checking if libproj can be found in proj-path])
    TMP=`ls -1 "$ax_lib_proj_lib"/libproj.* 2>/dev/null`
    if [[ "$TMP" = "" ]]; then
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([Could not identify libproj in directory $ax_lib_proj_inc, aborting!])
    else
      AC_MSG_RESULT([yes])
    fi
      
    PROJ_LIBS="-L$ax_lib_proj_lib -lproj"
    PROJ_CFLAGS="-I$ax_lib_proj_inc"
    CPPFLAGS="${CPPFLAGS} ${PROJ_CFLAGS}"
    LIBS="${LIBS} ${PROJ_LIBS}"
  elif [[ "$ax_lib_proj_pkg_config_identified" = "no" ]]; then
    # If we can't identify pkg-config but is on mac, we might be able to use
    # homebrew to at least get basic flags
    ax_lib_proj_kernelname=`uname -s | tr 'A-Z' 'a-z'`
    ax_lib_proj_ismacos=no
    case "$ax_lib_proj_kernelname" in
      darwin*)
        ax_lib_proj_ismacos=yes
        ;;
    esac

    ax_lib_proj_hasbrew=no
    ax_lib_proj_homebrewprefix=
    if [[ "$ax_lib_proj_ismacos" = "yes" ]]; then
      AC_MSG_CHECKING(for homebrew)
      which brew
      if [[ $? -ne 0 ]]; then
        AC_MSG_RESULT([not found])
      else
        ax_lib_proj_hasbrew=yes  
        AC_MSG_RESULT([found])
      fi
  
      if [[ "$ax_lib_proj_hasbrew" = "yes" ]]; then
        AC_MSG_CHECKING([for homebrew prefix])
        ax_lib_proj_homebrewprefix=`brew --prefix`
        if [[ $? -ne 0 ]]; then
          ax_lib_proj_homebrewprefix=
          AC_MSG_RESULT([not found])
        else
          AC_MSG_RESULT($ax_lib_proj_homebrewprefix)
        fi
      fi
  
      if [[ "$ax_lib_proj_homebrewprefix" != "" ]]; then
        PROJ_LIBS="-L$ax_lib_proj_homebrewprefix/lib -lproj"
        PROJ_CFLAGS="-I$HOMEBREWPREFIX/include"
        CPPFLAGS="${CPPFLAGS} ${PROJ_CFLAGS}"
        LIBS="${LIBS} ${PROJ_LIBS}"
      fi
    fi
  fi

  if [[ "$ax_lib_proj_with_proj6" != "yes" ]]; then
    # If proj6 isn't forced, then first atempt to find proj.4
    AC_CHECK_HEADERS(proj_api.h, [ax_lib_proj_proj4ok=yes], [ax_lib_proj_proj4ok=no])
    if [[ "$ax_lib_proj_proj4ok" = "no" ]]; then
      #  From PROJ >= 6, the proj_api.h header file was deprecated and requires ACCEPT_USE_OF_DEPRECATED_PROJ_API_H.
      #  So we try to compile with that flag before we give up.
      AC_MSG_CHECKING([with ACCEPT_USE_OF_DEPRECATED_PROJ_API_H])
      ax_lib_proj_save_CPPFLAGS="$CPPFLAGS"
      CPPFLAGS="-DACCEPT_USE_OF_DEPRECATED_PROJ_API_H=1 $CPPFLAGS"
      unset ac_cv_header_proj_api_h
      AC_CHECK_HEADERS(proj_api.h,[
          ax_lib_proj_proj4ok=yes
          PROJ_CFLAGS="$PROJ_CFLAGS -DACCEPT_USE_OF_DEPRECATED_PROJ_API_H=1"
        ],[
          ax_lib_proj_proj4ok=no
          CPPFLAGS="$ax_lib_proj_save_CPPFLAGS"
      ])
    fi
  
    if [[ "$ax_lib_proj_proj4ok" = "yes" ]] ; then
      AC_CHECK_LIB(proj, pj_init_plus, [PROJ_VARIANT=4], [ax_lib_proj_proj4ok=no])
      if [[ "$ax_lib_proj_proj4ok" = "no" -a "$ax_lib_proj_pkg_config_identified" = "yes" ]]; then
        AC_MSG_NOTICE([Retrying with static proj libraries])
        LIBS="${ax_lib_proj_save_LIBS} ${STATIC_PROJ_LIBS}"
        AC_CHECK_LIB(proj, pj_init_plus, [ 
          ax_lib_proj_proj4ok=yes
          PROJ_VARIANT=4
          PROJ_LIBS="$STATIC_PROJ_LIBS" 
        ], [
          LIBS="$ax_lib_proj_save_LIBS"
        ])
      elif [[ "$ax_lib_proj_proj4ok" = "no" ]]; then
        AC_MSG_NOTICE([Could not identify PROJ.4])
      fi
    fi
  else
    ax_lib_proj_proj4ok=no  
  fi

  AC_MSG_CHECKING([whether to require PROJ6 API])
  if [[ "$ax_lib_proj_with_proj6" = "yes" -o "$ax_lib_proj_proj4ok" = "no" ]]; then
    AC_MSG_RESULT([yes])
    ax_lib_proj_useproj6=yes
  else
    AC_MSG_RESULT([no])
  fi

  if [[ "$ax_lib_proj_useproj6" = "yes" ]]; then
    savedp6_LIBS=$LIBS
    ax_lib_proj_proj6ok=yes
    AC_CHECK_HEADERS(proj.h,, ax_lib_proj_proj6ok=no)
    if [[ "$ax_lib_proj_proj6ok" = "yes" ]]; then
      AC_CHECK_LIB(proj, proj_create_crs_to_crs, [PROJ_VARIANT=6], [ax_lib_proj_proj6ok=no])
      if [[ "$ax_lib_proj_proj6ok" = "no" -a "$ax_lib_proj_pkg_config_identified" = "yes" ]]; then
        AC_MSG_NOTICE([Retrying with static proj libraries])
        LIBS="${ax_lib_proj_save_LIBS} ${STATIC_PROJ_LIBS}"
        AC_CHECK_LIB(proj, proj_create_crs_to_crs, [ 
    	    ax_lib_proj_proj6ok=yes
    	    PROJ_VARIANT=6
    	    PROJ_LIBS="$STATIC_PROJ_LIBS" 
          ], [
    	    LIBS="$savedp6_LIBS"
        ])
      elif [[ "$ax_lib_proj_proj6ok" = "no" ]]; then
        AC_MSG_NOTICE([Could not identify PROJ.6])
      fi
    fi
  fi

  AC_MSG_CHECKING([If PROJ could be identified])
  if [[ "$ax_lib_proj_proj4ok" = "yes" -o "$ax_lib_proj_proj6ok" = "yes" ]]; then
    AC_MSG_RESULT([Yes])
    PROJ_FOUND=yes
    AC_MSG_NOTICE([PROJ variant: $PROJ_VARIANT])
    AC_MSG_NOTICE([PROJ cflags:  $PROJ_CFLAGS])
    AC_MSG_NOTICE([PROJ libs:    $PROJ_LIBS])
  else
    AC_MSG_RESULT([No])
    AC_MSG_ERROR([Proj not found in standard search locations. Install proj.4/proj.6 library])  
  fi
else
  AC_MSG_NOTICE([PROJ check suppressed])
  PROJ_SUPPRESSED=yes
fi

if [[ "$ax_lib_proj_fun_arg" = "keepvar" ]]; then
  LIBS=$ax_lib_proj_save_LIBS
  CPPFLAGS=$ax_lib_proj_save_CPPFLAGS
fi

])


