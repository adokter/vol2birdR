AC_INIT([volbirdr],[0.1])

dnl This configure script uses three different scripts (free software) for finding dependencies
dnl tools/m4/ax_lib_hdf5.m4 for identifying HDF5 include & libraries.
dnl tools/m4/ax_compare_version.m4  for comparing HDF5-versions
dnl tools/m4/check_proj.m4 for identifying proj
dnl
dnl All above scripts are open source, use as you want provided that copyright notice is kept. 
dnl
m4_include([tools/m4/ax_lib_hdf5.m4])
m4_include([tools/m4/ax_compare_version.m4])
m4_include([tools/m4/check_proj.m4])

AX_LIB_HDF5()

hdf5_version_sufficient=no
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

# HDF5_LIBS is in bad order and some linkers might obstruct. Reverse it
NEW_HDF5_LIBS=""
for x in $HDF5_LIBS; do
  case "$x" in
    -lhdf5*)
      # HDF5_LIBS contains -hdf5xx and we don't want that since we aren't interested in hdf5_hl...
      ;;
    *)
      NEW_HDF5_LIBS="$x $NEW_HDF5_LIBS"
      ;;
  esac
done

VOL2BIRD_LIBS="-lhdf5 $NEW_HDF5_LIBS -lz -lm"
VOL2BIRD_LDFLAGS="$HDF5_LDFLAGS"
VOL2BIRD_CPPFLAGS="$HDF5_CPPFLAGS"

LIBS="$LIBS $VOL2BIRD_LIBS"
LDFLAGS="$LDFLAGS $VOL2BIRD_LDFLAGS"
CPPFLAGS="$CPPFLAGS $VOL2BIRD_CPPFLAGS"

CHECK_PROJ([keepvar])

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

VOL2BIRD_LIBS="$VOL2BIRD_LDFLAGS $PROJ_LDFLAGS $PROJ_LIBS $VOL2BIRD_LIBS $PKG_LIBS"
VOL2BIRD_CPPFLAGS="$PKG_CPPFLAGS $PROJ_CFLAGS $VOL2BIRD_CPPFLAGS"

echo "VOL2BIRD_CPPFLAGS=$VOL2BIRD_CPPFLAGS"
echo "VOL2BIRD_LIBS=$VOL2BIRD_LIBS"

# Optional local copy of PROJ_LIB
# Used for making proj.db available in binaries
# Modified after RGDAL by Adriaan Dokter on 2/5/2025
# Note: PROJ_GDAL_DATA_COPY and PROJ_LIB are environment variables typically set by build server
data_copy=no
if test "${PROJ_GDAL_DATA_COPY}" ; then
    data_copy=yes
    AC_MSG_NOTICE([PROJ_GDAL_DATA_COPY used.])
else
    AC_ARG_WITH([data-copy],
        AS_HELP_STRING([--with-data-copy=yes/no],[local copy of data directories in package, default no]),
               [data_copy=$withval])
fi
if test "${data_copy}" = "yes" ; then
AC_MSG_NOTICE([Copy data for:])
  proj_lib0="${PROJ_LIB}"
  AC_ARG_WITH([proj-data],
    AS_HELP_STRING([--with-proj-data=DIR],[location of PROJ data directory]),
    [proj_lib1=$withval])
  if test -n "${proj_lib0}" ; then
    proj_lib="${proj_lib0}"
  else
    proj_lib="${proj_lib1}"
  fi
  if test -n "${proj_lib}" ; then
    if test -d "${proj_lib}" ; then
      cp -r "${proj_lib}" "${R_PACKAGE_DIR}"
      AC_MSG_NOTICE([  PROJ: ${proj_lib}])
    else
      AC_MSG_ERROR([PROJ data files not found; set environment variable PROJ_LIB=DIR or --with-proj-data=DIR.])
    fi
  else
      AC_MSG_ERROR([PROJ data files not found; set environment variable PROJ_LIB=DIR or --with-proj-data=DIR.])
  fi
fi


ENABLE_IRIS2ODIM=no
AC_MSG_CHECKING([if iris2odim support can be enabled])
if [[ -f src/libiris2odim/iris2odim.c -a -f src/includes/libiris2odim/iris2odim.h ]]; then
  AC_MSG_RESULT([yes])
  ENABLE_IRIS2ODIM=yes
else
  AC_MSG_RESULT([no])
fi

dnl substitute CPPFLAGS and LIBS
AC_SUBST(VOL2BIRD_CPPFLAGS)
AC_SUBST(VOL2BIRD_LIBS)
AC_SUBST(HDF5_API_VERS_USE)
AC_SUBST(ENABLE_IRIS2ODIM)

dnl and do subsitution in the src/Makevars.in 
AC_CONFIG_FILES([src/Makevars])
AC_OUTPUT
