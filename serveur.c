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
#include <pthread.h>


#include "mots_5_lettres.h"

#define LOG_DFT "wrdlp_srv.log"

typedef struct wordl_p {
	char prop[32];
	int sock;
	int carac_trouve;
}wordlp;

struct machin {
	wordlp * w;
	char * a_deviner;
};
/* Met toutes les lettres minuscules présentes dans ch en majuscule (les autres
 * caractères sont inchangés).
 * Précondition : ch contient un caractère nul qui sert de délimiteur de fin */
void chaine_toupper(char *ch);

/* Vider le tampon de l'entrée standard */
void vider_tampon();

void * worker (void * arg);

/* Met dans la chaine prop (tableau d'au moins 6 char) un mot de 5 lettres saisi
 * par l'utilisateur, si besoin mis en majuscule, et terminé par un '\0'.
 * Redemande la saisie tant que
 * - le mot de l'utilisateur a moins de 5 lettres ou
 * - n'est pas dans la liste de mots.
 * Si l'utilisateur saisit un mot de plus de 5 lettres, seules les 5 premières
 * sont prises en compte.
 */


/* Pour chaque lettre de prop_joueur, affiche :
 * * cette lettre en majuscule si elle figure à la même position dans a_deviner
 * * cette lettre en minuscule si elle fait partie de a_deviner mais à une autre
 *   position
 * * le caractère _ sinon
 * L'affichage se termine par un saut de ligne.
 * Retourne le nombre de lettres trouvées à la bonne position
 * Préconditions : prop_joueur et a_deviner contiennent au moins 5 caractères
 *                 qui sont tous des lettres majuscules */
int traiter_prop(const char *prop_joueur, const char *a_deviner, wordlp * wordle);

int main(int argc,char **argv)
{

		/*  Ouverture du fichier de logs */
	int log = open(argc >= 2 ? argv[1] : LOG_DFT, O_WRONLY | O_APPEND | O_CREAT, 0644);
	if (log < 0) {
	perror("open");
	exit(2);
	}
		/* Création d'un mutex pour l'écriture dans le fichier de logs */
	pthread_mutex_t mut_logfile;
	pthread_mutex_init(&mut_logfile, NULL);

	/*creer la socket */
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock<0){
		perror("sock");
		exit(2);
	}

	struct sockaddr_in sa = {
		.sin_family = AF_INET,
		.sin_port =htons(4242),
		.sin_addr.s_addr = htonl(INADDR_ANY)
	};

		/* Optionnel : faire en sorte de pouvoir réutiliser l'adresse sans
	* attendre après la fin du serveur. */
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	/*lier la socket à l addresse sa avec comme port 4242 */
	if (bind( sock,(struct sockaddr * ) &sa,sizeof(sa)) < 0) {
		perror("bind");
		exit(2);
	}

	int nb_tentatives = 0;
	char prop_joueur[6], a_deviner[6];
	pthread_t th;


	/*là on boucle pour que le serveur soit toujour actif et ne ferme pas la socket
	apres chaque fin de partie */
	while (1) {
		srand(time(NULL));
		mot_alea5(a_deviner);
		/*mettre la socket en ecoute passive */
		if (listen(sock,10) < 0 ) {
			perror("listen");
			continue;
		}
		struct sockaddr_in addr_clt;
		socklen_t taille_addr = sizeof(struct sockaddr_in);
		wordlp * w = malloc(sizeof(wordlp));
		int sock_echange = accept(sock,(struct sockaddr *) &addr_clt, &taille_addr);

		w->sock = sock_echange;
		char addr_char[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &(addr_clt.sin_addr), addr_char, INET_ADDRSTRLEN) == NULL) {
			perror("inet_ntop");
		} else {
		time_t now = time(NULL);
		char date_heure[32], log_mess[256];
		strftime(date_heure, 32, "%F:%T", localtime(&now));
		sprintf(log_mess, "%s : connection avec %s\n", date_heure, addr_char);
		pthread_mutex_lock(&mut_logfile);
		write(log, log_mess, strlen(log_mess));
		pthread_mutex_unlock(&mut_logfile);
		}
		struct machin * chin = malloc(sizeof(struct machin));
		chin->w = malloc(sizeof(wordlp));
		chin->w = w;
		chin->a_deviner = malloc(6*sizeof(char));
		strcpy(chin->a_deviner,a_deviner);
		
		pthread_create(&th,NULL,worker,chin);

		pthread_detach(th);
	}

	return 0;
}

void * worker (void * arg){
	struct machin * chin = arg;
	int nb_tentatives = 0;
	char prop_joueur[6];

	do {
		nb_tentatives = nb_tentatives + 1;
		/*le serveur recois la proposition du client */
		 recv(chin->w->sock,prop_joueur,6,0);
		 /*traite la proposition et lui renvoie une structure wordl_p contenant
		 des infos pour le client */
		 traiter_prop(prop_joueur, chin->a_deviner,chin->w);
		 send(chin->w->sock,&chin->w->carac_trouve,sizeof(int),0);
	} while (chin->w->carac_trouve < 5);
	 /*là le client a trouvé toutes les caracteres*/
	char msg[256];
	sprintf(msg,"Bravo, mot trouvé en %d tentatives\n", nb_tentatives);
	/*le serveur lui envoie ce dernier message lui indiquant qu il a trouvé en combien
	de tentatives*/
	send(chin->w->sock,msg,strlen(msg)+1,0);

	return NULL;
}

void chaine_toupper(char *ch)
{
	int i;
	for (i = 0; ch[i] != '\0'; i = i + 1)
		ch[i] = toupper(ch[i]);
}

int traiter_prop(const char *prop_joueur, const char *a_deviner,wordlp * wrld)
{
	int i, nb_lettres_trouvees = 0;
	for (i = 0; i < 5; i = i + 1) {
		if (prop_joueur[i] == a_deviner[i]) {
			wrld->prop[i] =  prop_joueur[i];
			nb_lettres_trouvees = nb_lettres_trouvees + 1;
		} else if (strchr(a_deviner, prop_joueur[i])) {
			wrld->prop[i] = tolower(prop_joueur[i]);
		} else {
			wrld->prop[i] = '_';
		}
	}
	wrld->prop[i] = '\0';
	wrld->carac_trouve = nb_lettres_trouvees;
	send(wrld->sock,wrld,sizeof(wrld),0);
	return nb_lettres_trouvees;
}

void vider_tampon()
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}
