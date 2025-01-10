# Kompilátor a flagy
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread

# Súbory
SRCS_SERVER = server.c world.c snake.c fruit.c input.c common.c gamestate.c
SRCS_CLIENT = client.c world.c snake.c common.c gamestate.c fruit.c input.c
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
TARGET_SERVER = server
TARGET_CLIENT = client

# Ciele
all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(TARGET_SERVER) $(TARGET_CLIENT)

