#check if it work correctly with multi-thread program:

	Just verify if dynStruct work properly wth multi-thread program.
	If not fix the issues.

#store opcode for each acces and disass in python with capstone

       	this can be usefull for the dissertation
  	it could be very helpfull to have the instruction which did the access to understand certain behavior.
	this will also help to have better type recovery.
	
#check if write instruction handle signed number, float number, ptr, ... via the opcode stored in an access and disass with capstone

       The goal here is also to made the structure recovering more accurate by looking at some other information at runtime.

#optimization of actual implem:
	
	Actually runing a simple ls with dynStruct took ~120ms instead of 2ms or 3ms without it.  
	Such an overhead is not a surprise but it can be reduced.  
	All instrumentation are done via a clean call, maybe some of them can be done inline, this will reduce the overhead.  

	*look at the new sample in DynamoRIO doc with replacement of clean call at each intruction by inline instrumentation*
	*inline instrumentaiton instead of clean call each time*  
	*alloc for multiple access, orig, block each time to not have to call alloc every time*
	*data reorganisation (remove ptr in tree_t struct and include a tree_t member in each struct which is tored in a tree)*
	*flush free block periodically to reduce memory overhead for complexe program like emacs*

#check the behavior of the plt part on diverse compiler (gcc work well, so test at least clang and tcc):

	Until now all test are done using GCC, using an other compiler is not supposed to be a problem but this need the same check than multi-threaded program.

#check the behavior with program using signal handler:

	Same thing than for muti-threaded program.
	*especially the state of the internal stack of function*

#add possibility to add an other dynamo file to a serialized file (by loading both of them):

	dynStruct use a dynamic approach, so the code coverage of one run can not be enough.
	This option will allow to aggregate data extract for multiple run of the program.

	*run struct recovery only on new block*
	*be careful of stuff like unique id from the dynamo data*

#add an option to produce radare2 script which do automatique typing in radare2 debugger:
     	
	I usually use radare2 as decompiler and debugger (It's a big open source project).  
	Radare2 can be script to automate action during the debugging, this option will produce a script for radare2.  
	This script will load the structure recovered (with user modification) in radare2 and used address of malloc from block in the struct to automatically associated the structure (pf* commands in radare2).

#pointeur detection:
	
	The structure recovering of dynStruct is only based on the size of the different access, looking at some more information at runtime will allow dynStruct to be extract pointeur.  
	I think it's also possible to detect pointer to structure and know what is this structure.  

	*instruction which manipul addr (like ld, ...)*   
	*ptr to heap data: easy just look if addr in block tree*  
	*ptr to func: easys for binary with symbol, for strip binary check if prologue at addr ? just check if addr it's in text section ?*  
	*for other data ptr: check in what section is the addr ?*  
	*if ptr to an other block remeber it (find a way to mark only 1 block, addr are not enough), used it to detect ptr to struct or ptr to array, to mark block maybe in increasing index to identify block here instead of in python (a long may be enough)*  

#struct recovering by padding:
	
	Compiler generally align structure, so holes (called pad in dynStruct.py output) are detect in the middle of a struct may mean there is an inner struct.  
	I'm not sure if this is possible be cause detect the start of the inner struct will be a challenge.  

	*used padding to determine inner struct ven it's not a array_struct*

#detect array at runtime by pattern of how the data are accessed:
	
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
       *if use with serrialized data remove, replace existing struct who have the same name with the one on the header if size are equal.*

#check other language like rust, go, ... (I think most of them don't call malloc but have their own allocator system, mmap wrapping can help but it's not very precise): need flexibility on allocator:

       It's impossible to detect all programing language and to do specific stuff for all of them, but if only add some option (like the flexibility on the allocator) allow dynStruct to handle more program language properly do it.

#search other usage of the data extract:

       This one is just if some new idea to use the data extract pop in my mind.

#wrap mmap syscall and use new memory as a big block (not for structure recovering), mark block as mmap and don't try to recover struct. but if mmap by allocator don't save it:

       For program who deal directly with mmap.

#wrap brk syscall to handle old fashion allocator:

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
