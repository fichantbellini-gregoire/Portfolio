/**
* @file sokoban_version1.c
* @brief Programme d'un Sokoban jouable
* @author Fichant--Bellini Grégoire
* @version Version 1.0
* @date 15/10/2025
*
* Exemple de programme qui permet de jouer au jeu du Sokoban en C
* dans le cadre de la SAE1.01.
*/

// Déclaration des fichiers inclus
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Déclaration des constantes et des constantes symboliques
#define TAILLE 12
const char CIBLE = '.', JOUEUR = '@', CASE = ' ', JOUEUR_SUR_CIBLE = '+'; 
const char BOITE = '$', BOITE_SUR_CIBLE = '*'; /*
BOITE et BOITE_SUR_CIBLE sont les caisses du jeux.*/ 
const char RESTART = 'r', ABANDON = 'x';

// Déclaration des types 
typedef char t_plateau[TAILLE][TAILLE];
typedef char t_tabCible[TAILLE][TAILLE];

// Déclaration des prototypes des fonctions et des procédures du programmme
void charger_partie(t_plateau plateau, char fichier[]);
void enregistrer_partie(t_plateau plateau, char fichier[]);
void afficher_plateau(t_plateau plateau);
void afficher_entete(char fichier[], int nbDeplacement);
int kbhit();
void trouver_joueur(t_plateau plateau, int *cordJoueurX, int *cordJoueurY);
void deplacer(t_plateau plateau, int *x, int *y, char touche, t_tabCible cibles, int *nbDeplacements);
void remplir_tabCible(t_plateau t_plateau, t_tabCible cibles);
bool gagner(t_plateau plateau, t_tabCible cibles);
void abandonner_partie(t_plateau plateau);
void deplacer_haut(t_plateau plateau, int *x, int *y, t_tabCible cibles);
void deplacer_gauche(t_plateau plateau, int *x, int *y, t_tabCible cibles);
void deplacer_droite(t_plateau plateau, int *x, int *y, t_tabCible cibles);
void deplacer_bas(t_plateau plateau, int *x, int *y, t_tabCible cibles);
void incrementer_deplacement(int *deplacementNiveau);
void recommencer_partie(t_plateau plateau, char fichier[], int *nbDeplacements);


// Définition de la fonction main 
int main(){
    t_plateau plateauJeu;
    char fichierChoisi[30];
    int joueurX = 0, joueurY = 0;
    int nbrDeplacement = 0;
    bool estGagne = false;
    char touche;
    t_tabCible ciblesNiveau;

    printf("Saisir l'un des niveaux (exemple : niveau1.sok):\n");
    scanf("%s", fichierChoisi);
    charger_partie(plateauJeu, fichierChoisi);
    remplir_tabCible(plateauJeu, ciblesNiveau);
    afficher_entete(fichierChoisi, nbrDeplacement);
    afficher_plateau(plateauJeu);
    while ((!estGagne) && (touche != ABANDON)){
        if (kbhit()){
        touche = getchar();
        trouver_joueur(plateauJeu, &joueurX, &joueurY);
        deplacer(plateauJeu, &joueurX, &joueurY, touche, ciblesNiveau, &nbrDeplacement);
        if (touche == RESTART){
            recommencer_partie(plateauJeu, fichierChoisi, &nbrDeplacement);
        }
        afficher_entete(fichierChoisi, nbrDeplacement);
        afficher_plateau(plateauJeu);
        estGagne = gagner(plateauJeu, ciblesNiveau);
    }
    }
    
    if (estGagne){
        printf("Mes félicitations, vous avez gagné et terminé le niveau !\n");
    } else if (touche == ABANDON){
        abandonner_partie(plateauJeu);
    }

    return EXIT_SUCCESS;
}

