CC 		=   gcc
CFLAGS	=	-Wall -Werror -ansi -pedantic

EXE		=   scpitest
SRCS	=	main.c scpiparser.c
HDRS	=	scpiparser.h
OBJS	=	${SRCS:.c=.o}

.SUFFIXES:

.SUFFIXES: .o .c

.c.o: $(HDRS)
	$(CC) $(CFLAGS) -c $<
	
all:	$(EXE)

$(EXE):	$(OBJS)
	$(CC) -o $@ $(OBJS)
