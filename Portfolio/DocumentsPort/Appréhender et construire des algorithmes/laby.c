#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

////////////////////////////
// CONSTANTES
////////////////////////////

// paramèters généraux
#define TAILLE  20      // taille du labyrinthe
#define MAX_CAR 500     // longueur max d'une suite de caractères
#define PAUSE   300000  // durée dune pause entre 2 mouvements

// liste des symboles
#define MUR         '#'
#define PERSONNAGE  '@'
#define VIDE        ' '
#define POINT       '.'

// liste des "directions" gérées par le jeu
#define NORD  'N'
#define SUD   'S'
#define EST   'E'
#define OUEST 'O'

// position initiale du personnage (entrée du labyrinthe)
#define L_ENTREE 1
#define C_ENTREE 0

// position de la sortie
#define L_SORTIE TAILLE-2
#define C_SORTIE TAILLE-1


////////////////////////////
// DECLARATION DES TYPES
////////////////////////////
typedef char t_labyrinthe[TAILLE][TAILLE];
typedef char t_suite[MAX_CAR];

int pos_x_joueur = 0;
int pos_y_joueur = 0;
int compteur = 0 ; 

////////////////////////////
// PROTOTYPES DES FONCTIONS
////////////////////////////
int kbhit();
void afficher_entete(int longueur, int nbCartraites);
void afficher_labyrinthe(t_labyrinthe laby);
void charger_labyrinthe(t_labyrinthe laby, char fichier[]);
int charger_suite(char fichier[], t_suite t);
void deplacer_personnage(t_labyrinthe laby, int * l, int * y, char dir, int * nbCarTraites);
char convertir_mv_touche(char cardep);
void nettoyer(t_suite source, int nbCarSource, t_suite cible, int *nbCarCible);
int optimiser_fic_dep(t_labyrinthe laby, t_labyrinthe plateauCopie, int nbDeplacementsFic, t_suite t, t_suite *t_copie);


////////////////////////////
// PROGRAMME PRINCIPAL
////////////////////////////
int main(){
    t_labyrinthe leLabyrinthe;
    t_labyrinthe plateaucopie
    t_suite t;
    t_suite t_copy;
    t_labyrinthe laby ;
    t_suite plateauCopie;
    int lPersonnage = L_ENTREE; // coordonnées
    int cPersonnage = C_ENTREE; // du personnage
    int nbCarTraites = 0;
    char nomFichier[50];

    int nbCarInitial;
    t_suite laSuiteInitiale;
    int nbCar = 0;
    t_suite laSuite;
     int tailleDepOpti;

    int indice;
 

    charger_labyrinthe(leLabyrinthe, "labyrinthe.laby");
    printf("Nom de la suite à exécuter ? ");
    scanf("%s", nomFichier);

    nbCarInitial = charger_suite(nomFichier, laSuiteInitiale);
    nettoyer(laSuiteInitiale, nbCarInitial, laSuite, &nbCar);
    
    afficher_entete(nbCar, nbCarTraites);
    afficher_labyrinthe(leLabyrinthe);

    // boucle qui traite tous les caractères de la suite
    indice = 0;
    while (indice < nbCar){
        
        tailleDepOpti = optimiser_fic_dep(laby, plateaucopie, nbdeplacementsFic, t, &t_copy );
        deplacer_personnage(leLabyrinthe, &lPersonnage, &cPersonnage, laSuite[indice], &nbCarTraites , convertir_mv_touche(cSuivante));
        
        usleep(PAUSE);
        afficher_entete(nbCar, nbCarTraites);
        afficher_labyrinthe(leLabyrinthe);
        indice++;
        
    }
    if (lPersonnage == L_SORTIE && cPersonnage == C_SORTIE){
        printf("\n\nC'EST GAGNE\n\n");
    } else {
        printf("\n\nPERDU !\n\n");
    }
    //générer la bonne séquence
    t_suite laSuiteFinale;
    int indiceFinal = 0;
    for (int i=0 ; i<nbCar ; i++){
        if(laSuite[i] != '.'){
            laSuiteFinale[indiceFinal] = laSuite[i];
            indiceFinal++;
        }
    }
    printf("Suite optimisée (longueur %d): ", indiceFinal);
    for (int i=0 ; i< indiceFinal; i++){
        printf("%c", laSuiteFinale[i]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}

char convertir_mv_touche(char cardep) {

    if (cardep == 'n' || cardep == 'N') return 'z';
    if (cardep == 's' || cardep == 'S') return 's';
    if (cardep == 'e' || cardep == 'E') return 'q';
    if (cardep == 'o' || cardep == 'O') return 'd';

    return cardep;
}

void afficher_entete(int longueur, int nbCarTraites){
    // cette procédure efface l'écran puis affiche :
    //  - le nombre de mouvements effectués
    //  - et le nombre de collisions subies
    system("clear");
    printf("--------------------------------------------------\n");
    printf("           L A B Y R I N T H E\n\n");
    printf("Longueur suite : %d\n", longueur);   
    printf("Caractères traités : %d\n", nbCarTraites);   
    printf("--------------------------------------------------\n"); 
}

void afficher_labyrinthe(t_labyrinthe laby){
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                printf("%c", laby[ligne][colonne]);
            }
            printf("\n");
    }deplacer_personnage(t_labyrinthe laby, int * l, int * c, char dir, int * nbCarTraites)
}

