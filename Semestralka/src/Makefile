# Názov výsledných programov
TARGET = game
SERVER_TARGET = server
KLIENT_TARGET = klient

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
KLIENT_SRC = klient.c #input.c

# Hlavíčkové súbory
HEADERS = world.h snake.h input.h fruit.h position.h server.h

# Objektové súbory pre hru
OBJ = $(SRC:.c=.o)

# Objektové súbory pre server
SERVER_OBJ = $(SERVER_SRC:.c=.o)

# Objektové súbory pre klienta
KLIENT_OBJ = $(KLIENT_SRC:.c=.o)

# Pravidlo pre vytvorenie spustiteľného súboru hry
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Pravidlo pre vytvorenie spustiteľného súboru servera
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJ) $(LDFLAGS)

# Pravidlo pre vytvorenie spustiteľného súboru klienta
$(KLIENT_TARGET): $(KLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(KLIENT_TARGET) $(KLIENT_OBJ) $(LDFLAGS)

# Pravidlo pre preklad každého .c súboru na .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Pravidlo pre vyčistenie súborov
clean:
	rm -f $(OBJ) $(SERVER_OBJ) $(KLIENT_OBJ) $(TARGET) $(SERVER_TARGET) $(KLIENT_TARGET)
