NAME=spherevr
CC=gcc
CFLAGS=$(shell pkg-config --cflags sdl gl glu opencv) -Wall
LIBS=$(shell pkg-config --libs sdl gl glu opencv) -lm
LDFLAGS=
OBJ=$(NAME).o vuzix.o

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) $(LIBS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f $(NAME) $(OBJ)
