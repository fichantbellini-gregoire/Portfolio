if ( (*x < TAILLE) && (((*plateau)[*x+1][*y] == CASE && (*plateau[*x][*y] == '+')))){
            (*plateau)[*x+1][*y] = JOUEUR;
            (*plateau)[*x][*y] = CIBLE;
            *x = (*x) + 1;
        }

if ((*y > 0) && (((*plateau)[*x][*y-1] == CASE && (*plateau[*x][*y] == '+')))){
            (*plateau)[*x][*y-1] = JOUEUR;
            (*plateau)[*x][*y] = CIBLE;
            *y = (*y) - 1;
        }

if ((*x > 0) && (((*plateau)[*x-1][*y] == CASE && (*plateau[*x][*y] == '+')))){
            (*plateau)[*x-1][*y] = JOUEUR;
            (*plateau)[*x][*y] = CIBLE;
            *x = (*x) - 1;
        }

if ( (*x < TAILLE - 1) && (((*plateau)[*x][*y+1] == CASE && (*plateau[*x][*y] == '+')))){
            (*plateau)[*x][*y+1] = JOUEUR;
            (*plateau)[*x][*y] = CIBLE;
            *y= (*y) + 1;
        }