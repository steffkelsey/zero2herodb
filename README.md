# Project for Zero2Hero C Programming @ Low Level Academy

## To Debug

Change the Makefile to compile with debugging information.  
```Makefile
# To add debug flag
obj/%.o : src/%.c	
	gcc -c $< -o $@ -Iinclude -g
```

Check for memory leaks with valgrind.  
```bash
# On open file
valgrind --leak-check=full ./bin/dbview -f ./mynewdb.db
# On list employees
valgrind --leak-check=full ./bin/dbview -f ./mynewdb.db -l
# On add employee
valgrind --leak-check=full ./bin/dbview -f ./mynewdb.db -a "Kimmy G., 1 Main St.,15" -l
# On update employee
valgrind --leak-check=full ./bin/dbview -f ./mynewdb.db -u "Kimmy G.,100" -l
# On remove employee
valgrind --leak-check=full ./bin/dbview -f ./mynewdb.db -r "Kimmy G." -l
```

Step through the program using gdb (with args).  
```bash
# Remove an employee named 'Jimmy F.'
gdb --args ./bin/dbview -f ./mynewdb.db -r "Jimmy F."
```

Once in GDB, switch to the TUI with `tui enable`.  
Cycle thgrough TUI layouts using `layout next`.  
Set breakpoints with `break main`.  
Run the program with `run`.  
Go to next line in src with `next`.  
Step into a line in src with `step`.  
See local vars with `info locals`.  
GDB cheat sheets [here](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf) 
and [here](https://cs.brown.edu/courses/cs033/docs/guides/gdb.pdf).
