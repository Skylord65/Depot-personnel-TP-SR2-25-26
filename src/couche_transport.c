#include <stdio.h>
#include "couche_transport.h"
#include "services_reseau.h"
#include "application.h"

/* ************************************************************************** */
/* *************** Fonctions utilitaires couche transport ******************* */
/* ************************************************************************** */

// RAJOUTER VOS FONCTIONS DANS CE FICHIER...

int generer_controle(paquet_t paquet){
    unsigned char somme_ctrl = paquet.lg_info ^ paquet.num_seq ^ paquet.type;
    int i = 0;
    while (i<paquet.lg_info)
    {
        somme_ctrl ^= paquet.info[i]; 
        i++;
    }
    
    return somme_ctrl;
}

int verifier_controle(paquet_t paquet){
    return paquet.somme_ctrl == generer_controle(paquet);
}

int inc(int *paquet_num, int modulo){
    *paquet_num = ((*paquet_num)+1)%modulo;
    return *paquet_num;
}

/*--------------------------------------*/
/* Fonction d'inclusion dans la fenetre */
/*--------------------------------------*/
int dans_fenetre(unsigned int inf, unsigned int pointeur, int taille) {

    unsigned int sup = (inf+taille-1) % SEQ_NUM_SIZE;

    return
        /* inf <= pointeur <= sup */
        ( inf <= sup && pointeur >= inf && pointeur <= sup ) ||
        /* sup < inf <= pointeur */
        ( sup < inf && pointeur >= inf) ||
        /* pointeur <= sup < inf */
        ( sup < inf && pointeur <= sup);
}
