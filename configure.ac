AC_INIT([volbirdr],[0.1])

dnl This configure script is a combination of the R-projects hdf5r and proj4 configure scripts. 
dnl so that this project uses same dependencies as the above mentioned projects
dnl hdf5r can be found here: https://github.com/hhoeflin/hdf5r
dnl proj https://github.com/s-u/proj4
dnl
m4_include([inst/m4/ax_lib_hdf5.m4])
m4_include([inst/m4/ax_compare_version.m4])
m4_include([inst/m4/hh_hdf5rlib.m4])

AX_LIB_HDF5()

if test "$with_hdf5" = "yes"; then 
AC_MSG_RESULT([Found hdf5 with version: $HDF5_VERSION])
AX_COMPARE_VERSION([$HDF5_VERSION], [ge], [1.8.12], [hdf5_version_sufficient="yes"], AC_MSG_ERROR([The version of hdf5 installed on your system is not sufficient. Please ensure that at least version 1.8.12 is installed]))
else 
  AC_MSG_ERROR([hdf5 does not seem to be installed on your platform.
  Please install the hdf5 library.
  The required HDF5 library files can be installed as follows:
      - Debian-based (e.g. Debian >= 8.0, Ubuntu >= 15.04): 'sudo apt-get install libhdf5-dev'
        - Old Debian-based (e.g Debian < 8.0, Ubuntu < 15.04): Install from source (see INSTALL)
        - OS X using Homebrew: 'brew install hdf5'
        - RPM-based (e.g Fedora): 'sudo yum install hdf5-devel'])
fi

if test "$hdf5_version_sufficient" = "yes"; then 
  AX_COMPARE_VERSION(
	[$HDF5_VERSION], 
	[ge], 
	[1.12.0], 
	[HDF5_API_VERS_USE="-DH5_USE_110_API"],
        [HDF5_API_VERS_USE=""])
fi

AC_SUBST(HDF5_VERSION_USE)
AC_SUBST(HDF5_API_VERS_USE)
AC_SUBST(DEFINE_H5FREE_MEMORY)

: ${R_HOME=`R RHOME`}
if test -z "${R_HOME}"; then
  echo "could not determine R_HOME"
  exit 127
fi
RBIN="${R_HOME}/bin/R"

# pick all flags for testing from R
: ${CC=`"${RBIN}" CMD config CC`}
: ${CFLAGS=`"${RBIN}" CMD config CFLAGS`}
: ${CPPFLAGS=`"${RBIN}" CMD config CPPFLAGS`}
: ${LDFLAGS=`"${RBIN}" CMD config LDFLAGS`}
: ${CPP="$CC -E"}
: ${CXX=`"${RBIN}" CMD config CXX`}
: ${CXXFLAGS=`"${RBIN}" CMD config CXXFLAGS`}

if test -z "${LIBS}"; then
  dnl prefer ggrep if installed
  AC_CHECK_PROG(GREP,ggrep,`which ggrep`, `which grep`)

  dnl check if $GREP supports option -o
  AC_MSG_CHECKING([whether ${GREP} accepts -o])
  if `echo '' | ${GREP} -o '' >/dev/null 2>&1`
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([${GREP} does not support option -o])
  fi
  [
  LIBS=`${H5CC} -show | ${GREP} -o " \-[lL][^[:space:]]\+" | tr "\n" " "` 
  ]
  
  dnl Fix for newer Debian versions
  VOL2BIRD_LIBS="${LIBS} -L. -lhdf5 -lz -lm"
fi

if test -z "${CPPFLAGS}"; then
  dnl prefer ggrep if installed
  AC_CHECK_PROG(GREP,ggrep,`which ggrep`, `which grep`)

  dnl check if $GREP supports option -o
  AC_MSG_CHECKING([whether ${GREP} accepts -o])
  if `echo '' | ${GREP} -o '' >/dev/null 2>&1`
  then
    AC_MSG_RESULT([yes])
  else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([${GREP} does not support option -o])
  fi

  echo 'int main() { return; }' > myconftest.c


  [
  VOL2BIRD_CPPFLAGS=`${H5CC} -show -c myconftest.c | ${GREP} -o "\-I[^[:space:]]\+" | tr "\n" " "`
  ]
  rm -f myconftest.c
fi

dnl If running mac osx we want to check if we have brew so that we can identify PROJ...
dnl
kernelname=`uname -s | tr 'A-Z' 'a-z'`
ISMACOS=no
case "$kernelname" in
  darwin*)
    ISMACOS=yes
    ;;
esac

