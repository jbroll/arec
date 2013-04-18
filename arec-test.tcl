#!/usr/bin/env tclkit8.6
#

package require tcltest

lappend auto_path lib ../lib

package require arec

cd [file dirname [file normalize [info script]]]/test

::tcltest::configure -testdir [file dirname [file normalize [info script]]] -singleproc 1

::tcltest::configure {*}$argv
::tcltest::runAllTests

