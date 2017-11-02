CC=gcc
CFLAGS=-Wall -g -lm
LDLIBS=-lpthread

all: test_lecteurs_redacteurs
	@echo "\033[91m Vous pouvez afficher le contenu de la fifo avec la cible make debug \033[0m"

debug: test_lecteurs_redacteurs
	$(CC) $(CFLAGS) $(LDLIBS) -DDEBUG test_lecteurs_redacteurs.c
clean:
	$(RM) test_lecteurs_redacteurs *.o
