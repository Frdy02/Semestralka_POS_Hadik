# Názov výsledných programov
TARGET = game
SERVER_TARGET = server
CLIENT_TARGET = klient

# Kompilátor
CC = gcc

# Flagy pre kompilátor
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Knižnice
LDFLAGS = -lncurses

# Zdrojové súbory pre hru
SRC = main.c world.c snake.c input.c fruit.c

# Zdrojové súbory pre server
SERVER_SRC = server.c world.c snake.c input.c fruit.c

# Zdrojové súbory pre klienta
CLIENT_SRC = klient.c input.c

# Hlavíčkové súbory
HEADERS = world.h snake.h input.h fruit.h position.h server.h

# Objektové súbory pre hru
OBJ = $(SRC:.c=.o)

# Objektové súbory pre server
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Objektové súbory pre klienta
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

# Pravidlo pre vytvorenie spustiteľného súboru hry
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Pravidlo pre vytvorenie spustiteľného súboru servera
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJ) $(LDFLAGS)

# Pravidlo pre vytvorenie spustiteľného súboru klienta
$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJ) $(LDFLAGS)

# Pravidlo pre preklad každého .c súboru na .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre vyčistenie súborov
clean:
	rm -f $(OBJ) $(SERVER_OBJ) $(CLIENT_OBJ) $(TARGET) $(SERVER_TARGET) $(CLIENT_TARGET)
