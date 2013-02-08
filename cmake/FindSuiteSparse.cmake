FIND_PATH(CHOLMOD_INCLUDE_DIR NAMES cholmod.h amd.h camd.h
    PATHS
    /usr/include/suitesparse
    /usr/include/ufsparse
    /opt/local/include/ufsparse
    /usr/local/include/ufsparse
    /usr/local/include
    /sw/include/ufsparse
    NO_DEFAULT_PATH
  )

FIND_LIBRARY(CHOLMOD_LIBRARY NAMES cholmod
     PATHS
     /usr/lib
     /usr/local/lib
     /opt/local/lib
     /sw/lib
     NO_DEFAULT_PATH
   )

FIND_LIBRARY(AMD_LIBRARY NAMES SHARED NAMES amd
  PATHS
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  /sw/lib
  NO_DEFAULT_PATH
  )

FIND_LIBRARY(CAMD_LIBRARY NAMES camd
  PATHS
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  /sw/lib
  NO_DEFAULT_PATH
  )
 
# this one only exists as of suitesparse4
FIND_LIBRARY(SUITESPARSECONFIG_LIBRARY NAMES suitesparseconfig
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )
FIND_LIBRARY(TBB_LIBRARY NAMES tbb
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )
FIND_LIBRARY(SPQR_LIBRARY NAMES spqr
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )

# Different platforms seemingly require linking against different sets of libraries
IF(CYGWIN)
  FIND_PACKAGE(PkgConfig)
  FIND_LIBRARY(COLAMD_LIBRARY NAMES colamd
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )
  PKG_CHECK_MODULES(LAPACK lapack REQUIRED)

  SET(CHOLMOD_LIBRARIES ${SUITESPARSECONFIG_LIBRARY} ${TBB_LIBRARY} ${SPQR_LIBRARY} ${CHOLMOD_LIBRARY} ${AMD_LIBRARY} ${CAMD_LIBRARY} ${COLAMD_LIBRARY} ${CCOLAMD_LIBRARY} ${LAPACK_LIBRARIES})

# MacPorts build of the SparseSuite requires linking against extra libraries

ELSEIF(APPLE)

  FIND_LIBRARY(COLAMD_LIBRARY NAMES colamd
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )

  FIND_LIBRARY(CCOLAMD_LIBRARY NAMES ccolamd
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )

  FIND_LIBRARY(METIS_LIBRARY NAMES metis
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /sw/lib
    NO_DEFAULT_PATH
    )


  SET(CHOLMOD_LIBRARIES ${SUITESPARSECONFIG_LIBRARY} ${TBB_LIBRARY} ${SPQR_LIBRARY} ${CHOLMOD_LIBRARY} ${AMD_LIBRARY} ${CAMD_LIBRARY} ${COLAMD_LIBRARY} ${CCOLAMD_LIBRARY} ${METIS_LIBRARY} "-framework Accelerate")
ELSEIF(SUITESPARSECONFIG_LIBRARY)
  SET(CHOLMOD_LIBRARIES ${CHOLMOD_LIBRARY} ${AMD_LIBRARY} ${SUITESPARSECONFIG_LIBRARY} ${TBB_LIBRARY} ${SPQR_LIBRARY})
ELSE(SUITESPARSECONFIG_LIBRARY)
  SET(CHOLMOD_LIBRARIES ${CHOLMOD_LIBRARY} ${AMD_LIBRARY} ${TBB_LIBRARY} ${SPQR_LIBRARY})
ENDIF(CYGWIN)

IF(CHOLMOD_INCLUDE_DIR AND CHOLMOD_LIBRARIES)
  SET(CHOLMOD_FOUND TRUE)
ELSE(CHOLMOD_INCLUDE_DIR AND CHOLMOD_LIBRARIES)
  SET(CHOLMOD_FOUND FALSE)
ENDIF(CHOLMOD_INCLUDE_DIR AND CHOLMOD_LIBRARIES)

# Look for csparse; note the difference in the directory specifications!
FIND_PATH(CSPARSE_INCLUDE_DIR NAMES cs.h
  PATHS
  /usr/include/suitesparse
  /usr/include
  /opt/local/include
  /usr/local/include
  /sw/include
  /usr/include/ufsparse
  /opt/local/include/ufsparse
  /usr/local/include/ufsparse
  /sw/include/ufsparse
  )

FIND_LIBRARY(CSPARSE_LIBRARY NAMES cxsparse
  PATHS
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  /sw/lib
  NO_DEFAULT_PATH
  )

IF(CSPARSE_INCLUDE_DIR AND CSPARSE_LIBRARY)
  SET(CSPARSE_FOUND TRUE)
ELSE(CSPARSE_INCLUDE_DIR AND CSPARSE_LIBRARY)
  SET(CSPARSE_FOUND FALSE)
ENDIF(CSPARSE_INCLUDE_DIR AND CSPARSE_LIBRARY)
