/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "protocol.h"

#define NO_ERROR 0

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {

	// TODO: Implement client logic

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

    // Variabili per argomenti e connessione
    char *server_ip = "127.0.0.1";
    int port = SERVER_PORT;
    char *request_arg = NULL;
    int my_socket;
    struct sockaddr_in server_addr;

    // Parsing argomenti da riga di comando
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            server_ip = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
            request_arg = argv[++i];
        }
    }

    if (request_arg == NULL) {
        printf("Errore: Parametro -r obbligatorio.\n");
        printf("Uso: %s [-s server] [-p port] -r \"type city\"\n", argv[0]);
        clearwinsock();
        return -1;
    }

    // Parsing della stringa di richiesta (es. "t bari" o "p Reggio Calabria")
    weather_request_t req;
    memset(&req, 0, sizeof(req)); // Pulizia memoria

    if (strlen(request_arg) < 3 || request_arg[1] != ' ') {
        // Formato minimo non rispettato o mancanza spazio
        req.type = request_arg[0];
    } else {
        req.type = request_arg[0];
        // Copia tutto ciò che c'è dopo il primo carattere e lo spazio
        strncpy(req.city, request_arg + 2, sizeof(req.city) - 1);
    }

	// TODO: Create socket
	my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket < 0) {
        printf("Errore nella creazione del socket.\n");
        clearwinsock();
        return -1;
    }

	// TODO: Configure server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	// TODO: Connect to server
	if (connect(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Errore di connessione al server %s:%d\n", server_ip, port);
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

	// TODO: Implement communication logic

    // 1. Invio Richiesta
	if (send(my_socket, (char*)&req, sizeof(req), 0) < 0) {
        printf("Errore nell'invio dei dati.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    // 2. Ricezione Risposta
    weather_response_t resp;
    if (recv(my_socket, (char*)&resp, sizeof(resp), 0) <= 0) {
        printf("Errore nella ricezione dei dati o connessione chiusa.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    // 3. Visualizzazione Output
    // Formato: Ricevuto risultato dal server ip <ip_address>. <messaggio_costruito>
    printf("Ricevuto risultato dal server ip %s. ", server_ip);

    if (resp.status != STATUS_SUCCESS) {
        if (resp.status == STATUS_CITY_UNAVAILABLE) {
            printf("Città non disponibile\n");
        } else {
            printf("Richiesta non valida\n");
        }
    } else {
        // Successo: formatta in base al tipo

        if (strlen(req.city) > 0) {
            req.city[0] = toupper(req.city[0]);
        }

        switch (resp.type) {
            case 't':
                printf("%s: Temperatura = %.1f°C\n", req.city, resp.value);
                break;
            case 'h':
                printf("%s: Umidità = %.1f%%\n", req.city, resp.value);
                break;
            case 'w':
                printf("%s: Vento = %.1f km/h\n", req.city, resp.value);
                break;
            case 'p':
                printf("%s: Pressione = %.1f hPa\n", req.city, resp.value);
                break;
            default:
                printf("Tipo di risposta sconosciuto.\n");
                break;
        }
    }

	// TODO: Close socket
	closesocket(my_socket);

	printf("Client terminated.\n");

	clearwinsock();
	return 0;
} // main end
