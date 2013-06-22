#!/usr/bin/env tclsh8.6
#
lappend auto_path ../lib lib /Users/john/lib

package require tcltest
package require arec

package require critcl 3.1

package provide arec-struct 0.4

source perm.tcl
source ../tna/template.tcl
source ../tna/functional.tcl

set Types [arec::types get name]
set Sizes [arec::types get size]

array set TMap {
    char	char
    uchar	"unsigned char"
    short	short
    ushort	"unsigned short"
    int		int
    uint	"unsigned int"
    long	long
    ulong	"unsigned long"
    float	float
    double	double
    string	char*
    Tcl_Obj*	Tcl_Obj*
    Tcl_Obj	Tcl_Obj*
}

array set IMap {
    char	char
    uchar	uchar
    short	short
    ushort	ushort
    int		int
    uint	uint
    long	long
    ulong	ulong
    float	float
    double	double
    string	string
    Tcl_Obj*	Tcl_Obj
    Tcl_Obj	Tcl_Obj
}

foreach type $Types {
    critcl::cproc sizeof_$type {} int [subst { return sizeof($::TMap($type)); }]
}

foreach type $Types {
    set type [regsub {[^_a-zA-Z]} $type {}]

    critcl::cproc sizeof_${type}_struct {} int [subst { 

	typedef struct _${type}_struct {
	    $::TMap($type)	value;
	} ${type}_struct;

	return sizeof(${type}_struct);
    }]
}

proc structN { n } {
    foreach types [combi $::Types $n list] {
	critcl::cproc sizeof_[join $types _] {} int [subst { 
	    typedef struct _struct_[join [map t $types { I $::IMap($t) }] _] { [subst {
		 [: { i type } [enumerate $types] { $::TMap($type) value$i; }]
	    }] }            struct_[join [map t $types { I $::IMap($t)}] _];
	    
	    return sizeof(  struct_[join [map t $types { I $::IMap($t)}] _]); 
	}]
    }
}

#structN 1
structN 2
structN 3
structN 4

