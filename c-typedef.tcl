# Make some procs to allow the evaluation of C typedefs
#
namespace eval arec {
    proc ::typedef { struct stype body type } {
	arec::typedef $struct $stype [regsub -all {\[} $body { [arec::c-expr }] $type 
    }
    proc c-expr { args } {
	set args [regsub {([a-zA-Z_][a-zA-Z_0-9]*)} $args {$::\1}]
	    ::expr $args
    }
    proc #define { name value } {
	set ::$name $value
    }

    proc ::/* { args } {}

    proc c-source { file } {
        set data [regsub -all -line {^#define } [cat $file] {arec::#define }]
        eval $data
    }
}
