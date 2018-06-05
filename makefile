CC = gcc
CFLAGS = -Wall -Wextra
OBJ = notebook.c

notebook: $(OBJ)
	$(CC) -o notebook $(OBJ) $(CFLAGS)

clean:
	rm notebook