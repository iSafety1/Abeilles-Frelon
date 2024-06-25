#include <stdlib.h>
#include <stdio.h>
#include <MLV/MLV_all.h>

char joueur_actu;
int phase_joueur;
int phase_tour;

#define CASE 50

// Dimensions de la grille en nombre de cases (origine en haut a gauche) :
#define COLONNES 12
#define LIGNES 18

// Les deux camps :
#define CAABEILLE 'A'
#define CAFRELON 'F'

// Les types d'unites :
#define REINE 'r'
#define OUVRIERE 'o'
#define ESCADRON 'e'
#define GUERRIERE 'g'
#define FRELON 'f'
#define RUCHE 'R'
#define NID 'N'

// Pour la recolte de pollen
#define RECOLTE 'p'
#define TRECOLTE 4
#define CRECOLTE 4

// Les temps necessaires a la production :
#define TREINEA 8
#define TREINEF 8
#define TOUVRIERE 2
#define TGUERRIERE 4
#define TESCADRON 6
#define TFRELON 5
#define TRECOLTE 4
#define TRUCHE 1
#define TNID 1

// Les couts necessaires a la production :
#define CREINEA 7
#define CREINEF 8
#define COUVRIERE 3
#define CGUERRIERE 5
#define CESCADRON 6
#define CFRELON 3
#define CRUCHE 10
#define CNID 10

// La force des unites
#define FREINE 6
#define FOUVRIERE 1
#define FGUERRIERE 5
#define FESCADRON 12
#define FFRELON 8


// La structure Unite :
typedef struct unite {
    char camp; // ABEILLE ou FRELON
    char type; // RUCHE, NID, REINE, OUVRIERE, GUERRIERE, ESCADRON ou FRELON
    int force; // la force de l'unite
    int posx, posy; // position actuelle sur la grille
    int destx, desty; // destination (negatif si immobile)
    char production; // production d'une ruche ou d'un nid et RECOLTE pour la recolte de pollen
    int temps; // nombres de tours total pour cette production
    int toursrestant; // tours restant pour cette production
    struct unite *usuiv, *uprec; // liste des unites affiliees a une ruche ou un nid
    struct unite *colsuiv, *colprec; // liste des autres ruches ou nids (colonies) du même camp
    struct unite *vsuiv, *vprec; // liste des autres unites sur la meme case
} Unite, *UListe;

// La structure Case :
typedef struct {
    Unite *colonie; // S'il y a une ruche ou un nid sur la case
    UListe occupant; // les autres occupants de la case
} Case;

// La structure Grille :
typedef struct {
    Case plateau[LIGNES][COLONNES];
    UListe abeille, frelon;
    int tour; // Numero du tour
    int ressourcesAbeille, ressourcesFrelon;
} Grille;


int creer_unite(UListe *Liste, char camp, char type, int posx, int posy) {
    //  si on ajoute un nid/ruche
    //      *Liste correpond a l'adresse du camp 
    //  si on ajoute une unite lié a un nid/ruche
    //      *Liste correpond a l'adresse du nid/ruche 

    Unite *unite = (Unite*)malloc(sizeof(Unite)); // allocation
    if (unite == NULL) {
        return -1;  // echec d'allocation
    }

    unite->camp = camp;
    unite->type = type;

    switch (type) {
        case 'r': unite->force = FREINE; break;
        case 'o': unite->force = FOUVRIERE; break;
        case 'g': unite->force = FGUERRIERE; break;
        case 'e': unite->force = FESCADRON; break;
        case 'f': unite->force = FFRELON; break;
        default: unite->force = 0; break;
    }

    unite->posx = posy;
    unite->posy = posx;
    unite->destx = -2;
    unite->desty = -2;
    unite->production = '\0';
    unite->temps = 0;
    unite->toursrestant = 0;

    unite->colsuiv = NULL; // nid/ruche suivante
    unite->colprec = NULL;
    unite->uprec = NULL; // unite suivante
    unite->usuiv = NULL;
    unite->vsuiv = NULL; // unite mm case suivante
    unite->vprec = NULL;

    if ((type == RUCHE || type == NID) && *Liste != NULL) {
        Unite *temp = *Liste;
        while (temp->colsuiv != NULL) {
            temp = temp->colsuiv;
        }
        temp->colsuiv = unite;
        unite->colprec = temp;
    } else if (*Liste != NULL) {
        Unite *temp = *Liste;
        while (temp->usuiv != NULL) {
            temp = temp->usuiv;
        }
        temp->usuiv = unite;
        unite->uprec = temp;
    } else {
        *Liste = unite;
    }

    return 0;
}

int supprimer_unite(Unite *unit, Grille * grille) {
    // adresse de l'unite
    if (unit == NULL) {
        return -1;
    }

    if (unit->type == RUCHE || unit->type == NID) {

        Unite *temp;
        while (unit->usuiv != NULL) {
            temp = unit->usuiv;
            unit->usuiv = temp->usuiv; // Mettre à jour le lien vers la prochaine unité
            free(temp); // Libérer l'unité actuelle
        }

        if (unit->colprec != NULL) {
            unit->colprec->colsuiv = unit->colsuiv;
        }
        if (unit->colsuiv != NULL) {
            unit->colsuiv->colprec = unit->colprec;
        }

        if (grille->abeille == unit) {
            grille->abeille = unit->colsuiv;
        }
        if (grille->frelon == unit) {
            grille->frelon = unit->colsuiv;
        }

    } else {
        if (unit->uprec != NULL) {
            unit->uprec->usuiv = unit->usuiv;
        }
        if (unit->usuiv != NULL) {
            unit->usuiv->uprec = unit->uprec;
        }
    }
    free(unit);
    return 0;
}

int init_grille(Grille * grille) {
    // init valeur
    grille->tour = 1;
    grille->ressourcesAbeille = 10;
    grille->ressourcesFrelon = 10;

    grille->abeille = NULL;
    grille->frelon = NULL;
    
    // init du plateau
    for (int i = 0; i < COLONNES; i++) {
        for (int j = 0; j < LIGNES; j++) {
            Case c = {.colonie = NULL, .occupant = NULL};
            grille->plateau[i][j] = c;
        }
    }
    return 0;
}

