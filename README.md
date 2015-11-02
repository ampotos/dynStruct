# dynStruct
dynStruct is a tool using dynamoRio to monitor memory access of an ELF binary via a data gatherer,
and use this data to recover structures of the original code.

The data gathered can also be used to quickly find where and by which function a menber of a structure is write or read.

Today only the data gatherer is available, the recovering of structures and a webui will come.

## Requirements
* CMake >= 2.8
* [DynamoRIO](https://github.com/DynamoRIO/dynamorio)

## Setup
Set the environment variable DYNAMORIO_HOME to the absolute path of your
DynamoRIO installation

execute `build.sh`

to compile dynStruct for a 32bits target on a 64bits os execute `build.sh 32`

## Data gatherer

###Usage

```
drrun -c <dynStruct_path> <dynStruct_args> -- <prog_path> <prog_args>

  -h print this help
  -o <file_name>	set output file name for json (default: <prog_name>.ds_out)
   			print output on console
  -w <module_name>	wrap <module_name>
			 dynStruct record memory blocks only
			 if *alloc is called from this module
  -m <module_name>	monitor <module_name>
			 dynStruct record memory access only if
			 they are done by a monitore module
  -a <module_name>	is used to tell dynStruct which module implements
			 allocs functions (malloc, calloc, realloc and free)
			 this has to be used with the -w option (ex : "-a ld -w ld")
			 this option can only be used one time
for -w, -a and -m options modules names are matched like <module_name>*
this allow to don't care about the version of a library
-m libc.so match with all libc verison

The main module is always monitored and wrapped
Tha libc allocs functions are always used (regardless the use of the -a option)

Example : drrun -c dynStruct -m libc.so - -- ls -l

This command run "ls -l" and will only look at block allocated by the program
but will monitor and record memory access from the program and the libc
and print the result on the console
```

### Example

We are going to analyse this little program.

```C
void print(char *str)
{
  puts(str);
}

int main()
{
  char *str;

  str = malloc(5);
  strcpy(str, "test");
  str[4] = 0;
  print(str);

  free(str);
}
```
Which after compilation look like this
![Example disassembly](http://i.imgur.com/L2i4zJS.png)

If we run `drrun -c  dynStruct - -- tests/example` we get
```
test
tast
block : 0x0000000000603010-0x0000000000603015(0x5) was free
alloc by 0x0000000000400617(main : 0x00000000004005f9 in example) and free by 0x000000000040064c(main : 0x00000000004005f9 in example)
	 WRITE :
	 was access at offset 1 (1 times)
	details :
			 1 bytes were accessed by 0x00000000004005e7 (print : 0x00000000004005b6 in example) 1 times
	 was access at offset 4 (2 times)
	details :
			 1 bytes were accessed by 0x000000000040062a (main : 0x00000000004005f9 in example) 1 times
			 1 bytes were accessed by 0x0000000000400636 (main : 0x00000000004005f9 in example) 1 times
	 was access at offset 0 (1 times)
	details :
			 4 bytes were accessed by 0x0000000000400624 (main : 0x00000000004005f9 in example) 1 times
```
We see all the right access on str done by the program himself.
We can notice the 4 bytes access at offset 0 of the block due to gcc optimisation for initializing the string.

Now if we run `drrun -c  dynStruct -m libc - -- tests/example` we are going to monitor all the libc access, and we get
```
test
tast
block : 0x0000000000603010-0x0000000000603015(0x5) was free
alloc by 0x0000000000400617(main : 0x00000000004005f9 in example) and free by 0x000000000040064c(main : 0x00000000004005f9 in example)
	 READ :
	 was access at offset 1 (2 times)
	details :
			 1 bytes were accessed by 0x00007fca292156c2 (_IO_default_xsputn : 0x00007fca29215650 in libc.so.6) 1 times
			 1 bytes were accessed by 0x00007fca29213c9d (_IO_file_xsputn@@GLIBC_2.2.5 : 0x00007fca29213b70 in libc.so.6) 1 times
	 was access at offset 2 (2 times)
	details :
			 1 bytes were accessed by 0x00007fca292156c2 (_IO_default_xsputn : 0x00007fca29215650 in libc.so.6) 1 times
			 1 bytes were accessed by 0x00007fca29213c9d (_IO_file_xsputn@@GLIBC_2.2.5 : 0x00007fca29213b70 in libc.so.6) 1 times
	 was access at offset 3 (2 times)
	details :
			 1 bytes were accessed by 0x00007fca292156c2 (_IO_default_xsputn : 0x00007fca29215650 in libc.so.6) 1 times
			 1 bytes were accessed by 0x00007fca29213c81 (_IO_file_xsputn@@GLIBC_2.2.5 : 0x00007fca29213b70 in libc.so.6) 1 times
	 was access at offset 0 (5 times)
	details :
			 1 bytes were accessed by 0x00007fca292156c2 (_IO_default_xsputn : 0x00007fca29215650 in libc.so.6) 1 times
			 16 bytes were accessed by 0x00007fca292210ca (strlen : 0x00007fca292210a0 in libc.so.6) 2 times
			 4 bytes were accessed by 0x00007fca29224c2e (__mempcpy_sse2 : 0x00007fca29224c00 in libc.so.6) 1 times
			 1 bytes were accessed by 0x00007fca29213c9d (_IO_file_xsputn@@GLIBC_2.2.5 : 0x00007fca29213b70 in libc.so.6) 1 times
	 WRITE :
	 was access at offset 1 (1 times)
	details :
			 1 bytes were accessed by 0x00000000004005e7 (print : 0x00000000004005b6 in example) 1 times
	 was access at offset 4 (2 times)
	details :
			 1 bytes were accessed by 0x000000000040062a (main : 0x00000000004005f9 in example) 1 times
			 1 bytes were accessed by 0x0000000000400636 (main : 0x00000000004005f9 in example) 1 times
	 was access at offset 0 (1 times)
	details :
			 4 bytes were accessed by 0x0000000000400624 (main : 0x00000000004005f9 in example) 1 times

```
Now all the read access done by the libc are listed.
