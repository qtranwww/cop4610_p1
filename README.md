make clean to clean

make run to run

Expected output of `make run`:

```text
gcc -g -Wall -std=c99 -Iinclude/ -c src/execution.c -o obj/execution.o
gcc -g -Wall -std=c99 -Iinclude/ -c src/lexer.c -o obj/lexer.o
gcc -g -Wall -std=c99 -Iinclude/ -c src/path.c -o obj/path.o
gcc -g -Wall -std=c99 -Iinclude/ -c src/redirection.c -o obj/redirection.o
gcc -g -Wall -std=c99 -Iinclude/ obj/execution.o obj/lexer.o obj/path.o obj/redirection.o -o bin/shell
bin/shell
user@linprog.cs.fsu.edu:/home/majors/user/path>
```