HASHOMEBREW=no
HOMEBREWPREFIX=
if [[ "$ISMACOS" = "yes" ]]; then
  AC_MSG_CHECKING(for homebrew)
  which brew
  if [[ $? -ne 0 ]]; then
    AC_MSG_RESULT(not found)
  else
    HASHOMEBREW=yes  
    AC_MSG_RESULT(found)
  fi
  
  if [[ "$HASHOMEBREW" = "yes" ]]; then
    AC_MSG_CHECKING(for homebrew prefix)
    HOMEBREWPREFIX=`brew --prefix`
    if [[ $? -ne 0 ]]; then
      HOMEBREWPREFIX=
      AC_MSG_RESULT(not found)
    else
      AC_MSG_RESULT($HOMEBREWPREFIX)
    fi
  fi
  
  if [[ "$HOMEBREWPREFIX" != "" ]]; then
    LIBS="$LIBS -L$HOMEBREWPREFIX/lib"
    CPPFLAGS="$CPPFLAGS -I$HOMEBREWPREFIX/include"
  fi
fi

# honor PKG_xx overrides
LIBS="${PKG_LIBS} ${LIBS}"
# for CPPFLAGS we will superfluously double R's flags
# since we'll set PKG_CPPFLAGS with this, but that shouldn't hurt
CPPFLAGS="${PKG_CPPFLAGS} ${CPPFLAGS} "

with_proj6=no
AC_ARG_ENABLE([proj6],
  AS_HELP_STRING(--enable-proj6, [use the new PROJ6 API even if PROJ4 API is available. The default is @<:@disable@:>@ since PROJ6 API has issues, use with caution.]),
  [with_proj6=$enableval], [with_proj6=$with_proj6])

# unless forced, try to use PROJ4 first
useproj6=no
proj4ok=yes
proj6ok=no

AC_CHECK_HEADERS(proj_api.h,,[
  AC_MSG_CHECKING([with ACCEPT_USE_OF_DEPRECATED_PROJ_API_H])
  AC_MSG_RESULT([])
  savCPPFLAGS="$CPPFLAGS"
  CPPFLAGS="-DACCEPT_USE_OF_DEPRECATED_PROJ_API_H=1 $CPPFLAGS"
  unset ac_cv_header_proj_api_h
  AC_CHECK_HEADERS(proj_api.h,,[
    proj4ok=no
    CPPFLAGS="$savCPPFLAGS"
  ])
])

dnl changes needed for static PROJ 6
AC_LANG(C++)
LIBS="${LIBS} ${PKG_LIBS}"

AC_CHECK_LIB(proj,pj_init_plus,,proj4ok=no)

AC_MSG_CHECKING([whether to require PROJ6 API])
if [[ "$with_proj6" = "yes" -o "$proj4ok" = "no" ]]; then
  AC_MSG_RESULT([yes])
  useproj6=yes
else
  AC_MSG_RESULT([no])
fi

if [[ "$useproj6" = "yes" ]]; then
  proj6ok=yes
  LIBS="${LIBS} -lsqlite3"
  AC_CHECK_HEADERS(proj.h,, proj6ok=no)
  AC_CHECK_LIB(proj, proj_create_crs_to_crs,, proj6ok=no)
fi

AC_MSG_CHECKING([If proj6 is usable])
AC_MSG_RESULT([$proj6ok])
if [[ "$proj6ok" = "yes" ]]; then
   CPPFLAGS="${CPPFLAGS}"
else
   CPPFLAGS="${CPPFLAGS} -DUSE_PROJ4_API=1"
fi  

if [[ "$proj4ok" = "no" -a "$proj6ok" = "no" ]]; then
   AC_MSG_ERROR([libproj and/or proj.h/proj_api.h not found in standard search locations.

*** Install PROJ library and if necessary set PKG_CPPFLAGS/PKG_LIBS accordingly.])
fi

AC_ARG_VAR([PKG_CPPFLAGS],[custom C preprocessor flags for packages compilation])
AC_ARG_VAR([PKG_LIBS],[custom libraries for package compilation])

VOL2BIRD_LIBS="$VOL2BIRD_LIBS $LIBS"
VOL2BIRD_CPPFLAGS="$VOL2BIRD_CPPFLAGS $CPPFLAGS"

dnl dummy substitute HDF5R_CFLAGS
VOL2BIRD_CFLAGS=""
AC_SUBST(VOL2BIRD_CFLAGS)

echo "VOL2BIRD_CPPFLAGS=$VOL2BIRD_CPPFLAGS"
echo "VOL2BIRD_LIBS=$VOL2BIRD_LIBS"

dnl substitute CPPFLAGS and LIBS
AC_SUBST(VOL2BIRD_CPPFLAGS)
AC_SUBST(VOL2BIRD_LIBS)
dnl and do subsitution in the src/Makevars.in 
AC_CONFIG_FILES([src/Makevars])
AC_OUTPUT
  
