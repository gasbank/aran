CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(pym)

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
