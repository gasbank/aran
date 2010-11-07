CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pym)
SET(CMAKE_OSX_ARCHITECTURES i386)
SET(PYM_3RDPARTIES_ROOT c:/devel/src/)
SET(MOSEK_ROOT "C:/Program Files/Mosek/")
SET(BOOST_ROOT C:/boost_1_44_0)

# ---------- Setup output Directories -------------------------
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/bin )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/bin )

# --------- Setup the Executable output Directory -------------
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
   ${PROJECT_BINARY_DIR}/bin )

ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(parser)
ADD_SUBDIRECTORY(optimizer)
ADD_SUBDIRECTORY(rs)