void deplacer_personnage(t_labyrinthe laby, int * l, int * c, char dir, int * nbCarTraites){
    // cette fonction tente de déplacer le personnage dans le labyrinthe
    // *l et *c : ligne et colonne où se situe le personnage
    // dir : sens du déplacement souhaité (HAUT, BAS, GAUCHE ou DROITE)
    // nbCarTraites : nombre de caractères de la suite traités
    int lSuivante;
    int cSuivante;

    // détection de la case située APRES le personnage ("APRES" selon la direction)
    lSuivante = *l;
    cSuivante = *c;
    switch (dir){
        case OUEST :
            cSuivante = (*c)-1; break;
        case NORD :
            lSuivante = (*l)-1; break;
        case SUD :
            lSuivante = (*l)+1; break;
        case EST :
            cSuivante = (*c)+1; break;
    }
    
    // incrémente le nombre de caractères traités
    (*nbCarTraites)++;
    
    // si autorisé, effectue le déplacement du personnage
    if (laby[lSuivante][cSuivante] != MUR){
        laby[*l][*c] = VIDE;
        laby[lSuivante][cSuivante] = PERSONNAGE;
        *l = lSuivante;
        *c = cSuivante;
    }
}

void charger_labyrinthe(t_labyrinthe laby, char fichier[]){
    // cette procédure lit les caractères d'un fichier dont le nom est en paramètre
    // et les recopie dans le labyrinthe passé en paramètre
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&laby[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

int charger_suite(char fichier[], t_suite t){
    FILE * f;
    char dep;
    int nb = 0;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("FICHIER NON TROUVE\n");
        exit(EXIT_FAILURE);
    } else {
        fread(&dep, sizeof(char), 1, f);
        if (feof(f)){
            printf("FICHIER VIDE\n");
        } else {
            while (!feof(f)){
                t[nb] = dep;
                nb++;
                fread(&dep, sizeof(char), 1, f);
            }
        }
    }
    fclose(f);
    return nb;
}

void nettoyer(t_suite suite1, int nbCar1, t_suite suite2, int *nbCar2){
    for (t_suite suite 1){
        if(convertir_mv_touche == 'n') {
            (toupper('n')=='N');
        }
        if(convertir_mv_touche == 's' ) {
            (toupper('s')== 'S');
        }
        if(convertir_mv_touche == 'e') {
            (toupper('e')== 'E');
        }
        if(convertir_mv_touche == 'o') {
            (toupper('o')== 'O');
        }
    }
    return t_suite suite2;
}
int optimiser_fic_dep(t_labyrinthe laby, t_labyrinthe plateauCopie, int nbDeplacementsFic, t_suite t, t_suite *t_copie){

    int placementbon = 0;
    int pos = 0;
    char cSuivante;
    t_labyrinthe copie;
    t_labyrinthe avant_coup;
    int posXdep;
    int posYdep;
    int j = 0;
    int caseVisite = 0;
    int historiqueMvmX[MAX_CAR];
    int historiqueMvmY[MAX_CAR];
    int historiquePosJ[MAX_CAR];
    int IndiceMvmEnTrop = 0;
    bool caisseBouge;

    int svg_x = pos_x_joueur;
    int svg_y = pos_y_joueur;
    int svg_c = compteur;
    int svg_p = placementCorrect;

    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            plateauCopie [i][j] = plateauOriginal [i][j];     
        }
    }

    for (int i = 0; i < TAILLE; i++)
        for (int j = 0; j < TAILLE; j++)
            copie[i][j] = plateauCopie[i][j];

    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateauCopie[i][j] == '*'){
                placementbon++;
            }
        }
    }


        for ( && pos < nbDeplacementsFic){

        cSuivante = t[pos];

        if (cSuivante != '\0'){
            posXdep = pos_x_joueur;
            posYdep = pos_y_joueur;

            for(int k=0; k<TAILLE; k++)
                for(int l=0; l<TAILLE; l++)
                    avant_coup[k][l] = plateauCopie[k][l];

              deplacer_personnage(leLabyrinthe, &lPersonnage, &cPersonnage, laSuite[indice], &nbCarTraites , convertir_mv_touche(cSuivante));

            if(pos_x_joueur != posXdep || pos_y_joueur != posYdep){
                
                caisseBouge = false;
                for(int k=0; k<TAILLE; k++){
                    for(int l=0; l<TAILLE; l++){
                        if(avant_coup[k][l] != plateauCopie[k][l]){
                            caisseBouge = true;
                        }
                    }
                }

                if (caisseBouge){
                    (*t_copie)[j] = cSuivante;
                    j++;
                    caseVisite = 0;
                }
                else{
                    IndiceMvmEnTrop = -1;
                    for(int i = 0; i < caseVisite; i++){
                        if (historiqueMvmX[i] == pos_x_joueur && historiqueMvmY[i] == pos_y_joueur){
                            IndiceMvmEnTrop = historiquePosJ[i];
                            caseVisite = i + 1;
                            break;
                        }
                    }

                    if (IndiceMvmEnTrop != -1){
                        j = IndiceMvmEnTrop;
                    }
                    else{
                        if(j < TAILLE_DEPLACEMENTS){
                            (*t_copie)[j] = cSuivante;                    
                            historiqueMvmX[caseVisite] = pos_x_joueur;
                            historiqueMvmY[caseVisite] = pos_y_joueur;
                            historiquePosJ[caseVisite] = j;
                            j++;
                            caseVisite++;
                        }
                    }
                }
            }
        }
        pos++;
    }

    pos_x_joueur = svg_x;
    pos_y_joueur = svg_y;
    placementCorrect = svg_p;

    return j;
}