// Définition des fonctions et des procédures du programme
void charger_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER\n");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrer_partie(t_plateau plateau, char fichier[]){
    FILE * f;
    char finDeLigne='\n';

    f = fopen(fichier, "w");
    for (int ligne=0 ; ligne<TAILLE ; ligne++){
        for (int colonne=0 ; colonne<TAILLE ; colonne++){
            fwrite(&plateau[ligne][colonne], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }
    fclose(f);
}

/**
* @brief Procédure d'affichage du plateau de jeu.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau qu'on veut afficher.
*/
void afficher_plateau(t_plateau plateau){
    for (int lig = 0 ; lig < TAILLE ; lig++){
        for (int col = 0; col < TAILLE; col++){
            printf("%c", plateau[lig][col]);
        }
        printf("\n");
    }
}

/**
* @brief Procédure d'affichage de l'entête du jeu.
* @param fichier de type char, Entrée/Sortie : le nom du fichier choisi 
* par le joueur au début de la partie.
* @param nbDeplacement de type int, Entrée : le nombre de déplacements
* effectués par le joueur durant la partie.
*/
void afficher_entete(char fichier[], int nbDeplacement){
    system("clear");
    printf("---------------\n");
    printf("niveau : %s\n", fichier);
    printf("Liste des actions possibles\n");
    printf("Vers la droite : d\t Vers la gauche : q\t Vers le haut : z\n");
    printf("Vers le bas : s\t  Recommencer : r\t Abondonner : x\n");
    printf("Le nombre de déplacement : %d\n", nbDeplacement);
    printf("---------------\n");


}

int kbhit(){
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}

/**
* @brief Fonction testant si le joueur a gagné la partie.
* Le joueur gagne losrque toutes les caisses sont sur les cibles du niveau.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
* @return True si toutes les caisses du niveau
* sont sur toutes les cibles du niveau, False si ce n'est pas le cas.
*/
bool gagner(t_plateau plateau, t_tabCible cibles){

    int nbBoiteNiveau, nbBoiteSurCible;
    nbBoiteNiveau = 0;
    nbBoiteSurCible = 0;
    for(int i = 0 ; i < TAILLE ; i++){
        for(int j = 0 ; j < TAILLE ; j++){
            if (plateau[i][j] == CIBLE){
                nbBoiteNiveau += 1;
            } else if ((plateau[i][j] == BOITE) && (cibles[i][j] == CIBLE)){
                nbBoiteNiveau += 1;
            } else if ((plateau[i][j] == JOUEUR) && (cibles[i][j] == CIBLE)){
                nbBoiteNiveau += 1;
            }
        }
    }

    for(int i = 0 ; i < TAILLE ; i++){
        for(int j = 0 ; j < TAILLE ; j++){
            if ((plateau[i][j] == BOITE) && (cibles[i][j] == CIBLE)){
                nbBoiteSurCible += 1;
            }
        }
    }

    return nbBoiteNiveau == nbBoiteSurCible;

}

/**
* @brief Procédure abandonnant la partie si le joueur le souhaite.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
*/
void abandonner_partie(t_plateau plateau){
    char reponse[20];
    char nomFichierSauvegarde[40];
    printf("Vous avez décidé d'abandonner la partie\n");
    printf("Voulez-vous sauvegarder votre partie ?\n");
    printf("Tapez votre réponse : Oui ou Non\n");
    scanf("%s", reponse);
    while ((strcmp(reponse,"Oui") != 0) && (strcmp(reponse,"Non") != 0)){
        printf("Réponse Incorrecte\n");
        printf("Tapez une réponse entre Oui et Non\n");
        scanf("%s", reponse);
    }
    
    if (strcmp(reponse,"Non") == 0){
        printf("Vous avez décidé de ne pas sauvegarder votre partie\n");
        printf("Courage la prochaine tentative sera peut être la bonne\n");
    } else {
         printf("Saisir le nom de votre nouveau fichier en format .sok\n");
         scanf("%s", nomFichierSauvegarde);
        enregistrer_partie(plateau, nomFichierSauvegarde);
    }

}

/**
* @brief Procédure gérant le déplacement global du joueur dans le jeu
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param touche de type char, Entrée : la touche tapée par le joueur.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
* @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur.
*/
void deplacer(t_plateau plateau, int *x, int *y, char touche, t_tabCible cibles, int *nbDeplacements){

    switch (touche){
        case 'z': // représente la touche pour aller vers le haut.
            deplacer_haut(plateau, x, y, cibles);
            incrementer_deplacement(nbDeplacements);
            break;
        case 's': // représente la touche pour aller vers le bas.
            deplacer_bas(plateau, x, y, cibles);
            incrementer_deplacement(nbDeplacements);
            break;
        case 'd': // représente la touche pour aller vers la droite.
            deplacer_droite(plateau, x, y, cibles);
            incrementer_deplacement(nbDeplacements);
            break;
        case 'q': // représente la touche pour aller vers la gauche.
            deplacer_gauche(plateau, x, y, cibles);
            incrementer_deplacement(nbDeplacements);
            break;
        default:
            break;
    }
}

/**
*
* @brief Procédure permettant d'obtenir les cordonnées du joueur sur le plateau
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu. 
* @param cordJoueurX de type int, Entrée/Sortie : l'abscisse du joueur
* sur le plateau de jeu.
* @param cordJoueurY de type int, Entrée/Sortie : l'ordonnée du joueur
* sur le plateau de jeu.
*/
void trouver_joueur(t_plateau plateau, int *cordJoueurX, int *cordJoueurY){
    for (int y = 0 ; y < TAILLE ; y++){
        for (int x = 0 ; x < TAILLE ; x++){
            if (plateau[x][y] == JOUEUR){
                *cordJoueurX = x;
                *cordJoueurY = y;
            }
        }
    }
}

/**
* @brief Procédure déplaçant vers le haut le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
*/
void deplacer_haut(t_plateau plateau, int *x, int *y, t_tabCible cibles){
    
    if ((*x > 0) && (plateau[*x-1][*y] == CASE)){ 
        plateau[*x-1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) - 1;
    } else if (plateau[*x-1][*y] == CIBLE){
        plateau[*x-1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) - 1;
    } else if ((*x > 1) && (plateau[*x-1][*y] == BOITE)){
        if ((plateau[*x-2][*y] == CASE) || (plateau[*x-2][*y] == CIBLE)){
            plateau[*x-1][*y] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x-2][*y] = BOITE;
            *x = (*x) - 1;
        }
    // Cas particulier du niveau 6 où une boite est déjà sur une cible du plateau.                                                            */    
    } else if ((*x > 1) && (plateau[*x-1][*y] == BOITE_SUR_CIBLE)){
        if ((plateau[*x-2][*y] == CASE) || (plateau[*x-2][*y] == CIBLE)){
            plateau[*x-1][*y] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x-2][*y] = BOITE;
            *x = (*x) - 1;
        } 
        
    }
}


