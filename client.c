/* Abdou lahat Sylla 12011836
Je déclare qu'il s'agit de mon propre travail. */


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "mots_5_lettres.h"
void saisir_prop(char *prop);
void chaine_toupper(char *ch);
void vider_tampon();

typedef struct wordl_p {
	char prop[32];
	int sock;
	int carac_trouve;
}wordlp;

 int main(int argc, char const *argv[]) {
   if (argc < 2){
     printf("pas assez d'argument. Usage: %s addr_ipv4 (ex: 127.0.0.1)\n",argv[0] );
     exit(1);
   }
   int sock = socket(AF_INET,SOCK_STREAM,0);

   if(sock<0){
     perror("sock");
     return 1;
   }


   struct sockaddr_in sa = { .sin_family = AF_INET,
   .sin_port = htons(4242) };

   if (inet_pton(AF_INET, argv[1], &sa.sin_addr) != 1) {
   fprintf(stderr, "adresse ipv4 non valable\n");
   exit(1);
   }

   if (connect(sock,(struct sockaddr * )&sa,sizeof(sa)) < 0) {
     perror("connect");
     exit(1);
   }

   char proposition[6];
   wordlp  * wordle = malloc(sizeof(wordle));
   wordle->sock = sock;
   do {
     saisir_prop(proposition);
     /*envoie la propositionau  serveur*/
     send(sock,proposition,strlen(proposition)+1,0);
     /* attend de recevoir la structure wordle que lui renvoie le serveur APRES
      traitement de proposition */
     recv(sock, wordle, sizeof(wordle),0);

     printf("%s\n",wordle->prop );
     /*là il attend juste confirmation sur le nombre de caractere trouve par le serveur
     pour savoir s il doit arreter ou pas*/
     recv(sock,&wordle->carac_trouve,sizeof(int),0);
   } while(wordle->carac_trouve < 5 );



  /*fin du jeu il recoit le dernier message du serveur qui lui dit entre autres
  apres combien de tentatives il a reussi */
    char msg[256];
    recv(sock,msg,100,0);
    printf("%s\n",msg );

   close(sock);
   return 0;
 }

 void saisir_prop(char *prop)
 {
 	int correct = 0;
 	while (!correct) {
 		printf("Votre proposition : ");
 		if (scanf("%5s", prop) == EOF) {
 			printf("\n");
 			exit(1);
 		}
 		if (strlen(prop) < 5) {
 			fprintf(stderr, "Mot trop court, entrer un mot de 5 lettres.\n");
 		} else {
 			chaine_toupper(prop);
 			if (!est_dans_liste_mots(prop))
 				fprintf(stderr, "Ce mot n'est pas dans la liste de mots.\n");
 			else
 				correct = 1;
 		}
 		vider_tampon();
 	}
 }

 void chaine_toupper(char *ch)
 {
 	int i;
 	for (i = 0; ch[i] != '\0'; i = i + 1)
 		ch[i] = toupper(ch[i]);
 }

 void vider_tampon()
 {
 	int c;
 	while ((c = getchar()) != '\n' && c != EOF)
 		;
 }
