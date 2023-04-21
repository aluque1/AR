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

/*
         socket
            |
            v
          connect
            |
            v
          sendto <--+
            |       |
            v       |
        recvfrom ---+
            |
            v
          close

*/

int main(int argc, char const *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    size_t len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Obtain address(es) matching host/port */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;      /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; /* Any protocol */

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
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

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        close(sfd);
    }

    if (rp == NULL)
    { /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result); /* No longer needed */

    /* While connection is up send remaining command-line arguments as separate
       datagrams, and read responses from server */

    for (;;)
    {
        printf("Enter command: ");
        fgets(buf, BUF_SIZE, stdin);

        len = strlen(buf) + 1; // +1 for the null byte
        if (len + 1 > BUF_SIZE)
        {
            fprintf(stderr, "Ignoring long message in argument \n");
            continue;
        }

        if (sendto(sfd, buf, len, 0, rp->ai_addr, rp->ai_addrlen) != len)
        {
            fprintf(stderr, "Error sending message \n");
            continue;
        }

        nread = recvfrom(sfd, buf, BUF_SIZE, 0, rp->ai_addr, &rp->ai_addrlen);
        if (nread == -1)
        {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("Received %ld bytes: %s \n", (long)nread, buf);
    }
    // close the socket
    close(sfd);
    return 0;
}