/**
* @brief Procédure déplaçant vers la gauche le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
*/
void deplacer_gauche(t_plateau plateau, int *x, int *y, t_tabCible cibles){
    
    if ((*y > 0) && (plateau[*x][*y-1] == CASE)){
        plateau[*x][*y-1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) - 1;
    } else if (plateau[*x][*y-1] == CIBLE){
        plateau[*x][*y-1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) - 1;
    } else if ((*y > 1) && (plateau[*x][*y-1] == BOITE)){
        if ((plateau[*x][*y-2] == CASE) || (plateau[*x][*y-2] == CIBLE)){
            plateau[*x][*y-1] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x][*y-2] = BOITE;
            *y = (*y) -1;
        }
    // Cas particulier du niveau 6 où une boite est déjà sur une cible du plateau.
    } else if ((*y > 1) && (plateau[*x][*y-1] == BOITE_SUR_CIBLE)){   
        if ((plateau[*x][*y-2] == CASE) || (plateau[*x][*y-2] == CIBLE)){
            plateau[*x][*y-1] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x][*y-2] = BOITE;
            *y = (*y) -1;
        }
    }

}

/**
* @brief Procédure déplaçant vers le bas le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
*/
void deplacer_bas(t_plateau plateau, int *x, int *y, t_tabCible cibles){

    if ((*x < TAILLE) && (plateau[*x+1][*y] == CASE)){
        plateau[*x + 1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) + 1;
    } else if (plateau[*x+1][*y] == CIBLE){
        plateau[*x+1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) + 1;
    } else if ((*x < TAILLE - 1) && (plateau[*x+1][*y] == BOITE)){
        if ((plateau[*x + 2][*y] == CASE) || (plateau[*x + 2][*y] == CIBLE)){
            plateau[*x + 1][*y] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x + 2][*y] = BOITE;
            *x = (*x) + 1;
        }
    // Cas particulier du niveau 6 où une boite est déjà sur une cible du plateau.
    } else if ((*x < TAILLE - 1) && (plateau[*x+1][*y] == BOITE_SUR_CIBLE)){
        if ((plateau[*x + 2][*y] == CASE) || (plateau[*x + 2][*y] == CIBLE)){
            plateau[*x + 1][*y] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x + 2][*y] = BOITE;
            *x = (*x) + 1;
        } 
    }
}

/**
* @brief Procédure déplaçant vers la droite le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
*/
void deplacer_droite(t_plateau plateau, int *x, int *y, t_tabCible cibles){

    if ((*y < TAILLE - 1) && (plateau[*x][*y + 1] == CASE)){
        plateau[*x][*y + 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) + 1;
    } else if (plateau[*x][*y+1] == CIBLE){
        plateau[*x][*y+1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) + 1;
    } else if ((*y < TAILLE - 2) && (plateau[*x][*y + 1] == BOITE)){
        if ((plateau[*x][*y + 2] == CASE) || (plateau[*x][*y + 2] == CIBLE)){
            plateau[*x][*y + 1] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x][*y + 2] = BOITE;
            *y = (*y) + 1;
        }
    // Cas particulier du niveau 6 où une boite est déjà sur une cible du plateau.
    } else if ((*y < TAILLE - 2) && (plateau[*x][*y + 1] == BOITE_SUR_CIBLE)){
        if ((plateau[*x][*y + 2] == CASE) || (plateau[*x][*y + 2] == CIBLE)){
            plateau[*x][*y + 1] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x][*y + 2] = BOITE;
            *y = (*y) + 1;
        }
    }
}

/**
* @brief Procédure remplissant le tableau des coordonnées des cibles d'un niveau.
* Il est composé de cases du plateau et de cibles.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
*/
void remplir_tabCible(t_plateau plateau, t_tabCible cibles){
    for (int lgn = 0 ; lgn < TAILLE ; lgn++){
        for (int col = 0; col < TAILLE; col++){
            if (plateau[lgn][col] == CIBLE){
                cibles[lgn][col] = CIBLE;
            } else if (plateau[lgn][col] == BOITE_SUR_CIBLE){
                cibles[lgn][col] = CIBLE;
            } else if (plateau[lgn][col] == JOUEUR_SUR_CIBLE){
                cibles[lgn][col] = CIBLE;
            } else {
                cibles[lgn][col] = CASE;
            }
        }
    }
}

/**
* @brief Procédure incrémentant de 1 le nombre de déplacements
* effectués par le joueur.
* @param deplacementNiveau de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur.
*/
void incrementer_deplacement(int *deplacementNiveau){
    (*deplacementNiveau)++;
}

/**
* @brief Procédure recommançant la partie depuis le début si le joueur le décide
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param fichier de type char, Entrée/Sortie : le nom fichier choisi 
* par le joueur au début de la partie.
* @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur. Il sera remis à 0 si le joueur décide
* de recommencer la partie.
*/
void recommencer_partie(t_plateau plateau, char fichier[], int *nbDeplacements){
    char repQuestion[20];
    printf("Voulez recommencer le niveau depuis le début ?\n");
    printf("Tapez votre réponse : Oui ou Non ?\n");
    scanf("%s", repQuestion);

    while ((strcmp(repQuestion,"Oui") != 0) && (strcmp(repQuestion,"Non") != 0)){
        printf("Réponse Incorrecte\n");
        printf("Tapez une réponse entre Oui et Non\n");
        scanf("%s", repQuestion);
    }

    if (strcmp(repQuestion,"Oui") == 0){
        charger_partie(plateau, fichier);
        *nbDeplacements = 0;
    }

}
