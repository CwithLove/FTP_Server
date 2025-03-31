#include "ftp_protocol.h"
#include "client_protocol.h"

int main(int argc, char **argv) {
    char *host;                  // Adresse (hostname) du serveur maître
    char buf[MAXLINE];           // Buffer pour la lecture des commandes saisies par l'utilisateur
    char cmd[COMMAND_LEN];       // Buffer pour stocker le type de commande extraite (ex: "get", "bye")
    char filename[MAXLINE];      // Buffer pour stocker le nom de fichier fourni par l'utilisateur
    typereq_t type;              // Type de requête à envoyer (actuellement seul GET est supporté)
    int status;                  // Variable pour le retour de l'analyse de commande
    int masterfd, clientfd;      // Descripteurs de fichier pour la connexion avec le serveur maître et l'esclave

    // Vérifie que le nombre d'arguments est correct
    if (argc != 2) {
        fprintf(stderr, "usage: %s <host>\n", argv[0]);
        exit(0);
    }

    // Affichage d'un message de connexion et récupération du hostname
    fprintf(stdout, "Connecting to %s...\n", argv[1]);
    host = argv[1];
    
    // Création d'une socket client et connexion au serveur maître sur le port 9199
    masterfd = open_clientfd(host, PORT);
    if (masterfd < 0) {
        fprintf(stderr, "Error: Failed to connect to server, check the connection informations %s\n", host);
        exit(0);
    }
    
    // Connexion à un esclave via le serveur maître
    clientfd = client_connect_to_slave(masterfd);
    if (clientfd < 0) {
        fprintf(stdout, "No Server available, please try again later.\n");
        exit(0);
    }

    // Affichage du menu et des commandes disponibles pour l'utilisateur
    fprintf(stdout, "----------------------------------------\n");
    fprintf(stdout, "Welcome to the FTP client!\n");
    fprintf(stdout, "----------------------------------------\n");
    fprintf(stdout, "Available commands:\n");
    fprintf(stdout, "1. get <filename>\n");
    fprintf(stdout, "2. bye\n\n");

    // Boucle principale pour lire les commandes de l'utilisateur
    while (1) {
        // Affiche le prompt et attend la commande de l'utilisateur
        fprintf(stdout, "ftp> ");
        if (fgets(buf, MAXLINE, stdin) == NULL) {
            fprintf(stderr, "Error: Failed to read input.\n");
            continue; // Recommence la boucle en cas d'erreur de lecture
        }
    
        // Analyse la commande saisie par l'utilisateur.
        // La fonction analyze_command extrait le nom de la commande dans 'cmd' et le nom de fichier dans 'filename'
        status = analyze_command(buf, cmd, filename);
        if (status == -1) {
            // Commande au format invalide
            fprintf(stderr, "Error: Invalid command format. Use: <get> <filename> or bye\n");
            continue;
        }
        
        // Si une seule commande est saisie (pas de paramètre supplémentaire)
        if (status == 0) {
            if (strcmp(cmd, "bye") == 0) {
                // Commande "bye" pour quitter le client
                fprintf(stdout, "Disconnecting...\n");
                Close(clientfd);
                exit(0);
            } else {
                // Si la commande unique n'est pas "bye", elle n'est pas supportée
                fprintf(stderr, "Error: Unsupported single command. Use 'bye' or <get> <filename>.\n");
                continue;
            }
        }
    
        // Si la commande comporte deux tokens, seul le "get" est accepté pour télécharger un fichier
        if (strcmp(cmd, "get") == 0) {
            type = GET; // Définit le type de requête à GET
            // Appel de la fonction pour le transfert de fichier côté client
            file_transfer_client(clientfd, filename, type);
        } else {
            // Toute autre commande est considérée comme non supportée
            fprintf(stderr, "Error: Unsupported command.\n");
            continue;
        }
    }
    return 0;
}