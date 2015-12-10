#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int const PORT = 8554, BACKLOG = 5;
size_t const BUFLEN = 4096;
char buf[BUFLEN];

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on bind");

    if (listen(sockfd, BACKLOG))
        error("ERROR on listen");

    sockaddr_in cli_addr;
    socklen_t cli_addr_len = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (sockaddr*)&cli_addr, &cli_addr_len);
    if (newsockfd < 0)
        error("ERROR on accept");

    ssize_t n = read(newsockfd, buf, BUFLEN);
    if (n < 0)
        error("ERROR on read");

    for (int i = 0; i < n; ++i)
        printf("%02X ", buf[i]);

    close(newsockfd);
    close(sockfd);
    return 0;
}
