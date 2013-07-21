
namespace eval arec {}

int ARecRegisterMethod(char *type, char *name, int flags)
{
    if ( !strcmp(type, "arec") ) {
	ARecAddMethod(ARecMethods, name, flags);
    } else {
	if ( (type = ARecLookupType(type)) == NULL ) {
	    return TCL_ERROR;
	}

	ARecAddMethod(type->methods, name, flags);
    }
    return TCL_OK;
}

# type	  - Type of ARec to add method to.  If type is "arec" add method as general method
#           available to all types.
#
# name	  - name of method.
#
# areg	  - critcl style args.
#
# rettype - critcl style return type.
#
# body	  - C code.
#
proc arec::cmethod { type name args rettype body } {
    variable UnPack

    lassign [critcl::name2c $name] ns cns name cname

    critcl::cproc $name $args $rettype "$UnPack\n$body" -pass-cdata 1 -register no
    critcl::cinit "ARecRegisterMethod(\"$type\", tcl_$cns$cname, 0);" {}
}

    critcl::ccode {
	typedef struct _ARecCD {
	    void *inst;
	    void *recs;
	    int   nrec;
	} ARecCD;
    }

    set arec::UnPack {
	/* unpack the ARec ClientData */
	ARecCD *__arec_cd = (ARecCD*) clientdata;

	void *inst = __arec_cd->type;
	void *recs = __arec_cd->recs;
	int  *nrec = __arec_cd->nrec;
    }


arec::cmethod arec dodo { int x int y } int {
    return x*y;
}

package provide arec-extend 1.0

