#
# Component Makefile
#
COMPONENT_SRCDIRS := .
COMPONENT_ADD_INCLUDEDIRS := .
COMPILEDATE=`date "+%Y%m%d"`
GITREV=`git rev-parse HEAD | cut -b 1-10`

CFLAGS += -DCOMPILEDATE="\"$(COMPILEDATE)\"" -DGITREV="\"$(GITREV)\""