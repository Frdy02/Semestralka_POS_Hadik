# Názvy výsledných programov
SERVER_TARGET = server_t
CLIENT_TARGET = klient_t
 
# Kompilátor
CC = gcc
 
# Flagy pre kompilátor
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g -D_GNU_SOURCE
 
# Knižnice
LDFLAGS = -lncurses -lpthread
 
# Zdrojové súbory pre server
SERVER_SRC = server_t.c world.c snake.c input.c fruit.c
 
# Zdrojové súbory pre klienta
CLIENT_SRC = klient_t.c input.c
 
# Hlavičkové súbory
HEADERS = world.h snake.h input.h fruit.h position.h server.h
 
# Objektové súbory pre server
SERVER_OBJ = $(SERVER_SRC:.c=.o)
 
# Objektové súbory pre klienta
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
 
# Pravidlo pre vytvorenie všetkých cieľov
all: $(SERVER_TARGET) $(CLIENT_TARGET)
 
# Pravidlo pre vytvorenie spustiteľného súboru servera
server: $(SERVER_TARGET)
 
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJ) $(LDFLAGS)
 
# Pravidlo pre vytvorenie spustiteľného súboru klienta
klient: $(CLIENT_TARGET)
 
$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJ) $(LDFLAGS)
 
# Pravidlo pre preklad každého .c súboru na .o
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
 
# Pravidlo pre vyčistenie súborov
clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_TARGET) $(CLIENT_TARGET)
