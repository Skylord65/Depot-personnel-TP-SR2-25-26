/*************************************************************
* proto_tdd_v4 -  récepteur                                  *
* TRANSFERT DE DONNEES  v4                                   *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"
#include <string.h>
#include <stdlib.h>

/* =============================== */
/* Programme principal - récepteur */
/* =============================== */
int main(int argc, char* argv[])
{
    unsigned char message[MAX_INFO]; /* message pour l'application */
    paquet_t paquet,tab_paquet[16], ack; /* paquet utilisé par le protocole */
    int paquet_acquitté[16];
    int borne_inf = 0, taille_fin;
    int fin = 0; /* condition d'arrêt */

    if(argc==2){
        taille_fin = atoi(argv[1]);
    } else if (argc==1) {
        taille_fin = 4;
    } else
    {
        perror("parametre");
    }

    for (int i =0; i<10;i++){
        paquet_acquitté[i]=0;
    }

    init_reseau(RECEPTION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* tant que le récepteur reçoit des données */
    while ( !fin ) {

        // attendre(); /* optionnel ici car de_reseau() fct bloquante */
        de_reseau(&paquet);
        printf("--------------D %d-------------->|\n", paquet.num_seq);
        if (verifier_controle(paquet) && dans_fenetre(borne_inf, paquet.num_seq, taille_fin)) {
                
            ack.type = ACK;
            ack.num_seq = paquet.num_seq;
            ack.somme_ctrl = generer_controle(ack);
            
            printf("<--------------ACK %d--------------|\n", ack.num_seq);
            vers_reseau(&ack);
            tab_paquet[paquet.num_seq]=paquet;
            paquet_acquitté[paquet.num_seq]=1;

            while (paquet_acquitté[borne_inf]==1)
            {
                paquet_acquitté[borne_inf]=0;
                inc(&borne_inf, 16);
                for (int i=0; i<tab_paquet[borne_inf].lg_info; i++) {
                    message[i] = tab_paquet[borne_inf].info[i];
                }
                fin = vers_application(message, tab_paquet[borne_inf].lg_info);
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
