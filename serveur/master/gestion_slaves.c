#include "gestion_slaves.h"

// Tableaux de configuration pour les esclaves (à adapter selon votre configuration réseau)
char *slaves_config_host[NB_SLAVES] = {"im2ag-217-03", "im2ag-217-04", "im2ag-203-14"}; // A CHANGER
char *slaves_config_port[NB_SLAVES] = {"2222", "2223", "2224"}; // A CHANGER

int master_connect_to_slaves(int *slaves_connected) {
    int i = 0;
    int slave_found = 0;  // Compteur du nombre d'esclaves connectés
    while (i < NB_SLAVES) {
        // Tente d'ouvrir une connexion avec l'esclave i
        int slave_fd = open_clientfd(slaves_config_host[i], atoi(slaves_config_port[i]));
        if (slave_fd < 0) {
            // Si la connexion échoue, affiche un message d'erreur
            fprintf(stderr, "Slave %s: %d is not available\n", slaves_config_host[i], atoi(slaves_config_port[i]));
        } else {
            // Si la connexion est établie, stocke les informations de l'esclave dans le tableau
            slaves[slave_found].fd = slave_fd;
            slaves[slave_found].hostname = strdup(slaves_config_host[i]);
            slaves[slave_found].port = atoi(slaves_config_port[i]);
            slaves[slave_found].available = AVAILBLE;
            fprintf(stdout, "Connected to slave %s: %d\n", slaves[slave_found].hostname, slaves[slave_found].port);
            slave_found++;  // Incrémente le nombre d'esclaves connectés
        }
        i++;  // Passe à l'esclave suivant dans la configuration
    }
    // Met à jour le nombre d'esclaves connectés via le paramètre passé par adresse
    *slaves_connected = slave_found;
    // Retourne 1 si au moins un esclave est connecté, sinon 0
    return slave_found == 0 ? 0 : 1;
}

int redirection_to_slave(int connfd, int num_slave_connected, int last_slave_selected) {
    slave_info_t info;  // Structure pour stocker les informations de l'esclave à transmettre au client

    // Parcourt tous les esclaves pour trouver un esclave disponible
    for (int slave_index = 0; slave_index < num_slave_connected; slave_index++) {
        // Calcul de l'indice suivant dans la rotation (round-robin)
        int i = (last_slave_selected + slave_index + 1) % num_slave_connected;
        
        // Vérifie la disponibilité de l'esclave
        if (slaves[slave_index].available == AVAILBLE) {
            // L'esclave est disponible : préparation de la structure avec le statut disponible
            info.slave_available = htonl(1);
            // Copie le nom de l'hôte de l'esclave sélectionné
            strcpy(info.hostname, slaves[i].hostname);
            // Affecte le port de l'esclave et le convertit en format réseau
            info.port = slaves[i].port;
            info.port = htonl(info.port);
            // Envoie les informations de l'esclave au client
            Rio_writen(connfd, &info, sizeof(slave_info_t));
            return i;  // Retourne l'indice de l'esclave sélectionné
        } else {
            // Si l'esclave n'est pas disponible, affiche un message d'erreur
            fprintf(stderr, "Slave %s: %d is not available\n", slaves[i].hostname, slaves[i].port);
        }
    }
    // Aucun esclave n'est disponible : préparation d'une réponse négative
    info.slave_available = 0;
    info.hostname[0] = '\0';
    info.port = 0;
    // Envoie l'information d'indisponibilité au client
    Rio_writen(connfd, &info, sizeof(slave_info_t));
    return 0;  // Retourne 0 pour indiquer qu'aucun esclave n'a été sélectionné
}