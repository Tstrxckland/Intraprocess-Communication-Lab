#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "list.h"

#define PORT 9001
#define ACK "ACK"
#define BUFFER_SIZE 1024

static int serverSocket = -1;
static int clientSocket = -1;
static list_t *mylist = NULL;

void cleanup(void) {
    if (clientSocket >= 0) {
        close(clientSocket);
        clientSocket = -1;
    }

    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }

    if (mylist != NULL) {
        list_free(mylist);
        mylist = NULL;
    }
}

void handle_signal(int sig) {
    (void)sig;
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    int n, val, idx;
    int reuse = 1;

    char buf[BUFFER_SIZE];
    char sbuf[BUFFER_SIZE];
    char *token;
    char *endptr;
    struct sockaddr_in servAddr;

    atexit(cleanup);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Create the linked list.
    mylist = list_alloc();

    if (mylist == NULL) {
        fprintf(stderr, "Failed to allocate linked list\n");
        return EXIT_FAILURE;
    }

    // Create the server socket.
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Allow the server port to be reused.
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               &reuse, sizeof(reuse));

    // Configure the server address.
    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket.
    if (bind(serverSocket, (struct sockaddr *)&servAddr,
             sizeof(servAddr)) < 0) {
        perror("Bind failed");
        return EXIT_FAILURE;
    }

    // Listen for client connections.
    if (listen(serverSocket, 1) < 0) {
        perror("Listen failed");
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept a client connection.
    clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket < 0) {
        perror("Accept failed");
        return EXIT_FAILURE;
    }

    printf("Client connected!\n");

    while (1) {
        memset(buf, 0, sizeof(buf));
        memset(sbuf, 0, sizeof(sbuf));

        // Receive a command from the client.
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0);

        if (n <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buf[n] = '\0';
        token = strtok(buf, " \r\n");

        if (token == NULL) {
            snprintf(sbuf, sizeof(sbuf), "ERROR: Empty command");
        }

        // EXIT
        else if (strcmp(token, "exit") == 0) {
            printf("Exit command received.\n");
            break;
        }

        // GET LENGTH
        else if (strcmp(token, "get_length") == 0) {
            val = list_length(mylist);
            snprintf(sbuf, sizeof(sbuf), "Length = %d", val);
        }

        // ADD FRONT
        else if (strcmp(token, "add_front") == 0) {
            token = strtok(NULL, " \r\n");

            if (token == NULL) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: Missing value");
            } else {
                val = (int)strtol(token, &endptr, 10);

                if (*endptr != '\0') {
                    snprintf(sbuf, sizeof(sbuf),
                             "ERROR: Invalid value");
                } else {
                    list_add_to_front(mylist, val);
                    snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
                }
            }
        }

        // ADD BACK
        else if (strcmp(token, "add_back") == 0) {
            token = strtok(NULL, " \r\n");

            if (token == NULL) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: Missing value");
            } else {
                val = (int)strtol(token, &endptr, 10);

                if (*endptr != '\0') {
                    snprintf(sbuf, sizeof(sbuf),
                             "ERROR: Invalid value");
                } else {
                    list_add_to_back(mylist, val);
                    snprintf(sbuf, sizeof(sbuf), "%s%d", ACK, val);
                }
            }
        }

        // ADD AT POSITION
        else if (strcmp(token, "add_position") == 0) {
            token = strtok(NULL, " \r\n");

            if (token == NULL) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: Missing index");
            } else {
                idx = atoi(token);
                token = strtok(NULL, " \r\n");

                if (token == NULL) {
                    snprintf(sbuf, sizeof(sbuf),
                             "ERROR: Missing value");
                } else {
                    val = atoi(token);

                    if (idx < 0 || idx > list_length(mylist)) {
                        snprintf(sbuf, sizeof(sbuf),
                                 "ERROR: Invalid index");
                    } else {
                        list_add_at_index(mylist, val, idx);
                        snprintf(sbuf, sizeof(sbuf),
                                 "%s%d", ACK, val);
                    }
                }
            }
        }

        // REMOVE BACK
        else if (strcmp(token, "remove_back") == 0) {
            if (list_length(mylist) == 0) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: List is empty");
            } else {
                val = list_remove_from_back(mylist);
                snprintf(sbuf, sizeof(sbuf), "%d", val);
            }
        }

        // REMOVE FRONT
        else if (strcmp(token, "remove_front") == 0) {
            if (list_length(mylist) == 0) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: List is empty");
            } else {
                val = list_remove_from_front(mylist);
                snprintf(sbuf, sizeof(sbuf), "%d", val);
            }
        }

        // REMOVE AT POSITION
        else if (strcmp(token, "remove_position") == 0) {
            token = strtok(NULL, " \r\n");

            if (token == NULL) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: Missing index");
            } else {
                idx = atoi(token);

                if (idx < 0 || idx >= list_length(mylist)) {
                    snprintf(sbuf, sizeof(sbuf),
                             "ERROR: Invalid index");
                } else {
                    val = list_remove_at_index(mylist, idx);
                    snprintf(sbuf, sizeof(sbuf), "%d", val);
                }
            }
        }

        // GET ELEMENT
        else if (strcmp(token, "get") == 0) {
            token = strtok(NULL, " \r\n");

            if (token == NULL) {
                snprintf(sbuf, sizeof(sbuf),
                         "ERROR: Missing index");
            } else {
                idx = atoi(token);

                if (idx < 0 || idx >= list_length(mylist)) {
                    snprintf(sbuf, sizeof(sbuf),
                             "ERROR: Invalid index");
                } else {
                    val = list_get_elem_at(mylist, idx);
                    snprintf(sbuf, sizeof(sbuf), "%d", val);
                }
            }
        }

        // PRINT LINKED LIST
        else if (strcmp(token, "print") == 0) {
            char *listString = listToString(mylist);

            if (listString != NULL) {
                snprintf(sbuf, sizeof(sbuf), "%s", listString);
            } else {
                snprintf(sbuf, sizeof(sbuf), "Empty list");
            }
        }

        // MENU
        else if (strcmp(token, "menu") == 0) {
            snprintf(sbuf, sizeof(sbuf),
                     "Commands: print, get_length, add_back, "
                     "add_front, add_position, remove_back, "
                     "remove_front, remove_position, get, exit");
        }

        // UNKNOWN COMMAND
        else {
            snprintf(sbuf, sizeof(sbuf),
                     "ERROR: Unknown command");
        }

        // Send the response to the client.
        if (send(clientSocket, sbuf, sizeof(sbuf), 0) < 0) {
            perror("Send failed");
            break;
        }
    }

    printf("Shutting down server...\n");
    return EXIT_SUCCESS;
}
