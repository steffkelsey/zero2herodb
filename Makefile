TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Seshire Ln.,120"
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -a "Kimmy G.,1882 Gerard St.,10" -l
	./$(TARGET) -f ./mynewdb.db -u "Timmy H.,130" -l
	./$(TARGET) -f ./mynewdb.db -r "Timmy H." -l
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -r "Kimmy G." -l
	./$(TARGET) -f ./mynewdb.db -r "Kimmy G." -l

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude

# To add debug flag
#obj/%.o : src/%.c	
#	gcc -c $< -o $@ -Iinclude -g