int remplir_grille(Grille *grille) {
    // reinitiliser toutes les cases
    for (int i = 0; i < LIGNES; i++) {
        for (int j = 0; j < COLONNES; j++) {
            grille->plateau[i][j].colonie = NULL;
            grille->plateau[i][j].occupant = NULL;
            
        }
    }

    // initialise les insectes sur les memes cases
    UListe temp = grille->abeille;
    UListe temp2;
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            temp2->vsuiv = NULL;
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }
    temp = grille->frelon;
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            temp2->vsuiv = NULL;
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }
    
    // rempli le plateau des valeur des insectes
    Unite * temp_g = grille->abeille;
    Unite * tempo = NULL;
    Unite * temp_v;
    while (temp_g != NULL) {
        tempo = temp_g;
        while (tempo != NULL) { // pour chaque unite du camp abeille on rempli le plateau avec les valeurs des unites abeille 
            if (grille->plateau[tempo->posy][tempo->posx].occupant == NULL) {
                grille->plateau[tempo->posy][tempo->posx].occupant = tempo;
            } else {
                temp_v = grille->plateau[tempo->posy][tempo->posx].occupant;
                while (temp_v->vsuiv != NULL) {
                    temp_v = temp_v->vsuiv;
                }
                temp_v->vsuiv = tempo;
            }
            
            tempo = tempo->usuiv;
        }
        temp_g = temp_g->colsuiv;
    }

    temp_g = grille->frelon;
    while (temp_g != NULL) {
        tempo = temp_g;
        while (tempo != NULL) {
            if (grille->plateau[tempo->posy][tempo->posx].occupant == NULL) {
                grille->plateau[tempo->posy][tempo->posx].occupant = tempo;
            } else {
                temp_v = grille->plateau[tempo->posy][tempo->posx].occupant;
                while (temp_v->vsuiv != NULL) {
                    temp_v = temp_v->vsuiv;
                }
                temp_v->vsuiv = tempo;
                tempo->vprec = temp_v;
            }
            tempo = tempo->usuiv;
        }
        temp_g = temp_g->colsuiv;
    }
    return 0;
}

int dest_ici(Grille grille, int y, int x) {
    Unite * temp_g = grille.abeille;
    Unite * temp = NULL;

    // verifie si une abeille arrive
    while (temp_g != NULL) {
        temp = temp_g;
        while (temp != NULL) {
            if (temp->destx == x && temp->desty == y) {
                return 1; // si une abeille arrive sur la case
            }
            temp = temp->usuiv;
        }
        temp_g = temp_g->colsuiv;
    }
    
    // verifie si un frelon arrive
    temp_g = grille.frelon;
    while (temp_g != NULL) {
        temp = temp_g;
        while (temp != NULL) {
            if (temp->destx == x && temp->desty == y) {
                return 2; // si un frelon arrive sur la case
            }
            temp = temp->usuiv;
        }
        temp_g = temp_g->colsuiv;
    }
    return 0; // si personne n'arrive
}

int deplacement(Unite uni, int new_x, int new_y, Grille grille) {
    if (phase_joueur == 0) { // si phase de production
        return -1;
    }
    if (uni.type == 'r' && uni.toursrestant > 0) { // si la reine est en train de produire
        return -1;
    }
    if (uni.type == 'o' && uni.toursrestant != 0) { // si l'ouvriere est en train de produire
        return -1;
    }
    if (uni.type == 'R' || uni.type == 'N') { // si c'est une ruche ou un nid
        return -1;
    }

    if (-1 <= uni.posy - new_x && uni.posy - new_x <= 1) {
        if (-1 <= uni.posx - new_y && uni.posx - new_y <= 1) {
            return 0; // si la case est a une distance de 1
        }
    }
    return -1; // erreur de deplacement
}

void affichage_graph_reine(Unite unit, int i, int j) {
    if (unit.camp == 'A') {
        MLV_draw_filled_rectangle(10 + i*CASE, 10 + j*CASE, (int) CASE/2, (int) CASE/3, MLV_COLOR_YELLOW);
    } else {
        MLV_draw_filled_rectangle(10 + i*CASE + CASE/2, 10 + j*CASE, (int) CASE/2, (int) CASE/3, MLV_COLOR_RED);
    }
}

void affichage_graph_habitat(Unite unit, int i, int j) {
    if (unit.camp == 'A') {
        MLV_draw_filled_rectangle((int) 10 + i*CASE + CASE/3, (int) 10 + j*CASE + CASE/3, (int) CASE/3, (int) CASE/3, MLV_COLOR_YELLOW);
    } else {
        MLV_draw_filled_rectangle((int) 10 + i*CASE + CASE/3, (int) 10 + j*CASE + CASE/3, (int) CASE/3, (int) CASE/3, MLV_COLOR_RED);
    }
}

void affichage_graph_frelon(int i, int j) {
    MLV_draw_filled_rectangle(10 + i*CASE + 2*CASE/3, 10 + j*CASE + CASE/3, (int) CASE/3, (int) CASE/3, MLV_COLOR_RED);
}

void affichage_graph_ouvriere(int i, int j) {
    MLV_draw_filled_rectangle(10 + i*CASE, 10 + j*CASE + CASE/3, (int) CASE/3, (int) CASE/3, MLV_COLOR_YELLOW);
}

void affichage_graph_guerriere(int i, int j) {
    MLV_draw_filled_rectangle(10 + i*CASE, 10 + j*CASE + 2*CASE/3, (int) CASE/2, (int) CASE/3, MLV_COLOR_YELLOW);
}

void affichage_graph_escadron(int i, int j) {
    MLV_draw_filled_rectangle(10 + i*CASE + CASE/2, 10 + j*CASE + 2*CASE/3, (int) CASE/2, (int) CASE/3, MLV_COLOR_YELLOW);
}

