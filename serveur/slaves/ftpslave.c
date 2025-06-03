#include "server_protocol.h"

/**
 * @brief Gestionnaire du signal SIGINT (Ctrl+C) pour une terminaison propre du slave.
 *
 * Lorsque le signal SIGINT est reçu, cette fonction :
 * - Affiche un message d'arrêt.
 * - Envoie un message de déconnexion (statut DEAD) au master si connecté.
 * - Ferme les sockets d'écoute et de connexion au master.
 * - Termine le programme.
 *
 * @param sig Numéro du signal reçu.
 */
void sigint_handler(int sig) {
    slave_status_t status = DEAD;   // Définition du statut "DEAD" pour indiquer la déconnexion
    fprintf(stderr, "Shutting down...\n");
    // Envoi du message de déconnexion au master si le descripteur est valide
    if (masterfd == -1) {
        exit(0);
    }
    Rio_writen(masterfd, &status, sizeof(slave_status_t));
    close(listenfd);  // Fermeture de la socket d'écoute
    close(masterfd);  // Fermeture de la connexion avec le master
    exit(0);          // Terminaison du programme
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Use: %s <slave_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    slave_status_t status;          // Variable pour le statut du slave (AVAILBLE ou BUSY)
    socklen_t clientlen;            // Taille de la structure d'adresse client
    struct sockaddr_in clientaddr;  // Structure d'adresse pour les connexions entrantes
    clientlen = (socklen_t)sizeof(clientaddr);
    uint32_t port = atoi(argv[1]);  // Port sur lequel le slave écoute, fourni en argument

    // Mise en place de la socket d'écoute pour la connexion avec le master
    listenfd = Open_listenfd(port);
    if (listenfd < 0) {
        fprintf(stderr, "Error opening socket\n");
        exit(EXIT_FAILURE);
    }

    // Installation du gestionnaire de signal pour assurer une terminaison propre (SIGINT)
    Signal(SIGINT, sigint_handler);

    fd_set fdset;  // Ensemble de descripteurs de fichiers à surveiller avec select

    // Attente de la connexion du master sur la socket d'écoute
    masterfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    
    // Initialisation du set de descripteurs et ajout de listenfd et masterfd
    FD_ZERO(&fdset);
    FD_SET(listenfd, &fdset);
    FD_SET(masterfd, &fdset);
    int maxfd = (listenfd > masterfd) ? listenfd : masterfd;

    int connfd;  // Descripteur pour les connexions clients

    // Boucle principale du slave
    while (1) {
        // Pour l'instant, le slave se considère comme disponible
        status = AVAILBLE;
        // Envoi régulier du statut "AVAILBLE" au master
        Rio_writen(masterfd, &status, sizeof(slave_status_t));

        fd_set read_fds = fdset;  // Copie du set de descripteurs pour l'appel à select

        // Attente d'événements sur listenfd et masterfd
        select(maxfd + 1, &read_fds, NULL, NULL, NULL);

        // Vérifie si une connexion client est en attente sur la socket d'écoute
        if (FD_ISSET(listenfd, &read_fds)) {
            // Acceptation de la connexion client
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

            // Le slave devient occupé pour traiter le client : mise à jour du statut
            status = BUSY;
            Rio_writen(masterfd, &status, sizeof(slave_status_t));
            
            // Traitement du transfert de fichiers avec le client
            while (file_transfer_server(connfd));
            // Fermeture de la connexion client une fois le transfert terminé
            Close(connfd);
        }
        
        // Vérifie si un message est reçu du master
        if (FD_ISSET(masterfd, &read_fds)) {
            slave_status_t recv_status;
            Rio_readn(masterfd, &recv_status, sizeof(slave_status_t));
            if (recv_status == DEAD) {
                // Si le master indique une déconnexion, affiche un message et sort de la boucle
                fprintf(stderr, "Master disconnected\n");
                break;
            }
            // En cas de message autre que DEAD, ferme les connexions et termine le programme
            Close(listenfd);
            Close(connfd);
            exit(0);
        }
    }
}