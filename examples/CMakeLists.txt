# Ensure that examples run

SUBDIRS(molecules/H2O)
SUBDIRS(molecules/He)
IF (BUILD_AFQMC AND NOT QMC_COMPLEX)
  SUBDIRS(afqmc/n2_vdz)
ENDIF()
