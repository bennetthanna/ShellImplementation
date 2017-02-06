all: my_shell.cpp
	g++ -Wall -o my_shell my_shell.cpp

clean:
	$(RM) my_shell
