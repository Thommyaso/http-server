CC      = cc
CFLAGS  = -g
SRC     = src/main.c src/connection.c src/listener.c src/parser.c src/response.c src/router.c src/utils/buffer.c
OBJ     = $(SRC:.c=.o)
TARGET  = server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
