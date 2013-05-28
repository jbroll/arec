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
C code.  No parsing necessary.

The second use case is when there is a very large number of structures to be passed among several parts of the computation.
Tcl is used as the high level glue controling the flow of how the computation proceeds.

In my current optical raytracing project I'm using ARec in both of these capacities.  The optical surfaces that the rays 
will traverse each have between a dozen and a hundred parameters.  Tens of thousands of rays are traced and then the 
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

    _obj-command_ \[start \[end]] set name value

or 

    _obj-command_ \[start \[end]] get name

Example:

Create a new data type "Thing" with 4 members, x, y, nfobs and placing.  Then create an array of 10 Things accessed via the
"things" command.  Set the first elements x member to the value 10.

    arec::typedef struct Thing {
	double	x
	double	y
	int	nfobs
	char*	placeing
    }

    Thing create things 10

    things 0 set x 10

Two indicies may be given to operate on a range of array elements and if the index arguments are omitted, all array elements are implied.  The usual Tcl
convention regarding "end" and arithmatic on "end" is implimented.

To set all the element x members to the value 10 use:

    things set x 10

To set elements 1, 2, and 3 x members to the value 10 use:

    things 1 3 set x 10

--------

Two initial arrays of records are provided that describe the atomic types available.  _arec::types_ is an array of records  data structure describing
the avaible type and thier attributes.  Initially this table containes the default types and can be queired to list them:

	% puts [join [arec::types get name] \n]
	char uchar short ushort int uint long ulong float double string Tcl_Obj*

This array is itself of type _arec::type_ and contains these fields describing the attributes of the data types:

	name size align stype nfield afield fields set get shadow inst

After the user creates a new data type with the arec::typedef command it will appear in the arec::types array of records.

-------

User defined data types may them selves be used in datatype definitions.  After the above "Thing" type is defined an additional type may 
contain an array of "Thing"s:

    arec::typedef struct Fluff {
	int	mode
	Thing	leftside 2
	Thing	riteside 2
    }

    Fluff create fluffs 3

    puts [fluffs 1 riteside get]

This creats a new data type "Fluff" which contains a mode and two arrays of two things each.  Then an array of 3 "Fluff" records is created refered
to by the handle "fluffs". 

	








