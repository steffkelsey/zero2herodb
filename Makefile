TARGET = bin/threadpool
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) 

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*

$(TARGET): $(OBJ)
	gcc -o $@ $? -pthread

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinc

# To add debug flag
#obj/%.o : src/%.c	
#	gcc -c $< -o $@ -Iinclude -g

