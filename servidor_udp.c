#define _GNU_SOURCE // added so the vscode linter doesn't bug out

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

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
*/


/* 
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
    int sfd, s, j;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // -------------- socket & bind --------------
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // use my IP
    hints.ai_protocol = 0; // any protocol. Will be UDP because of SOCK_DGRAM
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrunfo: %s \n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // getaddrinfo() returns a list of possible directions. We check each one
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue; // keep going

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; // success

        if (rp == NULL)
            fprintf(stderr, "Could not bind\n"); // no direction works
        exit(EXIT_FAILURE);

        freeaddrinfo(result); // we don't need it anymore
        close(sfd);
    }

    // -------------- recvfrom & sendto --------------
    

    return 0;
}

 /*
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
 // -------------- socket & bind -------------- 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // use my IP
    hints.ai_protocol = 0; // any protocol. Will be UDP because of SOCK_DGRAM
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0)
    {
        fprintf(stderr, "getaddrunfo: %s \n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    // getaddrinfo() returns a list of possible directions. We check each one
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue; // keep going
        
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; // success
        
        if (rp == NULL) 
            fprintf(stderr, "Could not bind\n"); // no direction works
            exit(EXIT_FAILURE);

        freeaddrinfo(result); // we don't need it anymore
        close(sfd);
    } */

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