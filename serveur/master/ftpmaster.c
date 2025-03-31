#include "gestion_slaves.h"

#define MAX_NAME_LEN 256

// Variables globales
int listenfd;                  // Descripteur de socket pour l'écoute des connexions entrantes
int num_slaves_connected;      // Nombre d'esclaves actuellement connectés

slave_t slaves[NB_SLAVES];     // Tableau contenant les informations sur les esclaves

/**
 * @brief Gestionnaire du signal SIGINT (Ctrl+C)
 *
 * Ce gestionnaire ferme la socket d'écoute, notifie tous les esclaves connectés
 * de leur statut "DEAD" (mort) et termine le programme.
 *
 * @param sig Numéro du signal reçu.
 */
void sigint_handler(int sig) {
    slave_status_t status = DEAD;
    close(listenfd);
    for (int i = 0; i < num_slaves_connected; i++) {
        if (slaves[i].hostname != NULL) {
            write(slaves[i].fd, &status, sizeof(slave_status_t));
        }
    }
    exit(0);
}

int main(int argc, char** argv) {
    slave_status_t status;              // Variable pour stocker le statut reçu d'un esclave
    socklen_t clientlen;                // Taille de l'adresse du client
    struct sockaddr_in clientaddr;      // Structure contenant l'adresse du client
    uint32_t port;                      // Port sur lequel écouter
    int last_slave_selected = 0;        // Indice du dernier esclave sélectionné pour redirection
    
    // Initialisation de la taille de l'adresse client et du port d'écoute
    clientlen = (socklen_t)sizeof(clientaddr);
    port = PORT;

    // Etablissement de la connexion avec les esclaves
    if (!master_connect_to_slaves(&num_slaves_connected)) {
        fprintf(stderr, "Error connecting to slaves\n");
        exit(EXIT_FAILURE);
    }

    // Installation du gestionnaire de signal pour SIGINT (Ctrl+C)
    Signal(SIGINT, sigint_handler);

    // Ouverture de la socket d'écoute sur le port 9199
    listenfd = Open_listenfd(port);

    // Initialisation du set de descripteurs pour select
    fd_set fdset;
    int last_fd = -1;                   // Dernier descripteur de fichier le plus grand dans le set
    FD_ZERO(&fdset);                    // Réinitialisation du set
    FD_SET(listenfd, &fdset);           // Ajout de la socket d'écoute au set
    last_fd = listenfd;

    // Ajout des descripteurs des esclaves au set
    for (int i = 0; i < num_slaves_connected; i++) {
        FD_SET(slaves[i].fd, &fdset);
        if (slaves[i].fd > last_fd) {
            last_fd = slaves[i].fd;
        }
    }


    while(1) {
        fd_set read_fds = fdset; // Copie du set de descripteurs pour l'appel à select

        // Attente d'activité sur le socket d'écoute ou sur l'un des esclaves
        int nready = select(last_fd + 1, &read_fds, NULL, NULL, NULL);
        if (nready < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        // Vérification d'une connexion entrante d'un client
        if (FD_ISSET(listenfd, &read_fds)) {
            // Acceptation de la connexion entrante
            int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
             // Redirection de la connexion vers un esclave (en utilisant une stratégie de sélection)
            last_slave_selected = redirection_to_slave(connfd, num_slaves_connected, last_slave_selected);
            continue;
        }

        // Parcours de tous les esclaves pour vérifier s'il y a une mise à jour d'état
        for (int i = 0; i < num_slaves_connected; i++) {
            if (FD_ISSET(slaves[i].fd, &read_fds)) {
                // Lecture du statut envoyé par l'esclave
                Rio_readn(slaves[i].fd, &status, sizeof(slave_status_t));
                slaves[i].available = status;
                
                // Si l'esclave indique qu'il est mort (DEAD)
                if (status == DEAD) {
                    FD_CLR(slaves[i].fd, &fdset);  // Retrait du descripteur du set
                    fprintf(stderr, "Slave %s: %d is disconnected\n", slaves[i].hostname, slaves[i].port);
                    close(slaves[i].fd);  // Fermeture de la connexion avec cet esclave

                    // Mise à jour du dernier descripteur (last_fd) si nécessaire
                    if (slaves[i].fd == last_fd) {
                        last_fd = listenfd;
                        for (int j = 0; j < num_slaves_connected; j++) {
                            if (slaves[j].fd > last_fd && FD_ISSET(slaves[j].fd, &fdset))
                                last_fd = slaves[j].fd;
                        }
                    }
                    // Décalage des éléments du tableau pour supprimer l'esclave déconnecté
                    for (int j = i; j < num_slaves_connected; j++) { 
                        slaves[j] = slaves[j + 1];
                    }
                    num_slaves_connected--;  // Décrémentation du nombre d'esclaves connectés
                    i--;  // Ajustement de l'indice pour reprendre correctement la boucle

                    // Si aucun esclave n'est connecté, on ferme la socket d'écoute et on quitte
                    if (num_slaves_connected == 0) {
                        fprintf(stderr, "All slaves disconnected\n");
                        close(listenfd);
                        exit(0);
                    }
                }
            }
        }
    }

}