#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <thread>

#define ODD_PORT 18001
#define EVEN_PORT 18002
#define SERVER_IP "127.0.0.1"
#define MAXLINE 1024

void *send_odd_numbers(void *arg);
void *send_even_numbers(void *arg);

int create_socket_and_connect(int port);

int main() {
    pthread_t odd_thread, even_thread;

    // Criar e iniciar as threads
    pthread_create(&odd_thread, NULL, send_odd_numbers, NULL);
    pthread_create(&even_thread, NULL, send_even_numbers, NULL);

    // Aguarda as threads (apesar do cliente ser infinito, por consistência)
    pthread_join(odd_thread, NULL);
    pthread_join(even_thread, NULL);

    return 0;
}

void *send_odd_numbers(void *arg) {
    int sockfd;

    // Tentativa contínua de conexão ao servidor na porta ODD
    while (true) {
        sockfd = create_socket_and_connect(ODD_PORT);
        if (sockfd != -1) {
            break;  // Se a conexão for bem-sucedida, saia do loop
        }
        std::cout << "waiting for the server to wake up (ODD)\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Aguardar 10 ms antes de tentar novamente
    }

    while (true) {
        int random_number = rand() % 100;
        if (random_number % 2 != 0) {
            char message[MAXLINE];
            snprintf(message, sizeof(message), "Odd number: %d", random_number);
            send(sockfd, message, strlen(message), 0);
            std::cout << "Sent to ODD server: " << message << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Aguardar 100 ms
    }

    close(sockfd);
    return NULL;
}

void *send_even_numbers(void *arg) {
    int sockfd;

    // Tentativa contínua de conexão ao servidor na porta EVEN
    while (true) {
        sockfd = create_socket_and_connect(EVEN_PORT);
        if (sockfd != -1) {
            break;  // Se a conexão for bem-sucedida, saia do loop
        }
        std::cout << "waiting for the server to wake up (EVEN)\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Aguardar 10 ms antes de tentar novamente
    }

    while (true) {
        int random_number = rand() % 100;
        if (random_number % 2 == 0) {
            char message[MAXLINE];
            snprintf(message, sizeof(message), "Even number: %d", random_number);
            send(sockfd, message, strlen(message), 0);
            std::cout << "Sent to EVEN server: " << message << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Aguardar 100 ms
    }

    close(sockfd);
    return NULL;
}

int create_socket_and_connect(int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        // Conexão falhou, fechar o socket
        close(sockfd);
        return -1;
    }

    return sockfd;  // Retorna o socket se a conexão for bem-sucedida
}
