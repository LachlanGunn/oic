CC 		=   gcc
CFLAGS	=	-Wall -Werror -ansi -pedantic
CXX 		=   g++

EXE		=   scpitest
SRCS	=	main.c scpiparser.cpp
HDRS	=	scpiparser.h
OBJS_1	=	${SRCS:.c=.o}
OBJS	=	${OBJS_1:.cpp=.o}

.SUFFIXES:

.SUFFIXES: .o .c .cpp

.PHONY: clean

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) -c $<
	
all:	$(EXE)

$(EXE):	$(OBJS)
	$(CXX) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(EXE)
