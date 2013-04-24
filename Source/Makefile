#******************************************************************************
#
# Makefile - Rules for building the ScreenSaver project.
#
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=LM3S8962

#
# The base directory for project.
#
ROOT=.

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=./App/Src
VPATH+=./HAL
VPATH+=./HAL/Oled/Src
VPATH+=./Random
VPATH+=./Utils/Src

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=./App/Head
IPATH+=./HAL/Oled/Head
IPATH+=./Random
IPATH+=./LM3S_Lib/inc
IPATH+=./Utils/Head

#
# The default rule, which causes the screensaver project to be built.
#
all: ${COMPILER}
all: ${COMPILER}/screensaver.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir ${COMPILER}

#
# Rules for building the screensaver project.
#
${COMPILER}/screensaver.axf: ${COMPILER}/main.o
${COMPILER}/screensaver.axf: ${COMPILER}/screen_saver.o
${COMPILER}/screensaver.axf: ${COMPILER}/random.o
${COMPILER}/screensaver.axf: ${COMPILER}/rit128x96x4.o
${COMPILER}/screensaver.axf: ${COMPILER}/uartstdio.o
${COMPILER}/screensaver.axf: ${COMPILER}/ustdlib.o
${COMPILER}/screensaver.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/screensaver.axf: ${ROOT}/LM3S_Lib/DriverLib/libdriver.a
${COMPILER}/screensaver.axf: screensaver.ld
SCATTERgcc_screensaver=screensaver.ld
ENTRY_screensaver=ResetISR

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