void affichage_graph(Grille *grille) {
    UListe temp = grille->abeille;
    UListe temp2;
    // deplacement des abeilles
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->type == 'R' || temp2->type == 'N') {
                affichage_graph_habitat(*temp2, temp2->posx, temp2->posy);
            }
            if (temp2->type == 'r') {
                affichage_graph_reine(*temp2, temp2->posx, temp2->posy);
            }
            if (temp2->type == 'f') {
                affichage_graph_frelon(temp2->posx, temp2->posy);
            }
            if (temp2->type == 'o') {
                affichage_graph_ouvriere(temp2->posx, temp2->posy);
            }
            if (temp2->type == 'g') {
                affichage_graph_guerriere(temp2->posx, temp2->posy);
            }
            if (temp2->type == 'e') {
                affichage_graph_escadron(temp2->posx, temp2->posy);
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }

    temp = grille->frelon;
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->type == 'R' || temp2->type == 'N') {
                affichage_graph_habitat(*temp2, temp2->posx, temp2->posy);
            }
            if (temp2->type == 'r') {
                affichage_graph_reine(*temp2, temp2->posx, temp2->posy);
            }
            if (temp2->type == 'f') {
                affichage_graph_frelon(temp2->posx, temp2->posy);
            }
            if (temp2->type == 'o') {
                affichage_graph_ouvriere(temp2->posx, temp2->posy);
            }
            if (temp2->type == 'g') {
                affichage_graph_guerriere(temp2->posx, temp2->posy);
            }
            if (temp2->type == 'e') {
                affichage_graph_escadron(temp2->posx, temp2->posy);
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }


    for (int i = 0; i < LIGNES; i++) {
        for (int j = 0; j < COLONNES; j++) {
            MLV_draw_rectangle(10 + j*CASE, 10 + i*CASE, CASE, CASE, MLV_COLOR_BLUE);
        }
    }
    
    if (joueur_actu == 'A') {
        MLV_draw_text(CASE*20, CASE, "ABEILLE", MLV_COLOR_YELLOW);
    } else {
        MLV_draw_text(CASE*20, CASE, "FRELON", MLV_COLOR_RED);
    }
    
    if (phase_joueur == 0) {
        MLV_draw_text(CASE*20, CASE*1.5, "PHASE DE COMMANDE DE PRODUCTION", MLV_COLOR_BLUE);
    } else {
        MLV_draw_text(CASE*20, CASE*1.5, "PHASE DE DEPLACEMENT", MLV_COLOR_BLUE);
    }
    
    MLV_draw_text(CASE*18, CASE*3, "Unité selectionnée : ", MLV_COLOR_BLUE);
    MLV_draw_text(CASE*18, CASE*3.5, "Position : ", MLV_COLOR_BLUE);
    MLV_draw_text(CASE*18, CASE*5, "Se deplace en  : ", MLV_COLOR_BLUE);

    MLV_draw_text(CASE*27, CASE*3, "Tour n°%d", MLV_COLOR_BLUE, grille->tour);
    MLV_draw_text(CASE*27, CASE*4, "Ressources abeille : %d", MLV_COLOR_BLUE, grille->ressourcesAbeille);
    MLV_draw_text(CASE*27, CASE*4.5, "Resssources Frelon : %d", MLV_COLOR_BLUE, grille->ressourcesFrelon);

    MLV_Color color = MLV_COLOR_RED;
    if (phase_joueur >= 1) {
        color = MLV_COLOR_ORANGE;
    } else {
        color = MLV_COLOR_RED;
    }
    MLV_draw_text_box(
		CASE*17, CASE*12,
		CASE*4, CASE*2,
		"Fin de commande\nde production",
		10,
		color, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_CENTER,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);

    MLV_draw_text_box(
		CASE*22, CASE*12,
		CASE*4, CASE*2,
		"Fin du tour",
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_LEFT,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);

    MLV_draw_text_box(
		CASE*17, CASE*15,
		CASE*4, CASE*2,
		"Sauver",
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_LEFT,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);
    MLV_draw_text_box(
		CASE*22, CASE*15,
		CASE*4, CASE*2,
		"Charger",
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_LEFT,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);
    MLV_draw_text_box(
		CASE*27, CASE*15,
		CASE*4, CASE*2,
		"Quitter",
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_LEFT,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);

    MLV_update_window();
}

void affichage_selection_case(Grille grille, Case * selection_case, int selection_x, int selection_y, int butt, Unite *selection) {
    if (selection_case == NULL) return;
    MLV_draw_rectangle(10 + selection_y*CASE, 10 + selection_x*CASE, CASE, CASE, MLV_COLOR_ORANGE);

    int i = 1;
    MLV_Color color = MLV_COLOR_RED;

    Unite * temp = grille.plateau[selection_x][selection_y].occupant;

    while (temp != NULL) {
        if (butt == i + 8 && selection != NULL) {
            color = MLV_COLOR_ORANGE;
        } else {
            color = MLV_COLOR_RED;
        }
        char type[10];
        if (temp->type == 'r') {
            strcpy(type, "reine");
        } else if (temp->type == 'o') {
            strcpy(type, "ouvriere");
        } else if (temp->type == 'e') {
            strcpy(type, "escadron");
        } else if (temp->type == 'f') {
            strcpy(type, "frelon");
        } else if (temp->type == 'g') {
            strcpy(type, "guerriere");
        } else if (temp->type == 'R') {
            strcpy(type, "ruche");
        } else if (temp->type == 'N') {
            strcpy(type, "nid");
        }
        MLV_draw_text_box(
            CASE*13, CASE*1*i,
            CASE*3, CASE*1,
            "%s",
            10,
            color, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, type
        );
        i++;
        temp = temp->vsuiv;
    }

    MLV_update_window();
}

int obtenir_pos_unite_butt(Grille grille, int butt, int selection_x, int selection_y, Unite ** adr_select) {
    int count = 9;
    
    Unite * temp = grille.plateau[selection_x][selection_y].occupant;
    while (temp != NULL) {
        if (count == butt) {
            *adr_select = temp;
            return 0;
        }
        count++;
        temp = temp->vsuiv;
    }
    return -1;
}

