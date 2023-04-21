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
          bind
            |
            v
        recvfrom <--+
            |       |
            v       |
          sendto ---+
            |
            v
          close 

    UDP Server that listens to the port passed as first param.
    waits for a message from the client comprised of a single character.
    If the character is 'q' it closes the connection.
    If the character is 'h' it sends the current time.
    If the character is 'd' it sends the current date.    
*/
int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    // -------------- recvfrom & sendto --------------
    
    for (;;)
    {
        peer_addr_len = sizeof(struct sockaddr_storage);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
            continue; // error, keep going

        char host[NI_MAXHOST], service[NI_MAXSERV];

        s = getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0)
            printf("Received %ld bytes from %s:%s\n", (long) nread, host, service);
        else
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

        if (nread == 0)
            continue; // nothing to read, keep going

        time_t t;
        struct tm *tm;
        char s[64];

        switch (buf[0])
        {
        case 'q': // close connection and exit program  
            printf("Closing connection with %s:%s \n", host, service);
            close(sfd);
            exit(EXIT_SUCCESS);
            break;
        case 'h':
            t = time(NULL);
            tm = localtime(&t);
            strftime(s, sizeof(s), "%T", tm);
            printf("%s\n", s);
            sendto(sfd, s, strlen(s), 0, (struct sockaddr *) &peer_addr, peer_addr_len);
            break;
        case 'd':
            t = time(NULL);
            tm = localtime(&t);
            strftime(s, sizeof(s), "%F", tm);
            printf("%s\n", s);
            sendto(sfd, s, strlen(s), 0, (struct sockaddr *) &peer_addr, peer_addr_len);
            break;
        default:
            sprintf(s, "unknown command : %c [q : quit], [h : hora], [d : dia]", buf[0]);
            sendto(sfd, s , strlen(s), 0, (struct sockaddr *) &peer_addr, peer_addr_len);
            break;
        }
    }

    return 0;
}

// ============================================================ hombrecito ============================================================

/* struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname
    
    struct addrinfo *ai_next;      // linked list, next node
};

struct sockaddr {       <-----------------------------------------------------------+                
    unsigned short    sa_family;    // address family, AF_xxx                       |   
    char              sa_data[14];  // 14 bytes of protocol address                 |
};                                                                                  |
                                                                                    |
struct sockaddr_in {    <-----------------------------------------------------------+-------------> All these can be cast to sockaddr
    short int          sin_family;  // Address family, AF_INET                      |
    unsigned short int sin_port;    // Port number                                  |                
    struct in_addr     sin_addr;    // Internet address                             |  
    unsigned char      sin_zero[8]; // Same size as struct sockaddr                 |
};                                                                                  |
                                                                                    |                       
struct sockaddr_in6 {    <----------------------------------------------------------+
    sa_family_t     sin6_family;    // AF_INET6 
    in_port_t       sin6_port;      // Port number 
    uint32_t        sin6_flowinfo;  // IPv6 flow info
    struct in6_addr sin6_addr;      // IPv6 address 
    uint32_t        sin6_scope_id;  // Set of interfaces for a scope 
};


struct in_addr { // IPv4
    uint32_t s_addr; // that's a 32-bit int (4 bytes)
};

struct in6_addr { // IPv6
    unsigned char   s6_addr[16];   // IPv6 address
};

struct sockaddr_storage {
    sa_family_t  ss_family;     // address family

    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
};
 */