#include "client_protocol.h"

/**
 * @brief Récupère la taille d'un fichier.
 *
 * Utilise la fonction stat pour obtenir les informations du fichier. 
 * Si le fichier existe, retourne sa taille, sinon retourne 0.
 *
 * @param filename Nom du fichier dont on veut connaître la taille.
 * @return uint64_t Taille du fichier en octets ou 0 en cas d'erreur.
 */
uint64_t get_file_size(char *filename) {
    struct stat st;
    // Si stat réussit (retourne 0), on renvoie la taille du fichier.
    if (!stat(filename, &st)) {
        return st.st_size;
    } else {
        // En cas d'erreur (fichier non trouvé, etc.), on retourne 0.
        return 0; 
    }
}

void file_transfer_client(int clientfd, char *filename, typereq_t type) {
    request_t req;      // Structure de requête à envoyer au serveur
    response_t res;     // Structure de réponse reçue du serveur
    char filepath[MAXLINE]; // Buffer pour construire le chemin complet du fichier local
    int fd;             // Descripteur du fichier local
    uint64_t n;         // Nombre d'octets lus lors de la réception de la réponse
    uint64_t bytes_installed; // Taille du fichier déjà présent localement

    /* Construction du chemin de stockage du fichier :
     * Concatène STORAGE_PATH et le nom du fichier.
     */
    snprintf(filepath, STORAGE_PATH_LEN + MAX_FILENAME, "%s%s", STORAGE_PATH, filename);

    /* Vérification de l'existence du fichier localement :
     * Récupération de la taille du fichier s'il existe.
     */
    bytes_installed = get_file_size(filepath);

    // Préparation de la requête
    req.type = type;
    req.offset = bytes_installed;  // L'offset correspond à la taille du fichier déjà présent
    strncpy(req.filename, filename, MAX_FILENAME);
    // Supprime le caractère de nouvelle ligne s'il existe
    req.filename[strcspn(req.filename, "\n")] = '\0';  

    fprintf(stdout, "Sending request to server...\n");
    // Envoi de la requête au serveur
    Rio_writen(clientfd, &req, sizeof(request_t));

    // Réception de la réponse du serveur
    if ((n = rio_readn(clientfd, &res, sizeof(response_t))) != sizeof(response_t)) {
        if (n < 0) {
            perror("Error reading response");
            return;
        } else if (n == 0) {
            fprintf(stderr, "Error: Server disconnected\n");
            return;
        }
        fprintf(stderr, "Error: Invxalid response received!\n");
        return;
    }
    
    // Conversion de la taille du fichier de format réseau vers le format hôte
    res.file_size = ntohl(res.file_size);

    // Démarrage du chronomètre pour mesurer le temps de transfert
    struct timeval start, end;

    // Traitement en fonction du code de réponse du serveur
    switch (res.code)
    {
        case SUCCESS:
            // Si le transfert doit être effectué
            if (bytes_installed == 0) {
                // Si le fichier n'existe pas localement, on le crée (ou on écrase l'existant)
                if ((fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
                    perror("open");
                    return;
                }
            } else {
                // Si le fichier existe, on l'ouvre en mode ajout
                if ((fd = open(filepath, O_WRONLY | O_APPEND)) < 0) {
                    perror("open");
                    return;
                }
                // Positionnement du descripteur à l'offset pour reprendre le transfert
                if (lseek(fd, bytes_installed, SEEK_SET) < 0) {
                    perror("lseek");
                    close(fd);
                    return;
                }
            }

            /* Réception du fichier par blocs :
             * Calcul de la partie manquante du fichier.
             */
            uint32_t missing_part = res.file_size - bytes_installed;
            uint32_t received = 0;
            char buffer[MESSAGE_SIZE];

            // Démarrage du chronomètre
            gettimeofday(&start, NULL);
            while (received < missing_part) {
                // Détermination du nombre d'octets à lire lors de ce cycle (bloc de MESSAGE_SIZE ou le reste)
                uint32_t to_read = (missing_part - received < MESSAGE_SIZE) ? (missing_part - received) : MESSAGE_SIZE;
                uint64_t n_read = rio_readn(clientfd, buffer, to_read);
                // Vérifie si la lecture a échoué ou si le serveur a coupé la connexion
                if (n <= 0) {
                    fprintf(stderr, "Error: reading file data\n");
                    break;
                }
                // Écriture des données lues dans le fichier local
                uint64_t written = write(fd, buffer, n_read);
                if (written != n_read) {
                    perror("write");
                    break;
                }
                received += n_read;
            }
            close(fd);

            // Arrêt du chronomètre et calcul du temps écoulé
            gettimeofday(&end, NULL);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
            fprintf(stdout, "Transfer successfully complete.\n");
            fprintf(stdout, "%d bytes received in %.6f seconds (%.3f Kbytes/s).\n",
                    res.file_size, time_taken, res.file_size / time_taken / 1024.0);
            break;

        case UPDATED:
            // Le fichier est déjà à jour chez le client
            fprintf(stdout, "File already up to date.\n");
            break;
    
        case ERROR_FILE_NOT_FOUND:
            // Le serveur n'a pas trouvé le fichier demandé
            fprintf(stderr, "Error: File not found\n");
            return;

        case ERROR_INVALID_REQUEST:
            // La requête envoyée est invalide
            fprintf(stderr, "Error: Invalid request\n");
            return;

        default:
            // Cas par défaut pour une erreur inconnue
            fprintf(stderr, "Error: Unknown error\n");
            return;
    }
}

int analyze_command(char *buf, char *cmd, char *filename) {
    int tokenCount = 0;
    int i = 0, j = 0;
    int len = strlen(buf);

    // Supprimer le caractère de fin de ligne éventuel
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }

    // Ignorer les espaces initiaux
    while (buf[i] == ' ')
        i++;
    if (buf[i] == '\0')
        return -1;  // Chaîne vide après espaces

    // Extraction du premier token (commande)
    tokenCount++;
    j = 0;
    while (buf[i] != '\0' && buf[i] != ' ') {
        cmd[j++] = buf[i++];
    }
    cmd[j] = '\0';

    // Ignorer les espaces entre la commande et le filename
    while (buf[i] == ' ')
        i++;

    if (buf[i] == '\0') {
        // Seul le token commande est présent
        return 0;
    }

    // Extraction du second token (filename)
    tokenCount++;
    j = 0;
    while (buf[i] != '\0' && buf[i] != ' ') {
        filename[j++] = buf[i++];
    }
    filename[j] = '\0';

    // Ignorer les espaces éventuels en fin de chaîne
    while (buf[i] == ' ')
        i++;

    // S'il reste des caractères, il y a trop de tokens
    if (buf[i] != '\0')
        return -1;

    // Retourne le nombre de tokens - 1 (0 pour commande seule, 1 pour commande et filename)
    return tokenCount - 1;
}

int client_connect_to_slave(int masterfd) {
    slave_info_t slave_info;

    // Lecture des informations de l'esclave depuis le maître
    Rio_readn(masterfd, &slave_info, sizeof(slave_info_t));
    // Conversion des valeurs reçues en format hôte
    slave_info.slave_available = ntohl(slave_info.slave_available);
    slave_info.port = ntohl(slave_info.port);
    if (slave_info.slave_available == 0) {
        return -1;
    }
    // Tentative de connexion à l'esclave à l'aide de son hostname et de son port
    int fd = open_clientfd(slave_info.hostname, slave_info.port);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to connect to slave %s: %d\n", slave_info.hostname, slave_info.port);
        exit(0);
    }
    return fd;
}
