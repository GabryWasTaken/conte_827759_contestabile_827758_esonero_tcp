/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
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
#include <time.h>
#include <ctype.h> // Per tolower
#include "protocol.h"

#define NO_ERROR 0

// Helper per string comparison case insensitive portabile
int case_insensitive_compare(const char *s1, const char *s2) {
#if defined WIN32
    return stricmp(s1, s2);
#else
    return strcasecmp(s1, s2);
#endif
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

// Implementazione funzioni generazione dati
float get_temperature(void) {
    // Range: -10.0 to 40.0
    return -10.0f + ((float)rand() / RAND_MAX) * 50.0f;
}
float get_humidity(void) {
    // Range: 20.0 to 100.0
    return 20.0f + ((float)rand() / RAND_MAX) * 80.0f;
}
float get_wind(void) {
    // Range: 0.0 to 100.0
    return ((float)rand() / RAND_MAX) * 100.0f;
}
float get_pressure(void) {
    // Range: 950.0 to 1050.0
    return 950.0f + ((float)rand() / RAND_MAX) * 100.0f;
}

int main(int argc, char *argv[]) {

	// TODO: Implement server logic
    srand((unsigned int)time(NULL)); // Inizializza random seed

    int port = SERVER_PORT;
    int i;
    // Parsing argomenti server
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
    }

#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif

	int my_socket;
    struct sockaddr_in server_addr;

	// TODO: Create socket
	my_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    int reuse = 1;
    if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0) {
        printf("Errore nel settaggio di SO_REUSEADDR\n");
        // Non blocchiamo l'esecuzione, è solo un warning
    }
    // ---------------------------------

	// TODO: Configure server address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// TODO: Bind socket
	if (bind(my_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error binding socket\n");
        closesocket(my_socket);
        return -1;
    }

	// TODO: Set socket to listen
	if (listen(my_socket, QUEUE_SIZE) < 0) {
        printf("Error listening\n");
        closesocket(my_socket);
        return -1;
    }

    printf("Server in ascolto sulla porta %d\n", port);

	// TODO: Implement connection acceptance loop
    struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int client_socket;

    // Lista città supportate
    const char *supported_cities[] = {
        "Bari", "Roma", "Milano", "Napoli", "Torino",
        "Palermo", "Genova", "Bologna", "Firenze", "Venezia"
    };
    int num_cities = 10;

	while (1) {
#if defined WIN32
        client_socket = accept(my_socket, (struct sockaddr*)&client_addr, &client_len);
#else
        client_socket = accept(my_socket, (struct sockaddr*)&client_addr, (socklen_t*)&client_len);
#endif
        if (client_socket < 0) {
            printf("Error accepting connection\n");
            continue;
        }

	    // Handle client communication
        weather_request_t req;
        weather_response_t resp;
        memset(&resp, 0, sizeof(resp));

        // Ricevi richiesta
        if (recv(client_socket, (char*)&req, sizeof(req), 0) > 0) {

            printf("Richiesta '%c %s' dal client ip %s\n",
                   req.type, req.city, inet_ntoa(client_addr.sin_addr));

            // Validazione
            int city_found = 0;
            int type_valid = 0;

            // Check Type
            if (req.type == 't' || req.type == 'h' || req.type == 'w' || req.type == 'p') {
                type_valid = 1;
            }

            // Check City
            for (int j = 0; j < num_cities; j++) {
                if (case_insensitive_compare(req.city, supported_cities[j]) == 0) {
                    city_found = 1;
                    break;
                }
            }

            if (!type_valid) {
                resp.status = STATUS_INVALID_REQUEST;
                resp.type = '\0';
                resp.value = 0.0f;
            } else if (!city_found) {
                resp.status = STATUS_CITY_UNAVAILABLE;
                resp.type = '\0'; // Come da specifica per status != 0
                resp.value = 0.0f;
            } else {
                resp.status = STATUS_SUCCESS;
                resp.type = req.type;

                switch(req.type) {
                    case 't': resp.value = get_temperature(); break;
                    case 'h': resp.value = get_humidity(); break;
                    case 'w': resp.value = get_wind(); break;
                    case 'p': resp.value = get_pressure(); break;
                }
            }

            // Invia Risposta
            send(client_socket, (char*)&resp, sizeof(resp), 0);

        }

	    closesocket(client_socket);
	}

	printf("Server terminated.\n");

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
