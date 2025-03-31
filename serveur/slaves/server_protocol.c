#include "server_protocol.h"

// Descripteurs de fichiers globaux pour la socket d'écoute et la connexion avec le master
int listenfd = -1;
int masterfd = -1;

int file_transfer_server(int connfd) {
    request_t req;       // Structure contenant la requête envoyée par le client
    response_t res;      // Structure contenant la réponse à envoyer au client
    char filepath[MAXLINE]; // Buffer pour le chemin complet du fichier
    struct stat st;      // Structure pour récupérer les informations sur le fichier
    int fd;              // Descripteur du fichier à transférer
    ssize_t n;           // Nombre d'octets lus

    // Lecture de la requête du client
    if ((n = rio_readn(connfd, &req, sizeof(request_t))) != sizeof(request_t)) {
        // Si aucune donnée n'est lue, le client s'est déconnecté
        if (n == 0) {
            fprintf(stderr, "Bye bye! Client disconnected\n");
            return 0;
        } else if (n < 0) {
            // En cas d'erreur de lecture
            perror("Error reading request");
            return 1;
        } else {
            // Requête de taille invalide
            fprintf(stderr, "Error: Invalid request received!\n");
            return 1;
        }
    }

    // Conversion de l'offset de la requête du format réseau vers le format hôte
    req.offset = ntohl(req.offset);

    // Traitement de la requête en fonction de son type
    switch (req.type) {
        case GET:
            /* Constitution du chemin complet du fichier :
             * Concaténation du chemin de stockage et du nom de fichier demandé.
             */
            snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, req.filename);
            
            /* Tentative d'ouverture du fichier en mode lecture seule */
            if ((fd = open(filepath, O_RDONLY)) < 0) {
                // Si l'ouverture échoue, on informe le client avec un code d'erreur
                perror(filepath);
                res.code = ERROR_FILE_NOT_FOUND;
                res.file_size = htonl(0);
                rio_writen(connfd, &res, sizeof(response_t));
                return 1;
            }
            
            /* Récupération des informations sur le fichier (notamment la taille) */
            if (fstat(fd, &st) < 0) {
                perror("Error getting file size");
                res.code = ERROR_FILE_NOT_FOUND;
                res.file_size = htonl(0);
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }

            // Si le client a déjà le fichier à jour (offset >= taille du fichier)
            if (req.offset >= st.st_size) {
                res.code = UPDATED;
                res.file_size = htonl(st.st_size);
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }

            // Positionnement du descripteur de fichier à l'offset demandé par le client
            if (lseek(fd, req.offset, SEEK_SET) < 0) {
                perror("lseek");
                res.code = ERROR_INVALID_REQUEST;
                res.file_size = htonl(st.st_size);
                rio_writen(connfd, &res, sizeof(response_t));
                close(fd);
                return 1;
            }

            /* Préparation de la réponse :
             * - Conversion de la taille du fichier en format réseau.
             * - Définition du code de réussite.
             */
            res.file_size = htonl((uint32_t)st.st_size);
            res.code = SUCCESS;
            /* Envoi de l’en-tête de réponse (code et taille du fichier) au client */
            rio_writen(connfd, &res, sizeof(response_t));

            /* Transfert du fichier par blocs */
            uint32_t remaining = res.file_size;
            char buffer[MESSAGE_SIZE];
            while (remaining > 0) {
                // Détermination du nombre d'octets à lire lors de ce cycle (la taille du bloc ou le reste du fichier)
                uint32_t bytes_to_read = (remaining < MESSAGE_SIZE) ? remaining : MESSAGE_SIZE;
                uint64_t n_read = read(fd, buffer, bytes_to_read);
                if (n_read < 0) {
                    perror("Error reading file");
                    break;
                }
                if (n_read == 0) {
                    break; // Fin de fichier
                }
                // Envoi des octets lus vers le client
                rio_writen(connfd, buffer, n_read);
                remaining -= n_read;
            }
            // Fermeture du descripteur du fichier après transfert
            close(fd);
            break;

        default:
            // Pour une requête de type inconnu, on envoie un code d'erreur
            res.code = ERROR_INVALID_REQUEST;
            res.file_size = htonl(0);
            rio_writen(connfd, &res, sizeof(response_t));
            break;
    }
    // Retourne 1 pour indiquer que le transfert ou le traitement peut continuer
    return 1;
}
