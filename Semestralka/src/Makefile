TARGET = server

# Kompilátor
CC = gcc

# Flagy pre kompilátor
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Zdrojové súbory
SRC = main.c world.c snake.c input.c fruit.c klient.c server.c

# Závislosti (hlavičkové súbory)
HEADERS = world.h snake.h fruit.h input.h position.h

# Objektové súbory
OBJ = $(SRC:.c=.o)

# Pravidlo pre vytvorenie spustiteľného súboru
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Pravidlo pre preklad každého .c súboru na .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre vyčistenie súborov
clean:
	rm -f $(OBJ) $(TARGET)
