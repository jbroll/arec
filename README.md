arec
====

An array of records structure for Tcl.

-----

This is a C extension to allow simple access to arrays of C data structures.  I have used it for two use
cases.

The first is where you have a few dozen sets of parameters that are used in a complex computation.
It's very nice to use Tcl to organize and create the parameter sets and then drive the computations from the script.
The computation will be written in C, likely calling additional C libraries.
If you are using Tcl arrays or dicts to hold the parameters, they must be unpacked in to C before getting to the real code.
Although this is relativly simple and the Tcl API is well documented, it is tedious and error prone.
With ARec, the data are stored in memory direclly corrosponding to C data structures and can be directly accessed from the 
C code.

The second use case is when there is a very large number of structures to be passed among several parts of the computation.
Tcl is used as the high level glue controling the flow of how the computation proceeds.

In my current optical raytracing project I'm using ARec in both of these capacities.  The optical surfaces that the rays 
will traverse each have between a dozem and a hundred parameters.  Tens of thousands of rays are traced and then the 
resulting ray posiitons can be analysed, binned into images or saved into output files as directed by the Tcl script.

-----

The extension is written using crictl to create a Tcl lodable package. 

-----




