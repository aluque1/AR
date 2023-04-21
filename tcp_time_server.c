#define _GNU_SOURCE // added so the vscode linter doesn't bug out

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#define BUF_SIZE 512
#define LISTEN_BACKLOG 50

/*
         socket
            |
            v
          bind
            |
            v
          listen
            |
            v
          accept
            |
            v
        recvfrom <--+
            |       |
            v       |
          sendto ---+
            |
            v
          close

    TCP Server that listens to the port passed as first param.
    waits for a message from the client comprised of a single character.
    If the character is 'q' it closes the connection.
    If the character is 'h' it sends the current time.
    If the character is 'd' it sends the current date.
*/

int main(int argc, char const *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, cfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;      /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; /* Success */

        close(sfd);
    }

    // listen for connections
    if (listen(sfd, LISTEN_BACKLOG) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept a connection
    peer_addr_len = sizeof(struct sockaddr_storage);
    cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
    if (cfd == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // while the connection is open read from it, process the message and send a response
    while (buf[0] != 'q')
    {
        // read from the connection
        nread = recv(cfd, buf, BUF_SIZE, 0);
        if (nread == -1)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        // proccess the message
        switch (buf[0])
        {
        case 'q':
            printf("Closing connection with client %s\n", argv[1]);
            break;
        case 'h':
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(buf, BUF_SIZE, "%H:%M:%S", tm);
            break;
        case 'd':
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            strftime(buf, BUF_SIZE, "%T", tm);
            break;
        default:
            printf("Invalid command %c from client %s \n", buf[0], argv[1]);
            break;
        }

        // send the response
        if (send(cfd, buf, BUF_SIZE, 0) == -1)
        {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }

    // close the connection
    close(cfd);

    return 0;
}
