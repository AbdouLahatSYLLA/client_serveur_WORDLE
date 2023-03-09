 
all: serveur client

mots_5_lettres.o: mots_5_lettres.c mots_5_lettres.h
	gcc -c mots_5_lettres.c
serveur.o: serveur.c mots_5_lettres.h
	gcc -c serveur.c
client.o: client.c mots_5_lettres.h
	gcc -c client.c
serveur: serveur.o mots_5_lettres.o
	gcc -o serveur serveur.o mots_5_lettres.o
client: client.o  mots_5_lettres.o
	gcc -o client client.o  mots_5_lettres.o

clean:
	rm  *.o
