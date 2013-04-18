
arec	= lib/arec/pkgIndex.tcl

all: $(arec) 


$(arec) : arec.c arec.h arec.tcl
	critcl -target macosx-x86_32 -force -pkg arec
	rm -rf lib/arec/macosx-ix86
	mv lib/arec/macosx-x86_32 lib/arec/macosx-ix86


test : FORCE
	./arec-test.tcl 

FORCE:
