
arec32	= lib/arec/macosx-ix86/arec.dylib
arec64	= lib/arec/macosx-x86_64/arec.dylib

all: $(arec32)  $(arec64)


$(arec32) : arec.c arec.h arec.tcl
	critcl -target macosx-x86_32 -force -pkg arec
	rm -rf lib/arec/macosx-ix86
	mv lib/arec/macosx-x86_32 lib/arec/macosx-ix86

$(arec64) : arec.c arec.h arec.tcl
	critcl -target macosx-x86_64 -force -pkg arec
	#rm -rf lib/arec/macosx-ix86
	#mv lib/arec/macosx-x86_32 lib/arec/macosx-ix86


test : FORCE
	arch -i386   /usr/local/bin/tclsh8.6 ./arec-test.tcl 
	arch -x86_64 /usr/local/bin/tclsh8.6 ./arec-test.tcl


FORCE:
