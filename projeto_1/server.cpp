#include <iostream> 	//  Biblioteca padrão de c++
#include <pthread.h>    //  Para uso das threads
#include <sys/socket.h> //  Para criação de sockets em rede local
#include <netinet/in.h> //  Para conectar sockets na rede e tratatamento mais fácil de ip
#include <arpa/inet.h>  //  funç
#include <unistd.h>     //  Usado para descritores de texto em fluxo. Usado para coletar os dados http
#include <string.h>     //  Usado para facilitar o tratamento de strings;
#include <mutex>        //  Sendo usado para tratar as condições de disputa do stdout

//  Portas sequenciais para envio dos dados via
#define TEMP_PORT 18001
#define HUMID_PORT 18002
#define PRESS_PORT 18003
#define ALT_PORT 18004
#define MAXLINE 1024
#define ESP32_IP "192.168.15.100" // servidor padrão do ESP32

//  Criação da struct para armazernar todos os dados juntos
typedef struct Data_ESP32
{
    std::string temp;
    std::string hum;    
    std::string alt;
    std::string press;
} DATA;

std::mutex print_mutex; // mutex usado para garantir unicidade atômica de prints no terminal

//  Descrição das threads
void *handle_temp(void *arg);
void *handle_humid(void *arg);
void *handle_press(void *arg);
void *handle_alt(void *arg);

//  Função genérica de criar um socket
int create_server_socket(int port);

//Função de coleta dos dados através do HTTP do esp32
Data_ESP32 fetch_data_from_esp32();

std::string temp_data, humid_data, press_data, alt_data; // Strings de temperatura, umidade, pressão e altitude
int counter = 0; // contador de mensagens enviadas 
int main()
{
    while (1)   // looping infinito do servidor, para sempre fornecer
    {
        //  Leitura do HTTP do esp32
        Data_ESP32 data = fetch_data_from_esp32();


        std::cout << "\n" << ++counter << "º leitura:\n" << std::endl;
        std::cout << "Temperatura: " << data.temp << "°C" << std::endl;
        std::cout << "Umidade: " << data.hum << "%" << std::endl;
        std::cout << "Pressão: " << data.press << " hPa" << std::endl;
        std::cout << "Altitude: " << data.alt << "m\n" << std::endl;


        // Criar e iniciar as threads para transmitir as grandezas físicas
        pthread_t temp_thread, humid_thread, press_thread, alt_thread;
        pthread_create(&temp_thread, NULL, handle_temp, NULL);
        pthread_create(&humid_thread, NULL, handle_humid, NULL);
        pthread_create(&press_thread, NULL, handle_press, NULL);
        pthread_create(&alt_thread, NULL, handle_alt, NULL);

        // Aguardar o término das threads
        pthread_join(temp_thread, NULL);
        pthread_join(humid_thread, NULL);
        pthread_join(press_thread, NULL);
        pthread_join(alt_thread, NULL);
        std::cout << "\n\n finished " << counter << "º offering data. Keep going\n"
                  << std::endl;
        std::cout << "------------------<--->----------------------" << std::endl;
    }

    return 0;
}
// função usada para extrair, de uma string, o conteúdo entre dois elementos 
std::string extract_between(const std::string &data, const std::string &start_tag, const std::string &end_tag)
{
    std::size_t start_pos = data.find(start_tag);
    if (start_pos != std::string::npos)
    {
        start_pos += start_tag.length();
        std::size_t end_pos = data.find(end_tag, start_pos);
        if (end_pos != std::string::npos)
        {
            return data.substr(start_pos, end_pos - start_pos);
        }
    }
    return "Not Found";
}


Data_ESP32 fetch_data_from_esp32()
{
    std::string server_ip = ESP32_IP; // servidor padrão do ESP32

    int server_port = 80; // Porta HTTP padrão
    std::string request = "GET / HTTP/1.1\r\nHost: " + server_ip + "\r\nConnection: close\r\n\r\n";
    char buffer[4096];
    std::string response;
    Data_ESP32 data;
    data.alt = "vazio";
    data.hum = "vazio";
    data.press = "vazio";
    data.temp = "vazio";

    // Criar o socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cerr << "Erro ao criar o socket!" << std::endl;
        return data;
    }

    // Configurar o endereço do servidor
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    //  Converter 
    if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0)
    {
        std::cerr << "Endereço inválido!" << std::endl;
        close(sock);
        return data;
    }

    // Conectar ao ESP32
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Erro ao conectar ao coletor de dados ESP32!\nCertifique-se que este está ligado e em bom funcionamento\n  " << std::endl;
        close(sock);
        return data;
    }

    // Enviar a requisição HTTP
    send(sock, request.c_str(), request.size(), 0);

    // Ler a resposta
    while (true)
    {
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            break; // Nenhum dado recebido ou conexão encerrada
        }
        buffer[bytes_received] = '\0';
        response += buffer;
    }

    // Fechar o socket depois de ter recebido tudo
    close(sock);
    data.temp = extract_between(response, "<p>Temperatura: ", "&deg;C</p>");
    data.hum = extract_between(response, "<p>Umidade: ", "%</p>");
    data.press = extract_between(response, "<p>Pressão: ", " hPa</p>");
    data.alt = extract_between(response, "<p>Altitude: ", "m</p>");
    return data;
}

void *handle_temp(void *arg)
{
    int sockfd = create_server_socket(TEMP_PORT);
    int connfd;
    char buffer[MAXLINE];
    {
        std::lock_guard<std::mutex> lock(print_mutex);

        std::cout << " -- Waiting client for temperature in the port "<< TEMP_PORT << std::endl;
    }
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    Data_ESP32 data = fetch_data_from_esp32();

    send(connfd, data.temp.c_str(), data.temp.size(), 0);
    close(connfd);
    close(sockfd);
    return NULL;
}

void *handle_humid(void *arg)
{
    int sockfd = create_server_socket(HUMID_PORT);
    int connfd;
    char buffer[MAXLINE];
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << " -- Waiting client for humidity in the port " << HUMID_PORT << std::endl;
    }
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    Data_ESP32 data = fetch_data_from_esp32();

    send(connfd, data.hum.c_str(), data.hum.size(), 0);
    close(connfd);
    close(sockfd);
    return NULL;
}

void *handle_press(void *arg)
{
    int sockfd = create_server_socket(PRESS_PORT);
    int connfd;
    char buffer[MAXLINE];

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << " -- Waiting client for pression in the port " << PRESS_PORT << std::endl;
    }
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    Data_ESP32 data = fetch_data_from_esp32();
    send(connfd, data.press.c_str(), data.press.size(), 0);
    close(connfd);
    close(sockfd);
    return NULL;
}

void *handle_alt(void *arg)
{
    int sockfd = create_server_socket(ALT_PORT);
    int connfd;
    char buffer[MAXLINE];

    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << " -- Waiting client for altiitude in the port " << ALT_PORT << std::endl;
    }
    connfd = accept(sockfd, (struct sockaddr *)NULL, NULL);
    if (connfd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    Data_ESP32 data = fetch_data_from_esp32();

    send(connfd, data.alt.c_str(), data.alt.size(), 0);
    close(connfd);
    close(sockfd);
    return NULL;
}

int create_server_socket(int port)
{
    int listenfd;
    struct sockaddr_in server_addr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Falha em criar o socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Enable SO_REUSEADDR to allow the port to be reused quickly
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Failed to set SO_REUSEADDR" << std::endl;
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 10) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    return listenfd;
}
