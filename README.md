arec
====

An array of records structure for Tcl.

-----

This is a C extension to allow simple access to arrays of C data structures.  I have used it for two use
cases.

The first is where you have a few dozen sets of parameters that are used in a complex computation.
It's very nice to use Tcl to organize and create the parameter sets and then drive the computations from the script.
The computation will be written in C, likely calling additional C libraries.
If you are using Tcl arrays or dicts to hold the parameters, they must be unpacked before getting to the real code.
Although this is relativly simple and the Tcl API is well documented, it is tedious and error prone.
With ARec, the data are stored in memory directly corrosponding to C data structures and can be directly accessed from the 
C code.

The second use case is when there is a very large number of structures to be passed among several parts of the computation.
Tcl is used as the high level glue controling the flow of how the computation proceeds.

In my current optical raytracing project I'm using ARec in both of these capacities.  The optical surfaces that the rays 
will traverse each have between a dozem and a hundred parameters.  Tens of thousands of rays are traced and then the 
resulting ray posiitons can be analysed, binned into images or saved into output files as directed by the Tcl script.

-----

This is the first release and should be considered rather alpha.  There is a tcl test suite, but it checks the "working" cases 
and it is known that error handleing and reporting is minimal.  In code like this that plays with pointers and memory layout, these
errors are likely fatal.  Play nice.  If you report errors I'll try to fix things ASAP.

The extension is written using crictl to create a Tcl loadable package. 

-----

Simple Docuementation

The package defines one command arec::typedef.  This command is used to create new data type commands that themselves can  be used to
create data object commands.  The data object commands are handles to arrays of C data structures.  They can be used to get and set
indivudual values in the elements of the array.  The general form of the access syntax is :

    _obj-command_ \[start \[end]] name value

Example:

Create a new data type "Thing" with 4 members, x, y, nfobs and placing.  Then create an array of 10 Things accessed via the
"things" command.  Set the first elements x member to the value 10.

    arec::typedef struct Thing {
	double	x
	double	y
	int	nfobs
	char*	placeing
    }

    Thing things 10

    things 0 set x 10

Two indicies may be given to operate on a range of array elements and if the index arguments are omitted, all array elements are implied.

To set all the element x members to the value 10 use:

    things set x 10

To set elements 1, 2, and 3 x members to the value 10 use:

    things 1 3 set x 10









