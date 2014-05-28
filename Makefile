
OS  =$(shell uname)
ARCH=$(OS).$(shell uname -m)

all: arec.$(OS)

arec.Darwin : arec.Darwin.i386 arec.Darwin.x86_64
arec.Linux  :                  arec.Linux.x86_64

AREC_SRC = arec.c arec.h arec.tcl


arec.Darwin.i386	: lib/arec/macosx-ix86/arec.dylib
arec.Darwin.x86_64	: lib/arec/macosx-x86_64/arec.dylib
arec.Linux.x86_64 	: lib/arec/linux-x86_64/arec.so

lib/arec/linux-x86_64/arec.so	 : $(AREC_SRC)
	critcl -force -pkg arec

lib/arec/macosx-ix86/arec.dylib  : $(AREC_SRC)
	critcl -target macosx-x86_32 -pkg arec
	#rm -rf lib/arec/macosx-ix86
	#mv lib/arec/macosx-x86_32 lib/arec/macosx-ix86

lib/arec/macosx-x86_64/arec.dylib: $(AREC_SRC)
	critcl -target macosx-x86_64 -pkg arec


test : arec-struct.$(OS) test.$(OS)

test.Darwin : arec-test.tcl
	arch -i386   /usr/local/bin/tclsh8.6 ./arec-test.tcl 
	arch -x86_64 /usr/local/bin/tclsh8.6 ./arec-test.tcl

test.Linux : arec-test.tcl
	tclsh8.6 ./arec-test.tcl

arec-struct.Linux  : lib/arec-struct/linux-x86_64/arec-struct.so

lib/arec-struct/linux-x86_64/arec-struct.so : arec-struct.tcl
	critcl -pkg arec-struct

arec-struct.Darwin : lib/arec-struct/macosx-x86_64/arec-struct.dylib lib/arec-struct/macosx-ix86/arec-struct.dylib


lib/arec-struct/macosx-x86_64/arec-struct.dylib : arec-struct.tcl
	critcl -target macosx-x86_64 -pkg arec-struct

lib/arec-struct/macosx-ix86/arec-struct.dylib : arec-struct.tcl
	critcl -target macosx-x86_32 -pkg arec-struct
	#rm -rf lib/arec-struct/macosx-ix86
	#mv lib/arec-struct/macosx-x86_32 lib/arec-struct/macosx-ix86


FORCE:
