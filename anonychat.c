#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {

    int status;
    struct addrinfo hints, *p, *servinfo; // points to results linked list from getaddrinfo
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: anonychat hostname\n");
        return 1;
    }

    /*
        memset is used to fill a block of memory:    
        (pointer to memory [&hints is the memory address of hints], value to set, number of bytes to set to the value)        
    */

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // getaddrinfo performs DNS lookup as well, so passing example.net as an argument will return an IP address.
    if ((status = getaddrinfo(argv[1], NULL, &hints, &servinfo)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        struct sockaddr_in *ipv4;
        struct sockaddr_in6 *ipv6;

        // get the pointer to the address itself,
        // different fields in ipv4 and ipv6:
        if (p->ai_family == AF_INET) { // IPV4
            ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // Convert the IP to a string and print it
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("    %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(servinfo);
    return 0;



}



