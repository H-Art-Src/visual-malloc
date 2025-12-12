gcc -g -O0 -Wall -Wextra main.c -o out.exe -lraylib -lopengl32 -lgdi32 -lwinmm
gdb -ex=r out.exe