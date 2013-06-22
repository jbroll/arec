#!/usr/bin/env tclkit8.6
#

#parray tcl_platform

package require tcltest

lappend auto_path lib ../lib

package require arec
package require arec-struct

cd [file dirname [file normalize [info script]]]/test

::tcltest::configure -testdir [file dirname [file normalize [info script]]] -singleproc yes

::tcltest::configure {*}$argv
::tcltest::runAllTests

