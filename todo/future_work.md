This file is the todo list of dynStruct. This are just ideas, ways to look, some of them can be impossible to do.

# when reocrding an access, also record the base if the access is on form <base + disp + scale * index>
       this can allow to detect sub sutructure, structure inside an address space (like return of mmap)
       this can also allow to detect stack structure more easily
       Also this will not make the isntrumentation more heavier than now and everything can be do in post processing

# when -k is used change the name of the "struct" in the name by "array"
        Do the same when analysis is ran from the web_ui

#add a not research (maybe '!' at the start of the search field)
        Actually it can be difficult to find information which are different than an other (specifically in the access search).
	Doing something like: if the line start by '!' display everything which don't match the research

#check if it work correctly with multi-thread program:

	Just verify if dynStruct work properly wth multi-thread program.
	If not fix the issues.

# Handle signal to write output beofre exiting the program:
	Handle all signal which generate an exit if not catch by the program and wite the output file at that moment
	This can be useful to analyse program which take a long time to run completly (can stop after a few min and have the needed data to recover struct).

#optimization of actual implem:
	
	Actually runing a simple ls with dynStruct took ~120ms instead of 2ms or 3ms without it.  
	Such an overhead is not a surprise but it can be reduced.  
	All instrumentation are done via a clean call, maybe some of them can be done inline, this will reduce the overhead.  

	*look at the new sample in DynamoRIO doc with replacement of clean call at each intruction by inline instrumentation*
	*inline instrumentaiton instead of clean call each time*  
	*add a buffering for output, (dr_printf and dr_fprintf are not buffered)*

#optimize memory use of dynStruct.py
	Actually dynStruct.py keep every object in ram, which made impossible to analyse big output from the data gatherer.

#check the behavior of the plt part on diverse compiler (gcc work well, so test at least clang and tcc):

	Until now all test are done using GCC, using an other compiler is not supposed to be a problem but this need the same check than multi-threaded program.

#check the behavior with program using signal handler:

	Same thing than for muti-threaded program.
	*especially the state of the internal stack of function*

#Use a little bit of static analysis in recovery time to have a better context for the access
        Actually 1 additionnal instruction are recorded by the data gatherer to provide some context to the access instruciton (which is usually a MOV). Save more isntruction is not a good idea for overhead reason and increasing size of the ouput file.
	But using some static analysis in the recovering step to avoid having to record these instruction and have a better context is possible and provide very usefull information to have a good type recovering for the structure.
	This will not work with self-modifying code

#add possibility to add an other dynamo file to a serialized file (by loading both of them):

	dynStruct use a dynamic approach, so the code coverage of one run can not be enough.
	This option will allow to aggregate data extract for multiple run of the program.

	*run struct recovery only on new block*
	*be careful of stuff like unique id from the dynamo data*

#add an option to produce radare2 script which do automatique memory typing in radare2 debugger:
     	
	I usually use radare2 as decompiler and debugger (It's a big open source project).  
	Radare2 can be script to automate action during the debugging, this option will produce a script for radare2.  
	This script will load the structure recovered (with user modification) in radare2 and used address of malloc from block in the struct to automatically associated the structure (pf* commands in radare2).
	This script could add comment on allocation/deallocation and r/w access of member of struct

#pointeur detection at data gathering time:

	By looking at the addr store in block on the data gatherer it could be possible to detect pointer to structure/heap/read_only data and in case of heap pointer, store to what block (for linked-list detection for example).

	*ptr to heap data: easy just look if addr in block tree*  
	*ptr to func: easys for binary with symbol, for strip binary check if prologue at addr ? just check if addr it's in text section ?*  
	*for other data ptr: check in what section is the addr ?*  
	*if ptr to an other block remeber it (find a way to mark only 1 block, addr are not enough), used it to detect ptr to struct or ptr to array, to mark block maybe in increasing index to identify block here instead of in python (a long may be enough)*  

#inner struct recovering:
	
	Compiler generally align structure, so holes (called pad in dynStruct.py output) are detect in the middle of a struct may mean there is an inner struct.  
	I'm not sure if this is possible be cause detect the start of the inner struct will be a challenge.  
	For that you have to store the alignement use by the program in struct (and find a way tu find it).
	
	*used padding to determine inner struct ven it's not a array_struct*
	*can be done via static analyze if access (access pattern, before get root ptr of inner struct and after go to member)*


#detect array at runtime by pattern of how the data are accessed:
	Must record alignement size os the program in the data gatherer
	The goal here is to try to avoid the confusion between array and structure whith all member having the same size.  
	These access pattern can be compiler dependent and depend of the optimization level.  
	This can also add a non negligible overhead, so maybe this not a good idea but to know that I have to try.


#add a sort of time notion:
        
	The idea here is to add new feature to dynStruct, for example a timelaps which is displaying access to the mémory (can be all memory or a specific block).  
	This allow the interface to display what access was done before the access i'm looking at, and what will be the next.
	
	*not a timer (too time consuming)*  
	*a general timer ? (for all access) is a 64 bit number enough for importante program ?*  
	*a block timer + access timer per block ?*  
	*with the time notion build a sort of timelaps of memory with each access = 1 step. (for this general timer is better)*

#be more flexible with allocator func (be able to work with other func than malloc/realloc/free but with same proto). maybe a sort of config file to be able to work with any kind of allocator.:

    	add possiility to wrap multiple library
    	Someone can use it's own allocator and some language don't use malloc at all.  
	So find a way to be as flexible as possible here to not be able to analyse a program because of it's memory allocator.

#check what is the behavior with cpp program (object are allocated via malloc ?, ...): need flexibility on allocator:

       How dynStruct handle cpp program ?  
       If there is some specific detail, try to detect if the program is a c++ program or not and do the necessary stuff to handle cpp program correctly.

#add a option to dynStruct.py to load struct from a C style header instead of runing recovering
       this will add the possibility to change the structures without the graphical interface
       need to de-compact the struct before matching with block (and keep the same compact struct than in the file).
       *if use with serrialized data remove, replace existing struct who have the same name with the one on the header if size are equal.*


#check other language like rust, go, ... (I think most of them don't call malloc but have their own allocator system, mmap wrapping can help but it's not very precise): need flexibility on allocator:

       It's impossible to detect all programing language and to do specific stuff for all of them, but if only add some option (like the flexibility on the allocator) allow dynStruct to handle more program language properly do it.

#wrap mmap syscall and use new memory as a big block (not for structure recovering), mark block as mmap and don't try to recover struct. but if mmap by allocator don't save it:

       For program who deal directly with mmap.

#wrap brk syscall to handle old style allocator:

       Same reason as the last one

#add the stack with a stack frame = a block and try to do something with that:

      dynStruct look only at the heap because it's easier.  
      It's possible to look into the stack to but dynStruct will have to detect the different calling convention properly.  
      Look at the access pattern here will be very important in order to tell if it's a structure or a sequence of variable.

#port to arm/arm64:

      Just make dynStruct working on arm/arm64.
      This should be quite easy with th abstraction of DynamoRIO (except maybe for specific inline instrumentalisation).

#port to windows:

      Just make dynStruct working on windows.  
      Do all necessary specific stuff like syscall for allocate memory, parsing PE, ...

#port to mac os

      Just make dynStruct working on Mac os.  
      Do all necessary specific stuff like syscall for allocate memory, parsing MACH-O, ...

#support shared memory:

      Also look into shared memory.
