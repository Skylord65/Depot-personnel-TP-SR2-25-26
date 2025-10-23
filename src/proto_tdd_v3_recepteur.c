/*************************************************************
* proto_tdd_v3 -  récepteur                                  *
* TRANSFERT DE DONNEES  v3                                   *
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
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet, ack; /* paquet utilisé par le protocole */
    int paquet_attendu = 0;
    int fin = 0; /* condition d'arrêt */

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);
        printf("--------------D %d-------------->|\n", paquet.num_seq);
        if (verifier_controle(paquet)) {
            if (paquet.num_seq == paquet_attendu){
                /* extraction des donnees du paquet recu */
                for (int i=0; i<paquet.lg_info; i++) {
                    message[i] = paquet.info[i];
                }
                /* remise des données à la couche application */
                fin = vers_application(message, paquet.lg_info);
                ack.type = ACK;
                ack.num_seq = paquet.num_seq;
                ack.somme_ctrl = generer_controle(ack);
                inc(&paquet_attendu, 16);
                printf("<--------------ACK %d--------------|\n", ack.num_seq);
                vers_reseau(&ack);
            } else if (paquet_attendu!=0){
                printf("<--------------ACK %d--------------|\n", ack.num_seq);
                vers_reseau(&ack);
            }
            
        }
    }
    
    int i_test = 0;

    depart_temporisateur(500);

    int evt = attendre();

    while (evt == -1)
    {
        de_reseau(&paquet);
        if(i_test>20) return 0;
        if (test_temporisateur(1)==1){
            arret_temporisateur();
            depart_temporisateur(500);
            evt = attendre();
        }
        printf("<--------------ACK R %d--------------|\n", ack.num_seq);
        i_test++;
        vers_reseau(&ack);
    }
    
    printf("[TRP] Fin execution protocole transport.\n");
    return 0;
}
