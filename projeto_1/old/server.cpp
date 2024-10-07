#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>

#define ODD_PORT 18001
#define EVEN_PORT 18002
#define MAXLINE 1024

void *handle_odd(void *arg);
void *handle_even(void *arg);

int create_server_socket(int port);

int main() {
    pthread_t odd_thread, even_thread;

    // Criar e iniciar as threads para ouvir números ímpares e pares
    pthread_create(&odd_thread, NULL, handle_odd, NULL);
    pthread_create(&even_thread, NULL, handle_even, NULL);

    // Aguardar o término das threads
    pthread_join(odd_thread, NULL);
    pthread_join(even_thread, NULL);

    std::cout << "\n\nok, finished" << std::endl;

    return 0;
}

void *handle_odd(void *arg) {
    int sockfd = create_server_socket(ODD_PORT);
    int connfd;
    char buffer[MAXLINE];

    std::cout << "Conecte o socket de números ímpares" << std::endl;
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0) {
        std::cerr << "Accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - start;

        if (elapsed.count() >= 10.0) {
            break;  // Termina após 10 segundos
        }

        memset(buffer, 0, MAXLINE);
        read(connfd, buffer, MAXLINE);
        std::cout << " -> ímpar recebido: " << buffer << std::endl;
    }

    close(connfd);
    close(sockfd);
    return NULL;
}

void *handle_even(void *arg) {
    int sockfd = create_server_socket(EVEN_PORT);
    int connfd;
    char buffer[MAXLINE];

    std::cout << "Conecte o socket de números pares: " << std::endl;
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0) {
        std::cerr << "Accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - start;

        if (elapsed.count() >= 10.0) {
            break;  // Termina após 10 segundos
        }

        memset(buffer, 0, MAXLINE);
        read(connfd, buffer, MAXLINE);
        std::cout << " -> Par recebido: " << buffer << std::endl;
    }

    close(connfd);
    close(sockfd);
    return NULL;
}

int create_server_socket(int port) { // Cria um socket para ouvir na porta x
    int listenfd;
    struct sockaddr_in server_addr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Falha em criar o socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    return listenfd;
}