void affichage_selection(Unite * selection) {
    if (selection == NULL) return;
    MLV_draw_rectangle(10 + selection->posx*CASE, 10 + selection->posy*CASE, CASE, CASE, MLV_COLOR_ORANGE);
    MLV_draw_rectangle(10 + selection->destx*CASE, 10 + selection->desty*CASE, CASE, CASE, MLV_COLOR_GREEN);
    MLV_draw_text(CASE*20, CASE*3.5, "%d, %d", MLV_COLOR_WHITE, selection->posx, selection->posy);
    if (selection->destx >= 0) {
        MLV_draw_text(CASE*21, CASE*5, "%d, %d", MLV_COLOR_WHITE, selection->destx, selection->desty);
    }
    char type[10];
    if (selection->type == 'r') {
        strcpy(type, "reine");
    } else if (selection->type == 'o') {
        strcpy(type, "ouvriere");
    } else if (selection->type == 'e') {
        strcpy(type, "escadron");
    } else if (selection->type == 'f') {
        strcpy(type, "frelon");
    } else if (selection->type == 'g') {
        strcpy(type, "guerriere");
    } else if (selection->type == 'R') {
        strcpy(type, "ruche");
    } else if (selection->type == 'N') {
        strcpy(type, "nid");
    }
    MLV_draw_text(CASE*21, CASE*3, type, MLV_COLOR_WHITE);

    if (selection->production == 'r') {
        MLV_draw_text_box(
            CASE*17, CASE*9.25,
            CASE*3, CASE*1,
            "En cours\nTours restants: %d",
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
        );
    } else if (selection->production == 'o') {
        MLV_draw_text_box(
            CASE*20.66, CASE*9.25,
            CASE*3, CASE*1,
            "En cours\nTours restants: %d",
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
        );
    } else if (selection->production == 'g') {
        MLV_draw_text_box(
            CASE*24.33, CASE*9.25,
            CASE*3, CASE*1,
            "En cours\nTours restants: %d",
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
        );
    } else if (selection->production == 'e') {
        MLV_draw_text_box(
            CASE*28, CASE*9.25,
            CASE*3, CASE*1,
            "En cours\nTours restants: %d",
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
        );
    } else if (selection->production == 'f') {
        MLV_draw_text_box(
            CASE*28, CASE*9.25,
            CASE*3, CASE*1,
            "En cours\nTours restants: %d",
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
        );
    } else if (selection->production == 'R' || selection->production == 'N') {
        if (selection->toursrestant > 0) {
            MLV_draw_text_box(
                CASE*17, CASE*9.25,
                CASE*3, CASE*1,
                "En cours\nTours restants: %d",
                10,
                MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
                MLV_TEXT_CENTER,
                MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
            );
        } else {
            MLV_draw_text_box(
                CASE*17, CASE*9.25,
                CASE*3, CASE*1,
                "Création de ruche/nid\ndéjà effectué",
                10,
                MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
                MLV_TEXT_CENTER,
                MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
            );
        }
        
    } else if (selection->production == 'p') {
        MLV_draw_text_box(
            CASE*17, CASE*9.25,
            CASE*3, CASE*1,
            "En cours\nTours restants: %d",
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, selection->toursrestant
        );
    }
    
    MLV_update_window();
}

int coo_to_case(int *x, int *y) {
    if (10 <= *x && *x < LIGNES*CASE+10) {
        if (10 <= *y && *y < COLONNES*CASE+10) {
            *x = (*x-10)/CASE;
            *y = (*y-10)/CASE;
            return 0;
        }
    }
    return -1;
}

int coo_to_button(int *butt, int x, int y) {
    if (CASE*22 < x && x < CASE*26 && CASE*12 < y && y < CASE*14) {
        *butt = 0; // fin du tour
        return 0;
    } else if (CASE*17 < x && x < CASE*21 && CASE*15 < y && y < CASE*17) {
        *butt = 1; // sauver
        return 0;
    } else if (CASE*22 < x && x < CASE*26 && CASE*15 < y && y < CASE*17) {
        *butt = 2; // charger
        return 0;
    } else if (CASE*27 < x && x < CASE*31 && CASE*15 < y && y < CASE*17) {
        *butt = 3; // quitter
        return 0;
    } else if (CASE*17 < x && x < CASE*20 && CASE*7 < y && y < CASE*9) {
        *butt = 4; // ruche produit reine
        return 0;
    } else if (CASE*20.66 < x && x < CASE*23.66 && CASE*7 < y && y < CASE*9) {
        *butt = 5; // ruche produit ouvriere
        return 0;
    } else if (CASE*24.33 < x && x < CASE*27.33 && CASE*7 < y && y < CASE*9) {
        *butt = 6; // ruche produit guerriere
        return 0;
    } else if (CASE*28 < x && x < CASE*31 && CASE*7 < y && y < CASE*9) {
        *butt = 7; // ruche produit escadron
        return 0;
    } else if (CASE*17 < x && x < CASE*21 && CASE*12 < y && y < CASE*14) {
        *butt = 8; // fin du commande de production
        return 0;
    } else {
        for (int i = 1; i < 15; i++) {
            if (CASE*13 < x && x < CASE*16 && CASE*i < y && y < CASE*i+CASE) {
                *butt = i + 8;
                return 0;
            }
        }
        *butt = -1;
        return -1;
    }    
}

void affichage_joueur(Grille grille) {
    UListe temp;
    if (joueur_actu == 'A') {
        temp = grille.abeille;
    } else {
        temp = grille.frelon;
    }

    while (temp != NULL) {
        UListe temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->destx >= 0) {
                MLV_draw_rectangle(10 + temp2->posx*CASE, 10 + temp2->posy*CASE, CASE, CASE, MLV_COLOR_DARK_GRAY);
                MLV_draw_rectangle(10 + temp2->destx*CASE, 10 + temp2->desty*CASE, CASE, CASE, MLV_COLOR_GREEN);
            } else {
                MLV_draw_rectangle(10 + temp2->posx*CASE, 10 + temp2->posy*CASE, CASE, CASE, MLV_COLOR_WHITE);
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }
    
    MLV_update_window();
}

int actualise_deplacement(Grille * grille) {
    UListe temp = grille->abeille;
    UListe temp2;
    // deplacement des abeilles
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->destx >= 0) {
                temp2->posx = temp2->destx;
                temp2->posy = temp2->desty;
                temp2->destx = -2;
                temp2->desty = -2;
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }

    // deplacement des frelons
    temp = grille->frelon;
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->destx >= 0) {
                temp2->posx = temp2->destx;
                temp2->posy = temp2->desty;
                temp2->destx = -2;
                temp2->desty = -2;
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }
    return 0;
}

void affichage_bouton_ruche(Unite *selection) {
    int cout;
    int temps;
    if (selection->camp == 'A') {
        cout = CREINEA;
        temps = TREINEA;
    } else {
        cout = CREINEF;
        temps = TREINEF;
    }
    MLV_draw_text_box(
		CASE*17, CASE*7,
		CASE*3, CASE*2,
		"Produire Reine\nCoût : %d\nTemps : %d", 
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_CENTER,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, cout, temps
	);

    if (selection->camp == 'A') {
        MLV_draw_text_box(
            CASE*20.66, CASE*7,
            CASE*3, CASE*2,
            "Produire Ouvrière\nCoût : %d\nTemps : %d", 
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, COUVRIERE, TOUVRIERE
        );

        MLV_draw_text_box(
            CASE*24.33, CASE*7,
            CASE*3, CASE*2,
            "Produire Guerrière\nCoût : %d\nTemps : %d", 
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, CGUERRIERE, TGUERRIERE
        );

        MLV_draw_text_box(
            CASE*28, CASE*7,
            CASE*3, CASE*2,
            "Produire Escadron\nCoût : %d\nTemps : %d", 
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, CESCADRON, TESCADRON
        );

    } else {
        MLV_draw_text_box(
            CASE*28, CASE*7,
            CASE*3, CASE*2,
            "Produire Frelon\nCoût : %d\nTemps : %d", 
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, CFRELON, TFRELON
        );
    }
    
    MLV_update_window();
    return;
}

