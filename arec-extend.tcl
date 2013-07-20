
# ARec extention signature:
#
#    method(Tcl_Interp* ip, ARecType type, void *recs, int nrec, objv, objc)

#critcl::api ARecRegisterMethod(type, name, function);

namespace eval arec {}


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

    set code {}
    critcl::cproc arec::$name $args $rettype
}

arec::cmethod arec dodo { int x int y } int {
    return x*y;
}

package provide arec-extend 1.0



