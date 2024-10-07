#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <thread>
#include <mutex>

#define TEMP_PORT 18001
#define HUMID_PORT 18002
#define PRESS_PORT 18003
#define ALT_PORT 18004
// #define SERVER_IP "192.168.15.60" // ip do raspberrypi na rede atuals
#define SERVER_IP "127.0.0.0" // para testes locais em localhost
#define MAXLINE 1024

struct ThreadArgs // Usado para passar mais argumentos para dentro da thread
{
    int port;
    const char *desc;
};

std::mutex print_mutex; // mutex usado para o print coordenado. uso atomico da saida

void *receive_data(void *arg);
int create_socket_and_connect(int port);

int main()
{
    pthread_t temp_thread, humid_thread, press_thread, alt_thread;

    // Argumentos para as threads
    ThreadArgs temp_args = {TEMP_PORT, "Temperature"};
    ThreadArgs humid_args = {HUMID_PORT, "Humidity"};
    ThreadArgs press_args = {PRESS_PORT, "Pression"};
    ThreadArgs alt_args = {ALT_PORT, "Altitude"};

    // Criar e iniciar as threads
    pthread_create(&temp_thread, NULL, receive_data, &temp_args);
    pthread_create(&humid_thread, NULL, receive_data, &humid_args);
    pthread_create(&press_thread, NULL, receive_data, &press_args);
    pthread_create(&alt_thread, NULL, receive_data, &alt_args);

    // Aguardar as threads (apesar do cliente ser infinito, por consistência)
    pthread_join(temp_thread, NULL);
    pthread_join(humid_thread, NULL);
    pthread_join(press_thread, NULL);
    pthread_join(alt_thread, NULL);

    return 0;
}

void *receive_data(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    int sockfd;
    {
        std::lock_guard<std::mutex> lock(print_mutex);

        std::cout << " - Socket connection oppened to |" << args->desc << "|\t" << " int port \t|" << args->port << "|"
                  << std::endl;
    }
    while (true)
    {
        // Tentativa contínua de conexão ao servidor na porta especificada
        sockfd = create_socket_and_connect(args->port);
        if (sockfd != -1)
        {
            break; // Se a conexão for bem-sucedida, saia do loop
        }

        {
            std::lock_guard<std::mutex> lock(print_mutex);

            std::cout << "Aguardando o servidor na porta " << args->port << " para " << args->desc << "\n";
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Aguardar 10 ms antes de tentar novamente
    }

    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);
    read(sockfd, buffer, MAXLINE);
    std::cout << args->desc << " recept: " << buffer;
   if (args->desc == "Temperature")
    {
        std::cout << "ºC" << std::endl;
    }
    else if (args->desc == "Humidity")
    {
        std::cout << "%" << std::endl;
    }
    else if (args->desc == "Pression")
    {
        std::cout << " hPa" << std::endl;
    }
    else if (args->desc == "Altitude")
    {
        std::cout << "m" << std::endl;
    }


    close(sockfd);
    return NULL;
}

int create_socket_and_connect(int port)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Erro to create socket" << std::endl;
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Endereço inválido/ não suportado" << std::endl;
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Falha na conexão" << std::endl;
        close(sockfd);
        return -1;
    }

    return sockfd; // Retorna o socket se a conexão for bem-sucedida
}