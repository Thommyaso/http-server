CC      = cc
CFLAGS  = -g
SRC     = src/main.c src/connection.c src/listener.c src/parser.c src/response.c src/router.c src/utils/buffer.c
OBJDIR  = build
OBJ     = $(SRC:src/%.c=$(OBJDIR)/%.o)

TARGET  = server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)