void affichage_bouton_reine(Unite *selection) {
    if (selection->camp == 'A') {
        MLV_draw_text_box(
            CASE*17, CASE*7,
		    CASE*3, CASE*2,
            "Produire Ruche\nCoût : %d\nTemps : %d", 
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, CRUCHE, TRUCHE
        );
    } else {
        MLV_draw_text_box(
            CASE*17, CASE*7,
		    CASE*3, CASE*2,
            "Produire Nid\nCoût : %d\nTemps : %d", 
            10,
            MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
            MLV_TEXT_CENTER,
            MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, CNID, TNID
        );
    }
       
    MLV_update_window();
    return;
}

void affichage_bouton_ouvriere(Unite *selection) {
    MLV_draw_text_box(
        CASE*17, CASE*7,
        CASE*3, CASE*2,
        "Recolte pollen\nTemps : %d", 
        10,
        MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
        MLV_TEXT_CENTER,
        MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER, TRECOLTE
    );
    
    MLV_update_window();
    return;
}

int production_reine(Unite * selection, int butt, Grille *grille) {
    if (butt == 4) {
        if (selection->camp == 'A' && grille->ressourcesAbeille >= CRUCHE) {  // si abeille
            selection->production = 'R';
            selection->temps = TRUCHE;
            selection->toursrestant = TRUCHE;
            (*grille).ressourcesAbeille -= CRUCHE;
        } else if (selection->camp == 'F' && grille->ressourcesFrelon >= CNID){ // si frelon
            selection->production = 'R';
            selection->temps = TRUCHE;
            selection->toursrestant = TRUCHE;
            (*grille).ressourcesFrelon -= CNID;
        }
    }
    return 0;
}

int production_recolte(Unite * selection, int butt) {
    if (butt == 4) {
        selection->production = 'p';
        selection->temps = TRECOLTE;
        selection->toursrestant = TRECOLTE;
    }
    return 0;
}

int production_ruche(Unite * selection, int butt, Grille *grille) {
    if (butt == 4 && grille->ressourcesAbeille >= CREINEA) {
        selection->production = 'r';
        selection->temps = TREINEA;
        selection->toursrestant = TREINEA;
        (*grille).ressourcesAbeille -= CREINEA;
    } else if (butt == 5 && grille->ressourcesAbeille >= COUVRIERE) {
        selection->production = 'o';
        selection->temps = TOUVRIERE;
        selection->toursrestant = TOUVRIERE;
        (*grille).ressourcesAbeille -= COUVRIERE;
    } else if (butt == 6 && grille->ressourcesAbeille >= CGUERRIERE) {
        selection->production = 'g';
        selection->temps = TGUERRIERE;
        selection->toursrestant = TGUERRIERE;
        (*grille).ressourcesAbeille -= CGUERRIERE;
    } else if (butt == 7 && grille->ressourcesAbeille >= CESCADRON) {
        selection->production = 'e';
        selection->temps = TESCADRON;
        selection->toursrestant = TESCADRON;
        (*grille).ressourcesAbeille -= CESCADRON;
    }
    return 0;
}

int production_nid(Unite * selection, int butt, Grille * grille) {
    if (butt == 4 && grille->ressourcesFrelon >= CREINEF) {
        selection->production = 'r';
        selection->temps = CREINEF;
        selection->toursrestant = CREINEF;
        (*grille).ressourcesFrelon -= CREINEF;
    } else if (butt == 7 && grille->ressourcesFrelon >= CFRELON) {
        selection->production = 'f';
        selection->temps = CFRELON;
        selection->toursrestant = CFRELON;
        (*grille).ressourcesFrelon -= CFRELON;
    }
    return 0;
}

int place_dispo(Grille grille, int x, int y) {
    Unite * temp_g = grille.abeille;
    Unite * temp = NULL;

    while (temp_g != NULL) {
        temp = temp_g;
        while (temp != NULL) {
            if (temp->posx == x && temp->posy == y && (temp->type == 'R' || temp->type == 'N')) {
                return -1; // si pas de nid/ruche a cet emplacement
            }
            temp = temp->usuiv;
        }
        temp_g = temp_g->colsuiv;
    }
    
    temp_g = grille.frelon;
    while (temp_g != NULL) {
        temp = temp_g;
        while (temp != NULL) {
            if (temp->posx == x && temp->posy == y && (temp->type == 'R' || temp->type == 'N')) {
                return -1; // si pas de nid/ruche a cet emplacement
            }
            temp = temp->usuiv;
        }
        temp_g = temp_g->colsuiv;
    }
    
    return 1;
}

