/*************************************************************
* proto_tdd_v0 -  émetteur                                   *
* TRANSFERT DE DONNEES  v0                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    int prochain_paquet = 0;
    paquet_t paquet, ack; /* paquet utilisé par le protocole */

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while ( taille_msg != 0 ) {

        /* construction paquet */
        for (int i=0; i<taille_msg; i++) {
            paquet.info[i] = message[i];
        }
        paquet.lg_info = taille_msg;
        paquet.type = DATA;
        paquet.num_seq = prochain_paquet;

        paquet.somme_ctrl = generer_controle(paquet);
        printf("--------------D %d-------------->\n", paquet.num_seq);
        vers_reseau(&paquet);
        depart_temporisateur(100);
        int evt = attendre();
        int i = 0;
        while (evt!=-1 && i<=40)
        {
            printf("Retransmision\n");
            printf("--------------D %d-------------->\n", paquet.num_seq);
            vers_reseau(&paquet);
            depart_temporisateur(100);
            evt = attendre();
            i++;
        }
        if (i>=40){
            //arret_temporisateur();
            break;
        }
        de_reseau(&ack);
        printf("<--------------ACK--------------\n");
        arret_temporisateur();
        inc(&prochain_paquet, 2);




        /* lecture des donnees suivantes de la couche application */
        de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
