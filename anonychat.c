#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    
    int connect_listen = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: anonychat connect/listen\n");
        return 1;

        if (strcmp(argv[1], "connect") == 0) {
            connect_listen = 1;
            if (argc != 4) {
                fprintf(stderr, "usage: anonychat connect hostname port\n");
                return 1;
            }
        } else {
            connect_listen = 2;
            if (argc != 3) {
                fprintf(stderr, "usage: anonychat listen port\n");
                return 1;
            }
        }
    }



    
    int status;
    int sockfd;
    socklen_t addr_len;
    struct addrinfo hints, *p, *servinfo; // points to results linked list from getaddrinfo
    struct sockaddr_storage *their_addr; // This is will store incomming connection address (server code for listening / accepting)
    

    /*
        memset is used to fill a block of memory:    
        (pointer to memory [&hints is the memory address of hints], value to set, number of bytes to set to the value)        
    */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // getaddrinfo performs DNS lookup as well, so passing example.net as an argument will return an IP address.
    // TODO: pass port arugment for service in getaddrinfo "argv[2]" i.e. 23 or telnet. cat /etc/services for full list of services
    if ((status = getaddrinfo(argv[2], argv[3], &hints, &servinfo)) != 0) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        return 2;
    }
    
    /*
        socket(PF_INET, SOCK_STREAM, 0) is the old way to fill out the socket, however
        passing in the properties of the linkedlist returned by getaddrinfo is the new way because the values are the same.
        
        sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
 
        NOTE: Protocol 0 automatically sets the protocol for the socket type specified rather than needing to hardcode it.
    */

    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /*
        simply having bind(sockfd, p->ai_addr, p->ai_addrlen) won't work. Running this will give a segmentation fault (core dump)
        This is because p is empty, you need to itterate through the linked list returned from getaddrinfo (&servinfo is the linked list)
    */ 
    if (connect_listen == 1) {
        for (p = servinfo; p != NULL; p = p->ai_next) {
            printf("Attempting to connec to host: %s on port %s\n", argv[1], argv[2]);
    
            /*
                Might not need bind if we don't care what our local port is. Removing bind() means the kernel will choose a local port for us
                if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
                    perror("Failed to bind");
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
            */
    
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
                perror("Failed to connect");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (connect_listen == 2) {
        for (p = servinfo; p != NULL; p = p->ai_next) {
    
            // Need to call bind before listen so the server is running on a specific port
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
                perror("Failed to bind");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
    
            if (listen(sockfd, 1) < 0) {
                perror("Cannot listen on that socket");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
    
            addr_len = sizeof &their_addr;
            // their_addr needs to be typecast to sockaddr
            if (accept(sockfd, (struct sockaddr *)&their_addr, &addr_len) < 0) {
                perror("Cannot accept a connection from that host / on that port");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
    
            char *msg = "Lee was here!";
            int len, bytes_sent;
            len = strlen(msg);
            bytes_sent = send(sockfd, msg, len, 0);
            if (bytes_sent < 0) {
                perror("Couldn't send data");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            
            if (recv(sockfd, msg, len, 0) < 0) {
                perror("Couldn't recieve data");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        }
    }

    /*
        TODO: reuse port if in use
        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    */

    /*    
        This piece of code simply returns information abot a given IP / domain

        printf("IP addresses for %s:\n\n", argv[1]);

        char ipstr[INET6_ADDRSTRLEN];
        
        for(p = servinfo; p != NULL; p = p->ai_next) {
            void *addr;
            char *ipver;
            struct sockaddr_in *ipv4;
            struct sockaddr_in6 *ipv6;

            get the pointer to the address itself,
            different fields in ipv4 and ipv6:
            if (p->ai_family == AF_INET) { // IPV4
                ipv4 = (struct sockaddr_in *)p->ai_addr;
                addr = &(ipv4->sin_addr);
                ipver = "IPv4";
            } else {
                ipv6 = (struct sockaddr_in6 *)p->ai_addr;
                addr = &(ipv6->sin6_addr);
                ipver = "IPv6";
            }

            Convert the IP to a string and print it
            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            printf("    %s: %s\n", ipver, ipstr);
        }
    */
    
        
    freeaddrinfo(servinfo);
    return 0;

}



