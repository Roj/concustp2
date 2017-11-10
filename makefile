CFLAGS := -g
VFLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes
ARCHIVOS = log.o
PROGRAMA = main

all: clean $(PROGRAMA)

$(PROGRAMA): $(ARCHIVOS) $(PROGRAMA).o
	gcc -o $(PROGRAMA) $^

run: clean $(PROGRAMA)
	./$(PROGRAMA)

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ 

clean:
	rm -f $(PROGRAMA) *.o
	touch x.log
	rm *.log
	touch makefile~
	rm *~

valgrind: clean $(PROGRAMA)
	valgrind $(VFLAGS) ./$(PROGRAMA)

.PHONY: clean


