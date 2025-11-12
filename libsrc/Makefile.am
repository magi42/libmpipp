lib_LIBRARIES = libmpipp.a
libmpipp_a_SOURCES = mpe++.cc mpi++.cc
libmpiinclude_HEADERS = mpe++.h  mpi++.h
libmpiincludedir = $(includedir)/mpi++
EXTRA_HEADERS = mpe++.h

#  mpistream.cc
#  mpistream.h

INCLUDES = -I$(includedir) @MPI_INCLUDE@

########################################
# We don't want optimization
CXXFLAGS = -g


########################################
# Define to enable shared libraries

# libmagic_a_LDFLAGS = -version-info 1:0:0