int trouve_place_production(UListe productrice, Grille grille, int *x, int *y) {
    int posx = productrice->posx;
    int posy = productrice->posy;

    int directions[9][2] = {{0, 0}, {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {1, 1}, {-1, -1}, {-1, 1}, {1, -1}};

    int newx;
    int newy;
    for (int i = 0; i < 9; i++) {
        newx = posx + directions[i][0];
        newy = posy + directions[i][1];

        // Vérifiez si la nouvelle position est à l'intérieur des limites du tableau
        if (newx >= 0 && newx < COLONNES && newy >= 0 && newy < LIGNES) {
            if (place_dispo(grille, newx, newy) == 1) {
                *x = newx;
                *y = newy;
                return 1;
            }
        }
    }
    return -1;

}

int cout_unite_ruche(UListe ruche) {
    int count = 0;
    UListe temp = ruche;
    while (temp != NULL) {
        if (temp->type == 'r') {
            count += CREINEA;
        } else if (temp->type == 'o') {
            count += COUVRIERE;
        } else if (temp->type == 'g') {
            count += CGUERRIERE;
        } else if (temp->type == 'e') {
            count += CESCADRON;
        }
        temp = temp->usuiv;
    }
    return count;
}

int combat_fini(Case * c) {
    int pres_ab = 0; // presence de frelon
    int pres_fr = 0; // presence d'abeille
    UListe temp = c->occupant;

    while (temp != NULL) {
        if (temp->camp == 'A') {
            pres_ab++;
        } else if (temp->camp == 'F') {
            pres_fr++;
        }
        temp = temp->vsuiv;
    }
    if (pres_ab != 0 && pres_fr != 0) { // si presence des deux camps alors le combat n'est pas fini
        return 0;
    }
    return 1;
}

int combat(Case * c, Grille * grille) {
    UListe temp = c->occupant;
    Unite * combattantA;
    Unite * combattantF;

    remplir_grille(grille);

    while (combat_fini(c) == 0) {
        
        combattantA = NULL;
        combattantF = NULL;

        temp = c->occupant;
        while (temp != NULL) { // cherche ruche
            if (temp->type == 'R') {
                combattantA = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche ouvriere
            if (temp->type == 'o') {
                combattantA = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche reine
            if (temp->camp == 'A' && temp->type == 'r') {
                combattantA = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche guerriere
            if (temp->type == 'g') {
                combattantA = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche escadron
            if (temp->type == 'e') {
                combattantA = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche nid
            if (temp->type == 'N') {
                combattantF = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche frelon
            if (temp->type == 'f') {
                combattantF = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        temp = c->occupant;
        while (temp != NULL) { // cherche reine frelon
            if (temp->camp == 'F' && temp->type == 'r') {
                combattantF = temp;
                break;
            }
            temp = temp->vsuiv;
        }
        
        int tirageUniteA;
        int tirageUniteF;
        do {
            tirageUniteA = MLV_get_random_integer(1, 60) * combattantA->force;
            tirageUniteF = MLV_get_random_integer(1, 60) * combattantF->force;
        } while (tirageUniteA == tirageUniteF);
        Unite * perdant;
        Unite * gagnant;

        if (tirageUniteA < tirageUniteF) {
            perdant = combattantA;
            gagnant = combattantF;
        } else {
            perdant = combattantF;
            gagnant = combattantA;
        }

        if (perdant->type == 'R') {
            // augmenter les ress des frelons
            grille->ressourcesFrelon += cout_unite_ruche(perdant);
            
            // creer nid
            creer_unite(&grille->frelon, CAFRELON, NID, perdant->posy, perdant->posx);
            
            // lié le frelon au nid
            UListe dernier = grille->frelon;
            while (dernier->colsuiv != NULL) {
                dernier = dernier->colsuiv;
            }

            creer_unite(&dernier, CAFRELON, gagnant->type, gagnant->posy, gagnant->posx);
            supprimer_unite(gagnant, grille);
            // detruire ruche
            supprimer_unite(perdant, grille);
        
        } else if (perdant->type == 'N') {
            // creer ruche
            creer_unite(&grille->abeille, CAABEILLE, RUCHE, perdant->posy, perdant->posx);
            
            // lié l'abeille a la ruche
            UListe dernier = grille->abeille;
            while (dernier->colsuiv != NULL) {
                dernier = dernier->colsuiv;
            }
            
            creer_unite(&dernier, CAABEILLE, gagnant->type, gagnant->posy, gagnant->posx);
            supprimer_unite(gagnant, grille);
            // detruire nid
            supprimer_unite(perdant, grille);


        } else {
            // augmenter les ress des frelons
            if (perdant->type == 'r' && perdant->camp == 'A') {
                grille->ressourcesFrelon += CREINEA;
            } else if (perdant->type == 'o') {
                grille->ressourcesFrelon += COUVRIERE;
            } else if (perdant->type == 'g') {
                grille->ressourcesFrelon += CGUERRIERE;
            } else if (perdant->type == 'e') {
                grille->ressourcesFrelon += CESCADRON;
            }
            // detruire unité
            supprimer_unite(perdant, grille);
        }
        remplir_grille(grille);

    }
    return 0;
}

int resolution_combat(Grille * grille) {
    UListe temp = grille->abeille;
    UListe temp2;
    UListe temp_f;
    UListe temp2_f;

    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            temp_f = grille->frelon;
            while (temp_f != NULL) {
                temp2_f = temp_f;
                while (temp2_f != NULL) {
                    if (temp2->posx == temp2_f->posx) {
                        if (temp2->posy == temp2_f->posy) {
                            // si deux unités adverses sur la même case
                            combat(&grille->plateau[temp2->posy][temp2->posx], grille);
                            return 1;
                        }
                    }
                    temp2_f = temp2_f->usuiv;
                }
                temp_f = temp_f->colsuiv;
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }

    return 0;
}

int deploiement_produit(UListe productrice, char type, Grille grille) {
    int x;
    int y;
    if (trouve_place_production(productrice, grille, &x, &y) == 1) {
        if (type == 'R') {
            creer_unite(&grille.abeille, productrice->camp, type, y, x);
        } else {
            creer_unite(&grille.frelon, productrice->camp, type, y, x);
        }
        return 0;
    }
    return -1;
}

int decremente_prod(Grille * grille) {
    
    UListe temp = grille->abeille;
    UListe temp2;
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->production != '\0') {
                temp2->toursrestant--;
                if (temp2->type == 'o') {
                    grille->ressourcesAbeille++;
                }
                if (temp2->toursrestant == 0) {
                    if (temp2->type == 'o') { // si ouvriere
                        supprimer_unite(temp2, grille);
                    } else if (temp2->type == 'r') { // si reine
                        if (deploiement_produit(temp2, 'R', *grille) == 0) { // creation ruche
                            UListe dernier = grille->abeille;
                            while (dernier->colsuiv != NULL) {
                                dernier = dernier->colsuiv;
                            }
                            creer_unite(&dernier, temp2->camp, temp2->type, temp2->posy, temp2->posx);
                            UListe tempp = dernier;
                            while (tempp->usuiv != NULL) {
                                tempp = tempp->usuiv;
                            }
                            tempp->production = 'R'; // elle ne peut plus en creer
                            supprimer_unite(temp2, grille);
                        } else { // si pas de deploiement possibles, on repporte au prochain tour
                            temp2->production = 1;
                        }
                        
                    } else if (creer_unite(&temp, temp2->camp, temp2->production, temp2->posx, temp2->posy) == 0) {
                        if (temp2->type == 'R' || temp2->type == 'N') { // si creation par ruche/nid
                            temp2->production = '\0';
                            temp2->temps = 0;
                            temp2->toursrestant = 0;
                        }
                    } else {
                        temp2->toursrestant = 1;
                    }   
                }
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }

    temp = grille->frelon;
    while (temp != NULL) {
        temp2 = temp;
        while (temp2 != NULL) {
            if (temp2->production != '\0') {
                temp2->toursrestant--;
                if (temp2->toursrestant == 0) {
                    if (temp2->type == 'r') { // si reine
                        if (deploiement_produit(temp2, 'N', *grille) == 0) { // cherche place pour creation nid

                            UListe dernier = grille->frelon;
                            while (dernier->colsuiv != NULL) {
                                dernier = dernier->colsuiv;
                            }
                            creer_unite(&dernier, temp2->camp, temp2->type, temp2->posy, temp2->posx);
                            UListe tempp = dernier;
                            while (tempp->usuiv != NULL) {
                                tempp = tempp->usuiv;
                            }
                            tempp->production = 'R'; // elle ne peut plus en creer

                            supprimer_unite(temp2, grille);
                        } else {
                            // si pas de place on repporte au prochain tour
                            temp2->production = 1;
                        }

                    } else if (creer_unite(&temp, temp->camp, temp2->production, temp2->posy, temp2->posx) == 0) {
                        if (temp2->type == 'R' || temp2->type == 'N') {  // si creation par ruche/nid
                            temp2->production = '\0';
                            temp2->temps = 0;
                            temp2->toursrestant = 0;
                        }
                    } else {
                        temp2->toursrestant = 1;
                    }
                }
            }
            temp2 = temp2->usuiv;
        }
        temp = temp->colsuiv;
    }
    return 0;
}

int verif_win(Grille *grille) {
    if (grille->abeille == NULL) {
        return 1;
    }
    if (grille->frelon == NULL) {
        return 2;
    }
    // si il n'y a aucun camps sur la grille 
    if (grille->abeille == NULL && grille->frelon == NULL) {
        return 3;
    }
    return -1;
}

void affichage_win(int win) {
    if (win == 2) {
        MLV_draw_text(CASE*16, CASE*10, "Victoire des Abeilles", MLV_COLOR_YELLOW);
    } else {
        MLV_draw_text(CASE*16, CASE*10, "Victoire des Frelons", MLV_COLOR_RED);
    }

    MLV_draw_text_box(
		CASE*10, CASE*12,
		CASE*5, CASE*2,
		"Rejouer",
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_LEFT,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);

    MLV_draw_text_box(
		CASE*20, CASE*12,
		CASE*5, CASE*2,
		"Quitter",
		10,
		MLV_COLOR_RED, MLV_COLOR_GREEN, MLV_COLOR_BLACK,
		MLV_TEXT_LEFT,
		MLV_HORIZONTAL_CENTER, MLV_VERTICAL_CENTER
	);
    MLV_update_window();
}


void sauvegarderDonnees(FILE *fichier, Grille *grille) {
    if (fichier == NULL || grille == NULL) {
        fprintf(stderr, "Erreur : Paramètres invalides\n");
        return;
    }

    // En-tête de fichier : joueur actuel, ressources, tour actuel
    fprintf(fichier, "%c %d %d %d\n", joueur_actu, grille->ressourcesAbeille, grille->ressourcesFrelon, grille->tour);

    // Sauvegarde des données des abeilles en parcourant toutes les ruches et en sauvegardant les unités liées 
    UListe abeille = grille->abeille;
    while (abeille != NULL) { 
        UListe temp = abeille;
        while (temp != NULL) {
            fprintf(fichier, "%c %c %d %d %d \n", temp->camp, temp->type, temp->force, temp->posy, temp->posx); 
            temp = temp->usuiv;
        }
        abeille = abeille->colsuiv;
    }

    // Sauvegarde des données des frelons
    UListe frelon = grille->frelon;
    while (frelon != NULL) {
        UListe temp = frelon;
        while (temp != NULL) {
            fprintf(fichier, "%c %c %d %d %d \n", temp->camp, temp->type, temp->force, temp->posx, temp->posy);
            temp = temp->usuiv;
        }
        frelon = frelon->colsuiv;
    }
}

int supprimer_liste(UListe *liste) {
    if (liste == NULL) {
        fprintf(stderr, "Erreur : Paramètres invalides\n");
        return -1;
    }

    UListe temp = *liste;
    while (temp != NULL) {
        UListe temp2 = temp;
        temp = temp->colsuiv;
        while (temp2 != NULL) {
            UListe temp3 = temp2;
            temp2 = temp2->usuiv;
            free(temp3);
        }
    }
    *liste = NULL;
    return 0;
}

// Fonction de chargement du jeu depuis un fichier de sauvegarde et de création d'une nouvelle grille de jeu à partir de celui-ci 
int chargerPartie(FILE *fichier, Grille *grille) {
    if (fichier == NULL || grille == NULL) {
        fprintf(stderr, "Erreur : Paramètres invalides\n");
        return -1;
    }

    // Suppression de la grille actuelle
    supprimer_liste(&grille->abeille);
    supprimer_liste(&grille->frelon);

    // Lecture de l'en-tête du fichier
    char joueur;
    int ressourcesAbeille;
    int ressourcesFrelon;
    int tour;
    fscanf(fichier, "%c %d %d %d\n", &joueur, &ressourcesAbeille, &ressourcesFrelon, &tour);

    // Création de la nouvelle grille
    grille->tour = tour;
    grille->ressourcesAbeille = ressourcesAbeille;
    grille->ressourcesFrelon = ressourcesFrelon;
    if (joueur == 'A') {
        joueur_actu = 'A';
    } else {
        joueur_actu = 'F';
    }

    // Lecture des données des abeilles et création des unités correspondantes en prenant compte des ruches et des nids 
    char camp;
    char type;
    int force;
    int posx;
    int posy;
    while (fscanf(fichier, "%c %c %d %d %d\n", &camp, &type, &force, &posy, &posx) != EOF) {
        if (camp == 'A') {
            if (type == 'R') {
                creer_unite(&grille->abeille, camp, type, posy, posx);
            } else {
                creer_unite(&grille->abeille, camp, type, posy, posx);
            }
        } else {
            if (type == 'N') {
                creer_unite(&grille->frelon, camp, type, posx, posy);
            } else {
                creer_unite(&grille->frelon, camp, type, posx, posy);
            }
        }
    }
    return 0;
}


int main(void) {
    MLV_create_window("AvsF", "AvsF", 1920, 1080);

    int x;
    int y;
    Unite * selection;
    Case * selection_case;
    int selection_x;
    int selection_y;
    int win = -1;

    while (1) {
        int butt = -1;

        Grille * grille = malloc(sizeof(Grille));
        init_grille(grille);

        creer_unite(&grille->abeille, CAABEILLE, RUCHE, 0, 0);
        creer_unite(&grille->abeille, CAABEILLE, REINE, 0, 0);
        creer_unite(&grille->abeille, CAABEILLE, OUVRIERE, 0, 0);
        creer_unite(&grille->abeille, CAABEILLE, GUERRIERE, 0, 0);
        
        creer_unite(&grille->frelon, CAFRELON, NID, 17, 11);
        creer_unite(&grille->frelon, CAFRELON, REINE, 17, 11);
        creer_unite(&grille->frelon, CAFRELON, FRELON, 17, 11);
        creer_unite(&grille->frelon, CAFRELON, FRELON, 17, 11);

        remplir_grille(grille);

        int num = MLV_get_random_integer(1, 2);
        if (num == 1) {
            joueur_actu = 'A';
        } else {
            joueur_actu = 'F';
        }
                
        if (win == -1) {
            MLV_clear_window(MLV_COLOR_BLACK);
            affichage_graph(grille);
            affichage_joueur(*grille);
        }
        
        selection = NULL;
        selection_case = NULL;
        selection_x = -2;
        selection_y = -2;

        phase_tour = 0; // phase 0 ou 1 pour chaque joueur par tour
        phase_joueur = 0; // phase de production  ou de deplacement

        while (win == -1) {
            MLV_wait_mouse(&y, &x);

            if (coo_to_case(&x, &y) == 0) { // si clique sur le terrain
                if (selection_case != NULL) {
                    if (selection_case == &(grille->plateau[x][y])) {
                        selection_case = NULL;
                        selection = NULL;
                        selection_x = -2;
                        selection_y = -2;
                    }
                } else if (selection_case == NULL && grille->plateau[x][y].occupant != NULL) {
                    if ((grille->plateau[x][y]).occupant->camp == joueur_actu) {
                        selection_case = &(grille->plateau[x][y]);
                        selection_x = x;
                        selection_y = y;
                    }
                }

                if (selection != NULL) {
                    if (selection->destx == y && selection->desty == x) {
                        selection->destx = -2;
                        selection->desty = -2;
                    } else if (selection->posx != y || selection->posy != x) {
                        if (deplacement(*selection, x, y, *grille) == 0) {
                            selection->destx = y;
                            selection->desty = x;
                        }
                    }
                }

            } else { // si clique en dehors du terrain
                coo_to_button(&butt, y, x);
                if (selection_case != NULL) {
                    if (9 <= butt && butt <= 22) {
                        Unite * adr_selec = NULL;
                        if (obtenir_pos_unite_butt(*grille, butt, selection_x, selection_y, &adr_selec) == 0) {
                            if (selection == adr_selec) {
                                selection = NULL;
                            } else {
                                selection = adr_selec;
                            }

                        };
                    }
                }
                
                if (selection != NULL && phase_joueur == 0) { // si ordre de production
                    if (selection->type == 'R') {
                        if (4 <= butt && butt <= 7) {
                            if (selection->production == '\0') {
                                production_ruche(selection, butt, grille);
                            }
                        }
                    }
                    if (selection->type == 'r') {
                        if (4 == butt) {
                            if (selection->production == '\0' && selection->destx == -2) {
                                production_reine(selection, butt, grille);
                            }
                        }
                    }
                    if (selection->type == 'o') {
                        if (4 == butt) {
                            if (selection->production == '\0' && selection->destx == -2) {
                                production_recolte(selection, butt);
                            }
                        }
                    }
                    if (selection->type == 'N') {
                        if (4 == butt || butt == 7) {
                            if (selection->production == '\0') {
                                production_nid(selection, butt, grille);
                            }
                        }
                    }
                }

                if (butt == 3) {
                    selection = NULL;
                    break;
                }
                
                if (butt == 1) {
                    // ici pour sauvegarder
                    FILE *fichier = fopen("sauvegarde.txt", "w");
                    sauvegarderDonnees(fichier, grille);
                    fclose(fichier);
                }
                if (butt == 2) {
                    // ici pour charger
                    chargerPartie(fopen("sauvegarde.txt", "r"), grille);

                }

                if (butt == 8) {
                    phase_joueur++;
                }

                if (butt == 0) {
                    if (phase_tour == 1) {
                        actualise_deplacement(grille);
                        int res;
                        do {
                            res = resolution_combat(grille);
                        } while (res != 0);
                        decremente_prod(grille);

                        num = MLV_get_random_integer(1, 2);
                        if (num == 1) {
                            joueur_actu = 'A';
                        } else {
                            joueur_actu = 'F';
                        }
                        grille->tour++;
                        
                    }
                    phase_joueur = 0;
                    phase_tour++;
                    phase_tour = phase_tour%2;

                    if (joueur_actu == 'A') {
                        joueur_actu = 'F';
                    } else {
                        joueur_actu = 'A';
                    }
                    selection = NULL;
                    selection_case = NULL;
                }
            }
            
            MLV_clear_window(MLV_COLOR_BLACK);
            if (selection != NULL) {
                if (selection->type == 'R' || selection->type == 'N') {
                    affichage_bouton_ruche(selection);
                }
                if (selection->type == 'r') {
                    affichage_bouton_reine(selection);
                }
                if (selection->type == 'o') {
                    affichage_bouton_ouvriere(selection);
                }
            }

            win = verif_win(grille);

            remplir_grille(grille);

            affichage_graph(grille);
            affichage_joueur(*grille);
            affichage_selection(selection);
            affichage_selection_case(*grille, selection_case, selection_x, selection_y, butt, selection);
            
            
        }
        if (butt == 3) {
            selection = NULL;
            break;
        }

        MLV_clear_window(MLV_COLOR_BLACK);
        affichage_win(win);

        MLV_wait_mouse(&y, &x);

        if (CASE*12 < x && x < CASE*14 && CASE*10 < y && y < CASE*15) {
            win = -1; // rejouer
        }
        if (CASE*10 < x && x < CASE*14 && CASE*20 < y && y < CASE*25) {
            MLV_free_window();
            return 0; // quitter
        }
    }
    
    MLV_free_window();
    return 0;
}