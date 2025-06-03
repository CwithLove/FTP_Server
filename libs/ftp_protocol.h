#ifndef FTP_PROTOCOL_H
#define FTP_PROTOCOL_H

// Inclusion des bibliothèques nécessaires pour les opérations sur fichiers, sockets et autres
#include <time.h>
#include "csapp.h"          // Bibliothèque CSAPP pour les wrappers et gestion d'erreurs
#include <stdint.h>         // Pour utiliser des types entiers à taille fixe (ex: uint32_t)
#include <sys/stat.h>       // Pour récupérer des informations sur les fichiers (fstat)
#include <fcntl.h>          // Pour les opérations sur fichiers (open, etc.)
#include <unistd.h>         // Pour les fonctions close, read, write, etc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>     // Pour la création et la gestion des sockets
#include <netinet/in.h>     // Pour les structures d'adresses Internet

// Définition de constantes pour le protocole FTP

#define MAX_FILENAME 256    // Taille maximale d'un nom de fichier
#define MAX_HOSTNAME 256    // Taille maximale d'un nom d'hôte
#define MESSAGE_SIZE 1024   // Taille maximale d'un bloc de données pour le transfert de fichier
#define STORAGE_PATH "./storage/"  // Chemin du répertoire de stockage des fichiers
#define STORAGE_PATH_LEN 10   // Longueur du chemin de stockage (ici 10 caractères pour "./storage/")
#define PORT 9919           // Port d'écoute du serveur FTP (modifiable selon les besoins)
#define SLAVE_PORT 2222     // Port par défaut pour la connexion aux esclaves
#define NB_SLAVES 3         // Nombre total d'esclaves prévus dans le système

// Définition d'une énumération pour les types de requêtes
// Seul le type GET est traité dans cette phase
typedef enum {
    GET,    // Requête pour obtenir (télécharger) un fichier
    // PUT, // Pour envoyer un fichier (non implémenté)
    // LS,  // Pour lister le contenu d'un répertoire (non implémenté)
    // RM   // Pour supprimer un fichier (non implémenté)
} typereq_t;

// Structure représentant une requête dans le protocole FTP
typedef struct {
    typereq_t type;              // Type de la requête (GET, PUT, etc.)
    char filename[MAX_FILENAME]; // Nom du fichier demandé par le client
    uint32_t offset;             // Décalage à partir duquel commencer la lecture du fichier
} request_t;

// Définition d'une énumération pour les codes de réponse du protocole FTP
typedef enum {
    SUCCESS,              // La requête a été traitée avec succès
    UPDATED,              // Le fichier est déjà à jour chez le client
    ERROR_FILE_NOT_FOUND, // Le fichier demandé n'a pas été trouvé
    ERROR_INVALID_REQUEST // La requête reçue est invalide
} response_code_t;

// Structure représentant une réponse dans le protocole FTP
typedef struct {
    response_code_t code;  // Code de réponse indiquant le résultat de la requête
    uint32_t file_size;    // Taille du fichier en octets (en format réseau)
} response_t;

// Définition d'une énumération pour le statut d'un esclave dans le système
typedef enum {
    AVAILBLE, // Esclave disponible pour traiter une requête
    BUSY,     // Esclave occupé en cours de traitement d'une requête
    DEAD      // Esclave déconnecté ou hors service
} slave_status_t;

// Structure contenant les informations relatives à un esclave
typedef struct {
    char *hostname;           // Nom d'hôte de l'esclave
    uint32_t port;            // Port de communication de l'esclave
    int fd;                   // Descripteur de fichier pour la connexion avec l'esclave
    slave_status_t available; // Statut de l'esclave (AVAILBLE, BUSY, ou DEAD)
} slave_t;

// Structure utilisée pour transmettre les informations d'un esclave à un client
typedef struct {
    int slave_available;         // Indicateur de disponibilité de l'esclave (1 si disponible, 0 sinon)
    char hostname[MAX_HOSTNAME]; // Nom d'hôte de l'esclave
    uint32_t port;               // Port de l'esclave (en format réseau)
} slave_info_t;

#endif // FTP_PROTOCOL_H
