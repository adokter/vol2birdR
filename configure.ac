AC_INIT([volbirdr],[0.1])

dnl This configure script uses three different scripts (free software) for finding dependencies
dnl inst/m4/ax_lib_proj.m4 for identifying PROJ include & libraries.
dnl inst/m4/ax_lib_hdf5.m4 for identifying HDF5 include & libraries.
dnl inst/m4/ax_compare_version.m4  for comparing HDF5-versions
dnl
dnl All above scripts are open source, use as you want provided that copyright notice is kept. 
dnl
m4_include([inst/m4/ax_lib_hdf5.m4])
m4_include([inst/m4/ax_compare_version.m4])
m4_include([inst/m4/ax_lib_proj.m4])

AX_LIB_HDF5()

if test "$with_hdf5" = "yes"; then 
  AC_MSG_RESULT([Found hdf5 with version: $HDF5_VERSION])
  AX_COMPARE_VERSION([$HDF5_VERSION], 
    [ge], 
    [1.8.12], 
    [hdf5_version_sufficient="yes"], 
    AC_MSG_ERROR([The version of hdf5 installed on your system is not sufficient. Please ensure that at least version 1.8.12 is installed])
  )
else 
  AC_MSG_ERROR([hdf5 does not seem to be installed on your platform.])
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

AX_LIB_PROJ([])

if [[ "$PROJ_SUPPRESSED" = "yes" ]]; then
  AC_MSG_ERROR([proj required and should not be suppressed.])  
fi

if [[ "$PROJ_FOUND" = "yes" ]]; then
  if [[ "$PROJ_VARIANT" = "4" ]]; then
    VOL2BIRD_CPPFLAGS="$VOL2BIRD_CPPFLAGS -DUSE_PROJ4_API=1"
  fi
else
  AC_MSG_ERROR([proj does not seem to be installed on your platform.])  
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
  
