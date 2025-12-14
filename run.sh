gcc -g -O0 -Wall -Wextra main.c -o out.exe -lraylib -lopengl32 -lgdi32 -lwinmm
if [ $? -ne 0 ] 
then
    echo "Failure"
else
    gdb -ex=r out.exe
fi