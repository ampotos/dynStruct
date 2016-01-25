1. #web_ui (i'm actually working on it):
	Actually the tool is not usable by anyone except me, and even I can't find a information quickly, the goal of the web_ui is to provide an easy to use interface which will provide the information quickly.  
	Have an interface is mandatory for dynStruct because a want dynStruct to be usable for anybody.

2. #just after the web ui is finish start using dynStruct in real ctf chall to find out more idea for enhancement (need the web_ui):
     	
	When dynStruct will be usable I will ask a few friend to try it and give me their feelback (and I will use it as well).  
	It's important to do this just after the interface because is a big changement has to be done, it will be easier to do it now thant in a few month.  

3. #check if it work correctly with multi-thread program:
        
	Just verify if dynStruct work properly wth multi-thread program.  
      	If not fix the issues.

4. #check what is the behavior with forking program:
        
	Same thing than for muti-threaded program.

5. #check the behavior of the plt part on diverse compiler (gcc work well, so test at least clang and tcc):
        
	Until now all test are done using GCC, using an other compiler is not supposed to be a problem but this need the same check than multi-threaded program.       

6. #add possibility to add an other dynamo file to a serialized file (by loading both of them): 
        
	dynStruct use a dynamic approach, so the code coverage of one run can not be enough.  
	This option will allow to aggregate data extract for multiple run of the program.  

	*run struct recovery only on new block*  
	*be careful of stuff like unique id from the dynamo data*  

7. #add an option to produce radare2 script which do automatique typing in radare2 debugger:
     	
	I usually use radare2 as decompiler and debugger (It's a big open source project).  
	Radare2 can be script to automate action during the debugging, this option will produce a script for radare2.  
	This script will load the structure recovered (with user modification) in radare2 and used address of malloc from block in the struct to automatically associated the structure (pf* commands in radare2).

8. #optimization of actual implem:
	
	Actually runing a simple ls with dynStruct took ~120ms instead of 2ms or 3ms without it.  
	Such an overhead is not a surprise but it can be reduced.  
	All instrumentation are done via a clean call, maybe some of them can be done inline, this will reduce the overhead.  

	*inline instrumentaiton instead of clean call each time*  
	*alloc for multiple access, orig, block each time to not have to call alloc every time*

9. #pointeur detection:
	
	The structure recovering of dynStruct is only based on the size of the different access, looking at some more information at runtime will allow dynStruct to be extract pointeur.  
	I think it's also possible to detect pointer to structure and know what is this structure.  

	*instruction which manipul addr (like ld, ...)*   
	*ptr to heap data: easy just look if addr in block tree*  
	*ptr to func: easys for binary with symbol, for strip binary check if prologue at addr ? just check if addr it's in text section ?*  
	*for other data ptr: check in what section is the addr ?*  
	*if ptr to an other block remeber it (find a way to mark only 1 block, addr are not enough), used it to detect ptr to struct or ptr to array, to mark block maybe in increasing index to identify block here instead of in python (a long may be enough)*  

10. #check if write instruction handle signed number, float number, ptr, ... and store this in a flag in the orig struct. Use this to have more accurate type on structure recovering:
        
	The goal here is also to made the structure recovering more accurate by looking at some other information at runtime.

11. #struct recovering by padding:
	
	Compiler generally align structure, so holes (called pad in dynStruct.py output) are detect in the middle of a struct may mean there is an inner struct.  
	I'm not sure if this is possible be cause detect the start of the inner struct will be a challenge.  

	*used padding to determine inner struct ven it's not a array_struct*

12. #detect array at runtime by pattern of who the data are accessed:
	
	The goal here is to try to avoid the confusion between array and structure whith all member having the same size.  
	These access pattern can be compiler dependent and depend of the optimization level.  
	This can also add a non negligible overhead, so maybe this not a good idea but to know that I have to try.

13. #look at pattern access of the memory to mark access from equal size one after the other in all the block (an access pattern like that may be done by func like memset):
      	
	This is to reduce the erreur of the recovering of structure.  
	May not be usefull with the previous improvement of the structure recovering.

14. #add a sort of time notion:
        
	The idea here is to add new feature to dynStruct, for example a timelaps which is displaying access to the mémory (can be all memory or a specific block).  
	This allow the interface to display what access was done before the access i'm looking at, and what will be the next.
	
	*not a timer (too time consuming)*  
	*a general timer ? (for all access) is a 64 bit number enough for importante program ?*  
	*a block timer + access timer per block ?*  
	*with the time notion build a sort of timelaps of memory with each access = 1 step. (for this general timer is better)*

15. #be more flexible with allocator func (be able to work with other func than malloc/realloc/free but with same proto). maybe a sort of config file to be able to work with any kind of allocator.:

    	Someone can use it's own allocator and some language don't use malloc at all.  
	So find a way to be as flexible as possible here to not be able to analyse a program because of it's memory allocator.

16. #check what is the behavior with cpp program (object are allocated via malloc ?, ...): need flexibility on allocator:

       How dynStruct handle cpp program ?  
       If there is some specific detail, try to detect if the program is a c++ program or not and do the necessary stuff to handle cpp program correctly.

17. #check other language like rust, go, ... (I think most of them don't call malloc but have their own allocator system, mmap wrapping can help but it's not very precise): need flexibility on allocator:

       It's impossible to detect all programing language and to do specific stuff for all of them, but if only add some option (like the flexibility on the allocator) allow dynStruct to handle more program language properly do it.

18. #search other usage of the data extract:

       This one is just if some new idea to use the data extract pop in my mind.

19. #wrap mmap syscall and use new memory as a big block (not for structure recovering), mark block as mmap and don't try to recover struct. but if mmap by allocator don't save it:

       For program who deal direct with mmap.

20. #wrap brk syscall to handle old fashion allocator:

       Same reason as the last one

21. #add the stack with a stack frame = a block and try to do something with that:

      dynStruct look only at the heap because it's easier.  
      It's possible to look into the stack to but dynStruct will have to detect the different calling convention properly.  
      Look at the access pattern here will be very important in order to tell if it's a structure or a sequence of variable.

22. #port to arm/arm64:

      Just make dynStruct working on arm/arm64.
      This should be quite easy with th abstraction of DynamoRIO (except maybe for specific inline instrumentalisation).

23. #port to windows:

      Just make dynStruct working on windows.  
      Do all necessary specific stuff like syscall for allocate memory, parsing PE, ...

24. #port to mac os

      Just make dynStruct working on Mac os.  
      Do all necessary specific stuff like syscall for allocate memory, parsing MACH-O, ...

25. #support shared memory:

      Also look into shared memory.
