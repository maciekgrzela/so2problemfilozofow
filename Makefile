COMPILER=g++
FLAGS=-Wall -std=c++11 -pthread
NCURSES=-lncurses

all: compile

compile:
	$(COMPILER) $(FLAGS) main.cpp -o philosophers $(NCURSES)

clean:
	rm -f philosophers