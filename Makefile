NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRCS = main.c parse.c time.c log.c coder.c dongle.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re