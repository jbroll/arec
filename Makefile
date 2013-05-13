
OS  =$(shell uname)
ARCH=$(OS).$(shell uname -m)

all: arec.$(OS)

arec.Darwin : arec.Darwin.i386 arec.Darwin.x86_64 test.Darwin
arec.Linux  :                  arec.Linux.x86_64  test.Linux


arec.Darwin.i386	: lib/arec/macosx-ix86/arec.dylib
arec.Darwin.x86_64	: lib/arec/macosx-x86_64/arec.dylib
arec.Linux.x86_64 	: lib/arec/linux-x86_64/arec.so

lib/arec/linux-x86_64/arec.so : arec.c arec.h arec.tcl
	critcl -force -target linux-x86_64 -pkg arec

lib/arec/macosx-ix86/arec.dylib : arec.c arec.h arec.tcl
	critcl -target macosx-x86_32 -pkg arec
	rm -rf lib/arec/macosx-ix86
	mv lib/arec/macosx-x86_32 lib/arec/macosx-ix86

lib/arec/macosx-x86_64/arec.dylib : arec.c arec.h arec.tcl
	critcl -target macosx-x86_64 -pkg arec


test : test.$(OS)

test.Darwin : FORCE
	arch -i386   /usr/local/bin/tclsh8.6 ./arec-test.tcl 
	arch -x86_64 /usr/local/bin/tclsh8.6 ./arec-test.tcl

test.Linux : FORCE
	tclsh8.6 ./arec-test.tcl

struct : struct.tcl
	critcl -target macosx-x86_64 -pkg struct



FORCE:
