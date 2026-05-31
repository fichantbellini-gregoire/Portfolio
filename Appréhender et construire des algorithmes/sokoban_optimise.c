/**
* @file sokoban_optimise.c
* @brief Programme d'un Sokoban jouable devenue automatique
* @author Fichant--Bellini Grégoire
* @version Version 1.0
* @date 10/11/2025
*
* Exemple de programme qui permet de jouer au jeu du Sokoban en C
* dans le cadre de la SAE1.02.
*/

// Déclaration des fichiers inclus
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

// Déclaration des constantes et des constantes symboliques
#define TAILLE 12
#define TAILLE_MAX_DEPLACEMENT 500
#define TAILLE_CORDONNEES 2
#define NB_CAISSE_MAX 97


const char CIBLE = '.';
const char JOUEUR = '@';
const char CASE = ' ';
const char JOUEUR_SUR_CIBLE = '+'; 
const char BOITE = '$';
const char BOITE_SUR_CIBLE = '*'; /*
BOITE et BOITE_SUR_CIBLE sont les caisses du jeux.*/ 
const char RESTART = 'r';
const char ABANDON = 'x';
const char ANNULER_COUP = 'u';
const char DEZOOM = '-';
const char ZOOM ='+';
const char VIDE = ' ';

const int ZERO = 0;

// Déclaration des types 
typedef char t_plateau[TAILLE][TAILLE];
typedef char t_tabCible[TAILLE][TAILLE];
typedef int t_cordonnees[TAILLE_CORDONNEES];
typedef char t_tabDeplacement[TAILLE_MAX_DEPLACEMENT];
typedef t_cordonnees t_joueur[TAILLE_MAX_DEPLACEMENT+1];//tableau qui enregistre le placement du joueur a toutes les étapes
typedef t_cordonnees t_caisse_instant[NB_CAISSE_MAX];//tableau qui enregistre la position des caisse a un instant t
typedef t_caisse_instant t_caisse[TAILLE_MAX_DEPLACEMENT+1];//tableau qui enregistre le placement des caisses (dépplacé) au cours de la partie

typedef struct{
    t_joueur joueur;
    t_caisse caisse_bouge;
    int etat_actuel;
}t_jeu;

const t_cordonnees CAISSE_PAS_BOUGE = [ZERO,ZERO];

// Déclaration des prototypes des fonctions et des procédures du programmme
void charger_partie(t_plateau plateau, char fichier[]);
void enregistrer_partie(t_plateau plateau, char fichier[]);
void afficher_plateau(t_plateau plateau, int zoom);
void afficher_entete(char fichier[], int nbDeplacement);
int kbhit();
void trouver_joueur(t_plateau plateau, int *cordJoueurX, int *cordJoueurY);
bool deplacer(t_plateau plateau, int *x, int *y, char touche, t_tabCible cibles, int *nbDeplacements, t_tabDeplacement t_deplacement);
void remplir_tabCible(t_plateau t_plateau, t_tabCible cibles);
bool gagner(t_plateau plateau, t_tabCible cibles);
void abandonner_partie(t_plateau plateau);
bool deplacer_haut(t_plateau plateau, int *x, int *y,t_tabCible cibles, int *nbDeplacements, t_tabDeplacement t_deplacement);
bool deplacer_gauche(t_plateau plateau, int *x, int *y,t_tabCible cibles, int *nbDeplacements, t_tabDeplacement t_deplacement);
bool deplacer_droite(t_plateau plateau, int *x, int *y,t_tabCible cibles, int *nbDeplacements, t_tabDeplacement t_deplacement);
bool deplacer_bas(t_plateau plateau, int *x, int *y,t_tabCible cibles, int *nbDeplacements, t_tabDeplacement t_deplacement);
void recommencer_partie(t_plateau plateau, char fichier[], int *nbDeplacements);
void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]);
void gerer_zoom(int *zoomPartie, char toucheClavier);
void sauvegarder_deplacements(t_tabDeplacement t, int nb_deplacement);
void dectection_mouv_sans_caisse(t_deplacement t,int *deplacement);
void dectection_mouv_avec_caisse(t_deplacement t,int *deplacement);


void annuler_deplacement(t_plateau plateau, int *x, int *y, t_tabCible cibles, int *nbDeplacements, t_tabDeplacement t_deplacement);
void fusionner_plateaux(t_plateau plateau, t_tabCible cibles);
char transDep2Mouv(char dep);
void chargerDeplacements(t_tabDeplacement t, char fichier[], int * nb);


void init_jeu(t_jeu *jeux,t_plateau plateauJeu);
void init_joueur(t_joueur joueur,int debut);
void init_caisse(t_caisse caisse_bouge,int debut);
void mise_a_jour_caisse();
void mise_a_jour_joueur(t_jeu *jeux,char touche);





// Définition de la fonction main (les commentaires avec tabulation sont les commandes d'affichage)
int main(){
    t_plateau plateauJeu;
    t_jeu jeux;
    char fichierChoisiSok[30];
    char fichierChoisiDep[30];
    int joueurX = 0, joueurY = 0;
    int nbrDeplacement = 0; //le nombre de déplacement effectué par le joueur
    int deplacementCorrigé // le nombre de déplacement de la version corrigé
    char touche;
    t_tabCible ciblesNiveau;
    t_tabDeplacement tableauDeplacement;//l'historique qui va etre crée durant la partie
    int zoom_niveau = 1;
    t_tabDeplacement tabMouvement;//l'historique en provenance du fichier dep
    int deplacementTotal = 0;
    int depEnTrop;
    bool mouv_caisse;

        //printf("Saisir l'un des niveaux (exemple : niveau1.sok):\n");//initialisation du plateau
    scanf("%s", fichierChoisiSok);
    charger_partie(plateauJeu, fichierChoisiSok);


        //printf("Saisir l'un des déplacement (exemple : mouvement.dep):\n");//initialisation des
    scanf("%s",fichierChoisiDep);
    chargerDeplacements(tabMouvement,fichierChoisiDep,&deplacementTotal);

    remplir_tabCible(plateauJeu, ciblesNiveau);
        //afficher_entete(fichierChoisiDep, nbrDeplacement);
    afficher_plateau(plateauJeu, zoom_niveau);
    trouver_joueur(plateauJeu, &joueurX, &joueurY);

    //initialisation de la structure de jeu
    init_jeu(&jeux,plateauJeu);
    


    while ((!gagner(plateauJeu,ciblesNiveau)) && nbrDeplacement < deplacementTotal+1){//nbrDeplacement nous servira d'indice
        
        touche=transDep2Mouv(tabMouvement[nbrDeplacement]);

        

        if (touche == ANNULER_COUP){
            annuler_deplacement(plateauJeu,&joueurX,&joueurY,ciblesNiveau,&nbrDeplacement,tabMouvement);
            if (deplacementCorrigé-2 >= 0){//on evite de faire une erreur de segmentation
                deplacementCorrigé = deplacementCorrigé-2;//on retire le undo et le mouvement enlevé de l'historique final
            }
        }
        else{
            mouv_caisse = deplacer(plateauJeu, &joueurX, &joueurY, touche, ciblesNiveau, //deplacement
                &deplacementCorrigé, tableauDeplacement);

            if(jeux.joueur[jeux.etat_actuel][0]!= joueurX || jeux.joueur[jeux.etat_actuel][1]!= joueurY){//si le joueur c'est deplacer
                mise_a_jour_joueur();
                if(mouv_caisse){
                    mise_a_jour_caisse();
                }
            }

        }
        nbrDeplacement++;

            //afficher_entete(fichierChoisiSok, nbrDeplacement);//affichage 
        afficher_plateau(plateauJeu, zoom_niveau);

        usleep(250000);
    }
    if (gagner(plateauJeu,ciblesNiveau)){
        depEnTrop = deplacementTotal-nbrDeplacement;
        printf("La suite de déplacements %s est bien une solution pour la partie %s.",fichierChoisiDep,fichierChoisiSok);
        if (depEnTrop == 0){
            printf("et il n'y a aucun déplacement en trop.\n");
        }
        else{
            printf("cependant il y a %d déplacement en trop.\n",depEnTrop);
        }
    }
    else{
        printf("La suite de déplacements %s N’EST PAS une solution pour la partie %s\n",fichierChoisiDep,fichierChoisiSok);
    }

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
* Celui peut être en x1, en x2 ou en x3.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau qu'on veut afficher.
* @param zomm de type int, Entrée : représente le zoom actuel du niveau.
* Il évolue au cours de la partie selon les actions du joueur.
*/
void afficher_plateau(t_plateau plateau, int zoom){
    for (int lig = 0 ; lig < TAILLE ; lig++){
        for (int zoomLig = 0; zoomLig < zoom; zoomLig++){
            for (int col = 0; col < TAILLE; col++){
                for (int zoomCol = 0; zoomCol < zoom; zoomCol++){
                    printf("%c", plateau[lig][col]);

                }

            }
            printf("\n");
        }
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
    printf("Annuler le déplacement : u\t Zommer : +\t Dézommer : -\n");
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
* @brief Procédure abandonnant la partie si le joueur le souhaite
* Elle est utilisé soit lorsque le joueur gagne la partie
* soit lorsqu'il abandonne. 
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
* effectués par le joueur.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
* @param tDeplacement de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
*/
bool deplacer(t_plateau plateau, int *x, int *y, char touche, 
t_tabCible cibles, int *nbDeplacements, t_tabDeplacement tDeplacement){
    bool caisseBouger = false;

    switch (touche){
        case 'z': // représente la touche pour aller vers le haut.
            deplacer_haut(plateau, x, y, cibles, nbDeplacements, tDeplacement, caisseBouger);
            break;
        case 's': // représente la touche pour aller vers le bas.
            deplacer_bas(plateau, x, y, cibles, nbDeplacements, tDeplacement, caisseBouger);
            break;
        case 'd': // représente la touche pour aller vers la droite.
            deplacer_droite(plateau, x, y, cibles, nbDeplacements, tDeplacement, caisseBouger);
            break;
        case 'q': // représente la touche pour aller vers la gauche.
            deplacer_gauche(plateau, x, y, cibles, nbDeplacements, tDeplacement, caisseBouger);
            break;
        default://si une touche n'existe pas
            break;
    }
    return caisseBouger;
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
            if (plateau[x][y] == JOUEUR || plateau[x][y] == JOUEUR_SUR_CIBLE){
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
* @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur. Il sert aussi d'indice pour 
* le tableau de stockage des déplacements.
* @param tabloDeplacement de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
*/
bool deplacer_haut(t_plateau plateau, int *x, int *y,
t_tabCible cibles, int *nbDeplacements, t_tabDeplacement tabloDeplacement, bool *caisseBougee){
    
    if ((*x > 0) && (plateau[*x - 1][*y] == CASE)){ 
        plateau[*x - 1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) - 1;
        tabloDeplacement[*nbDeplacements] = 'h';
        *caisseBougee = false;

    } else if (plateau[*x - 1][*y] == CIBLE){
        plateau[*x - 1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) - 1;
        tabloDeplacement[*nbDeplacements] = 'h';
        *caisseBougee = false;

    } else if ((*x > 1) && ((plateau[*x - 1][*y] == BOITE) || 
    (plateau[*x - 1][*y] == BOITE_SUR_CIBLE))){
        if ((plateau[*x - 2][*y] == CASE) || (plateau[*x - 2][*y] == CIBLE)){
            plateau[*x - 1][*y] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x - 2][*y] = BOITE;
            *x = (*x) - 1;
            tabloDeplacement[*nbDeplacements] = 'H';
            *caisseBougee = true;
        }
    }
    return caisseBougee;
}


/**
* @brief Procédure déplaçant vers la gauche le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
 @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur. Il sert aussi d'indice pour 
* le tableau de stockage des déplacements.
* @param tabloDeplacement de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
*/
bool deplacer_gauche(t_plateau plateau, int *x, int *y,
t_tabCible cibles, int *nbDeplacements, t_tabDeplacement tabloDeplacement, bool *caisseBougee){

    if ((*y > 0) && (plateau[*x][*y - 1] == CASE)){
        plateau[*x][*y - 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) - 1;
        tabloDeplacement[*nbDeplacements] = 'g';
        *caisseBougee = false;

    } else if (plateau[*x][*y - 1] == CIBLE){
        plateau[*x][*y - 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) - 1;
        tabloDeplacement[*nbDeplacements] = 'g';
        *caisseBougee = false;
    
    } else if ((*y > 1) && ((plateau[*x][*y - 1] == BOITE) || 
    (plateau[*x][*y - 1] == BOITE_SUR_CIBLE))){
        if ((plateau[*x][*y - 2] == CASE) || (plateau[*x][*y - 2] == CIBLE)){
            plateau[*x][*y - 1] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x][*y - 2] = BOITE;
            *y = (*y) - 1;
            tabloDeplacement[*nbDeplacements] = 'G';
            *caisseBougee = true;
        }
    } 
    return caisseBougee;
}

/**
* @brief Procédure déplaçant vers le bas le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
 @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur. Il sert aussi d'indice pour 
* le tableau de stockage des déplacements.
* @param tabloDeplacement de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
*/
bool deplacer_bas(t_plateau plateau, int *x, int *y,
t_tabCible cibles, int *nbDeplacements, t_tabDeplacement tabloDeplacement, bool *caisseBougee){

    if ((*x < TAILLE) && (plateau[*x + 1][*y] == CASE)){
        plateau[*x + 1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) + 1;
        tabloDeplacement[*nbDeplacements] = 'b';
        *caisseBougee = false;
    
    } else if (plateau[*x + 1][*y] == CIBLE){
        plateau[*x+1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) + 1;
        tabloDeplacement[*nbDeplacements] = 'b';
        *caisseBougee = false;
        
    } else if ((*x < TAILLE - 1) && ((plateau[*x + 1][*y] == BOITE) || 
    (plateau[*x + 1][*y] == BOITE_SUR_CIBLE))){
        if ((plateau[*x + 2][*y] == CASE) || (plateau[*x + 2][*y] == CIBLE)){
            plateau[*x + 1][*y] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x + 2][*y] = BOITE;
            *x = (*x) + 1;
            tabloDeplacement[*nbDeplacements] = 'B';
            *caisseBougee = true;
        }
    }
    return caisseBougee;
}

/**
* @brief Procédure déplaçant vers la droite le joueur
* et les caisses si elles sont poussées.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
 @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur. Il sert aussi d'indice pour 
* le tableau de stockage des déplacements.
* @param tabloDeplacement de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
*/
bool deplacer_droite(t_plateau plateau, int *x, int *y,
t_tabCible cibles, int *nbDeplacements, t_tabDeplacement tabloDeplacement, bool *caisseBougee){

    if ((*y < TAILLE - 1) && (plateau[*x][*y + 1] == CASE)){
        plateau[*x][*y + 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) + 1;
        tabloDeplacement[*nbDeplacements] = 'd';
        *caisseBougee = false;
    
    } else if (plateau[*x][*y + 1] == CIBLE){
        plateau[*x][*y + 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) + 1;
        tabloDeplacement[*nbDeplacements] = 'd';
        *caisseBougee = false;
    
    } else if ((*y < TAILLE - 2) && ((plateau[*x][*y + 1] == BOITE) || 
    (plateau[*x][*y + 1] == BOITE_SUR_CIBLE))){
        if ((plateau[*x][*y + 2] == CASE) || (plateau[*x][*y + 2] == CIBLE)){
            plateau[*x][*y + 1] = JOUEUR;
            plateau[*x][*y] = cibles[*x][*y];
            plateau[*x][*y + 2] = BOITE;
            *y = (*y) + 1;
            tabloDeplacement[*nbDeplacements] = 'D';
            *caisseBougee = true;   
        }
    }
    return caisseBougee;
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
* @brief Procédure recommançant la partie depuis le début si le joueur le décide
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param fichier de type char, Entrée/Svoid enregistrerDeplacements(t_tabDeplacement fichierChoisiSok[30];lacement t, int nb, char fic[])ortie : le nom fichier choisi 
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

void enregistrerDeplacements(t_tabDeplacement t, int nb, char fic[]){
    FILE * f;

    f = fopen(fic, "w");
    fwrite(t,sizeof(char), nb, f);
    fclose(f);
}

/**
* @brief Procédure demandant au joueur s'il souhaite sauvegarder les
* déplacements qu'il a effectué durant la partie. Elle est utilisé soit
* lorsque le joueur abandonne ou lorsque qu'il gagne.
* @param t de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
* @param nbDeplacement de type int, Entrée : le nombre de déplacement
* total effectué par le joueur au cours de sa partie.
*/
void sauvegarder_deplacements(t_tabDeplacement t, int nbDeplacement){
    char reponseQuestion[20];
    char nomDuFichier[30];
    printf("Voulez vous sauvegarder les déplacements de la partie ?\n");
    printf("Tapez une réponse entre Oui et Non\n");
    scanf("%s", reponseQuestion);
    while ((strcmp(reponseQuestion,"Oui") != 0) && (strcmp(reponseQuestion,"Non") != 0)){
        printf("Réponse Incorrecte\n");
        printf("Tapez une réponse entre Oui et Non\n");
        scanf("%s", reponseQuestion);
    }
    if (strcmp(reponseQuestion,"Oui") == 0){
        printf("Saisir le nom du fichier sous la forme nom.dep :\n");
        scanf("%s", nomDuFichier);
        enregistrerDeplacements(t, nbDeplacement, nomDuFichier);

    }

}

/**
* @brief Procédure gérant le niveau de zoomage du plateau de jeu.
* Elle permet soit d'augmenter le niveau de zoom du plateau jusqu'a x3
* soit de le diminuer jusqu'à x1.
* @param zoomPartie de type int, Entrée/Sortie : représente le niveau de zoom
* actuel du niveau. Il sera soit augmenter, soit diminuer, soit maintenu.
* @param toucheClavier de type char, Entrée: la touche tapée par le joueur.
* Elle permet de savoir si le joueur a décidé d'augmenter le niveau
* du zoom du niveau par le '+' ou de le diminuer avec '-'
*/
void gerer_zoom(int *zoomPartie, char toucheClavier){   
   if ((*zoomPartie < 3 ) && (toucheClavier == '+')){
    (*zoomPartie)++;
   } else if ((*zoomPartie > 1) && (toucheClavier == '-')){
    (*zoomPartie) = (*zoomPartie) - 1;
   }
}

/**
* @brief Procédure permettant d'annuler le déplacement du joueur si
* celui le souhaite. Elle désincrémente aussi le nombre de déplacement
* du joueur de 1.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau de jeu.
* @param x de type int, Entrée/Sortie : l'abscisse du joueur sur le plateau.
* @param y de type int, Entrée/Sortie : l'ordonnée du joueur sur le plateau.
* @param touche de type char, Entrée : la touche tapée par le joueur.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau de coordonnées
* des cibles du niveau.
* @param nbDeplacements de type int, Entrée/Sortie : le nombre total de déplacements
* effectués par le joueur.
* @param tabDeplacement de type t_tabDeplacement, Entrée/Sortie : le tableau
* où sont mémorisés les déplacements du joueur.
*/
void annuler_deplacement(t_plateau plateau, int *x, int *y, t_tabCible
cibles, int *nbDeplacements, t_tabDeplacement tabDeplacement){
    
    if (tabDeplacement[*nbDeplacements - 1] == 'h'){
        plateau[*x + 1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) + 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;

    } else if (tabDeplacement[*nbDeplacements - 1] == 'H'){
        plateau[*x + 1][*y] = JOUEUR;
        plateau[*x][*y] = BOITE;
        plateau[*x - 1][*y] = cibles[*x - 1][*y];
        *x = (*x) + 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;

    } else if (tabDeplacement[*nbDeplacements - 1] == 'b'){
        plateau[*x - 1][*y] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *x = (*x) - 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;

    } else if (tabDeplacement[*nbDeplacements - 1] == 'B'){
        plateau[*x - 1][*y] = JOUEUR;
        plateau[*x][*y] = BOITE;
        plateau[*x + 1][*y] = cibles[*x + 1][*y];
        *x = (*x) - 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;

    } else if (tabDeplacement[*nbDeplacements - 1] == 'd'){
        plateau[*x][*y - 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) - 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;

    } else if (tabDeplacement[*nbDeplacements - 1] == 'D'){
        plateau[*x][*y - 1] = JOUEUR;
        plateau[*x][*y] = BOITE;
        plateau[*x][*y + 1] = cibles[*x][*y + 1];
        *y = (*y) - 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;

    } else if (tabDeplacement[*nbDeplacements - 1] == 'g'){
        plateau[*x][*y + 1] = JOUEUR;
        plateau[*x][*y] = cibles[*x][*y];
        *y = (*y) + 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;
        
    } else if (tabDeplacement[*nbDeplacements - 1] == 'G'){
        plateau[*x][*y + 1] = JOUEUR;
        plateau[*x][*y] = BOITE;
        plateau[*x][*y - 1] = cibles[*x][*y - 1];
        *y = (*y) + 1;
        tabDeplacement[*nbDeplacements - 1] = VIDE;
        tabDeplacement[*nbDeplacements] = VIDE;
        
    }
}

/**
* @brief Procédure qui fusionne le plateau de jeu avec le tableau contenant
* l'emplacement des cibles. Elle remplace certains caractères sur
* le plateau comme par exemple si le joueur est sur une cible.
* @param plateau de type t_plateau, Entrée/Sortie : le plateau
* de jeu représentant le niveau auquel joue le joueur.
* @param cibles de type t_tabCible, Entrée/Sortie : le tableau
* où les coordonnées des cibles sont stockées.
*/
void fusionner_plateaux(t_plateau plateau, t_tabCible cibles){
    
    for (int i = 0; i < TAILLE; i++ ){
        for (int j = 0; j < TAILLE; j++){
            if ((plateau[i][j] == BOITE) && (cibles[i][j] == CIBLE)){
                plateau[i][j] = BOITE_SUR_CIBLE;
            } else if ((plateau[i][j] == JOUEUR) && (cibles[i][j] == CIBLE)){
                plateau[i][j] = JOUEUR_SUR_CIBLE;
            }
        }
    }

}


void chargerDeplacements(t_tabDeplacement t, char fichier[], int * nb){
    FILE * f;
    char dep;
    *nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){
                t[*nb] = dep;
                (*nb)++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
}

char transDep2Mouv(char dep){
    char touche;
    char rep;
    touche = tolower(dep);
    return rep;
}

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// PARTIE OPTIMISATION //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


void init_jeu(t_jeu *jeux,t_plateau plateauJeu){
    jeux -> etat_actuel = ZERO;
    init_jeu(jeux->joueur,jeux->etat_actuel);
    init_caisse(jeux->caisse_bouge,jeux->etat_actuel);
    trouver_joueur(plateauJeu,&jeux -> joueur[jeux->etat_actuel][0],&jeux -> joueur[jeux->etat_actuel][1]);
}

void init_joueur(t_joueur joueur,int debut){
    int i;
    for(i = debut ; i < TAILLE_MAX_DEPLACEMENT+1 ; i++){
        joueur[i][0] = ZERO;
        joueur[i][1] = ZERO;
    }
}

void init_caisse(t_caisse caisse_bouge,int debut){
    int i;
    int j;
    for(i = debut ; i < TAILLE_MAX_DEPLACEMENT+1;i++){
        for(j = ZERO ; j < NB_CAISSE_MAX ;i++){
            caisse_bouge[i][j][0] = ZERO;
            caisse_bouge[i][j][1] = ZERO;
        }
    }
}

void trouver_caisse(t_caisse_instant caisse,t_plateau plateau){
    int i = ZERO;
    int j;
    int col;
    int lig;
    for (lig = ZERO ; lig < TAILLE ; lig++){
        for(col = ZERO ; col < TAILLE ; col++){
            if (plateau[lig][col]== BOITE || plateau[lig][col]==BOITE_SUR_CIBLE){
                caisse[i][0]=lig;
                caisse[i][1]=col;
                if(i+1 < NB_CAISSE_MAX){
                    i++;
                }
                
            }
        }
    }
    for(j = i ; j < NB_CAISSE_MAX ; j++){
        caisse[j][0] = ZERO;
        caisse[j][1] = ZERO;
    }
}
void mise_a_jour_joueur(t_jeu *jeux,char touche){
    if (jeux->etat_actuel+1 <= TAILLE_MAX_DEPLACEMENT){//on evite l'erreur de segmentation
        //on prend la derniere place connu du joueur
        jeux->joueur[jeux->etat_actuel+1][0] = jeux->joueur[jeux->etat_actuel][0];
        jeux->joueur[jeux->etat_actuel+1][1] = jeux->joueur[jeux->etat_actuel][1];

        jeux->etat_actuel++;
        switch(touche){
            case('q') :
                        (jeux->joueur[jeux->etat_actuel][1])--;
                        break;
        
            case('z') :
                        (jeux->joueur[jeux->etat_actuel][0])--;
                        break;
            
            case('s') :
                        (jeux->joueur[jeux->etat_actuel][0])++;
                        break;
            
            default : 
                        (jeux->joueur[jeux->etat_actuel][1])++;
                        break;
        }
    }
}