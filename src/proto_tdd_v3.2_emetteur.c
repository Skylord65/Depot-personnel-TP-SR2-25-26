/*************************************************************
* proto_tdd_v3.2 -  émetteur                                 *
* TRANSFERT DE DONNEES  v3.2                                 *
*                                                            *
* Protocole sans contrôle de flux, sans reprise sur erreurs  *
*                                                            *
* E. Lavinal - Univ. de Toulouse III - Paul Sabatier         *
**************************************************************/

#include <stdio.h>
#include "application.h"
#include "couche_transport.h"
#include "services_reseau.h"
#include <stdlib.h>

/* =============================== */
/* Programme principal - émetteur  */
/* =============================== */
int main(int argc, char* argv[])
{

    unsigned char message[MAX_INFO]; /* message de l'application */
    int taille_msg; /* taille du message */
    int prochain_paquet = 0,borne_inf = 0, taille_fin;
    paquet_t paquet[16], ack; /* paquet utilisé par le protocole */
    int compteur_ack = 0;

    if(argc==2){
        taille_fin = atoi(argv[1]);
    } else if (argc==1) {
        taille_fin = 4;
    } else
    {
        perror("parametre");
    }
    

    init_reseau(EMISSION);

    printf("[TRP] Initialisation reseau : OK.\n");
    printf("[TRP] Debut execution protocole transport.\n");

    /* lecture de donnees provenant de la couche application */
    de_application(message, &taille_msg);

    /* tant que l'émetteur a des données à envoyer */
    while (taille_msg != 0 || borne_inf != prochain_paquet) {
        
        if (dans_fenetre(borne_inf, prochain_paquet, taille_fin) && taille_msg > 0)
        {
            
            /* construction paquet */
            for (int i=0; i<taille_msg; i++) {
                paquet[prochain_paquet].info[i] = message[i];
            }
            paquet[prochain_paquet].lg_info = taille_msg;
            paquet[prochain_paquet].type = DATA;
            paquet[prochain_paquet].num_seq = prochain_paquet;

            paquet[prochain_paquet].somme_ctrl = generer_controle(paquet[prochain_paquet]);
        
            printf("|--------------D %d-------------->\n", paquet[prochain_paquet].num_seq);
            vers_reseau(&paquet[prochain_paquet]);
            if(borne_inf == prochain_paquet)
            {
                depart_temporisateur(100);
            }
            inc(&prochain_paquet, 16);
            de_application(message, &taille_msg);
        } else {
            int evt = attendre();
            if (evt==-1)
            {
                de_reseau(&ack);
                printf("|<--------------ACK %d--------------\n", ack.num_seq);
                if (verifier_controle(ack) && dans_fenetre(borne_inf, ack.num_seq, taille_fin))
                {
                    if(borne_inf != 0 && (borne_inf-1)==ack.num_seq){
                        compteur_ack++;
                        if(compteur_ack==3){
                            compteur_ack=0;
                            int i = borne_inf;
                            depart_temporisateur(100);
                            while (i!=prochain_paquet)
                            {
                                printf("|------------- R D %d-------------->\n", paquet[i].num_seq);
                                vers_reseau(&paquet[i]);
                                inc(&i, 16);
                            }
                        }
                    } else{
                        compteur_ack=0;
                    }

                    borne_inf = ack.num_seq;
                    inc(&borne_inf, 16);
                    if (borne_inf == prochain_paquet)
                    {
                        printf("Plus assez de crédits\n");
                        arret_temporisateur();
                    }
                    //printf("ack num addr : %p\n", &ack.num_seq);
                    
                }

            } else 
            {
                int i = borne_inf;
                depart_temporisateur(100);
                while (i!=prochain_paquet)
                {
                    printf("|------------- R D %d-------------->\n", paquet[i].num_seq);
                    vers_reseau(&paquet[i]);
                    inc(&i, 16);
                }
                
            }
        }




        /* lecture des donnees suivantes de la couche application */
        //de_application(message, &taille_msg);
    }

    printf("[TRP] Fin execution protocole transfert de donnees (TDD).\n");
    return 0;
}
