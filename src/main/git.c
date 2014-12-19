#include <stdlib.h> //NULL

#include <sys/socket.h> //socket functions
#include <netinet/in.h> //socket address structure

#include <netdb.h> //for gethostbyname(host)

#include "utils.h"

#include "helpers.c"

/* returns connected socket descriptor or -1 for error */
int connect_to_host(char *host, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) die(1,"couldn't create socket");

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(port);

    struct hostent *hostinfo = gethostbyname(host);
    if(hostinfo == NULL)
        die(2, "couldn't find server host info");

    name.sin_addr = *((struct in_addr *)hostinfo->h_addr);
    if(connect(fd, (struct sockaddr *)&name, sizeof(name)) < 0) die(1,"coundn't bind socket");
    return fd;
}

void read_to_end(int fd) {
    int len = 0;
    while((len = read_pkt_line(fd)) > 0) {
        write(1, buffer, len);
    }
}

int main(int argc, int *argv[]) {

    char *host = "localhost";
    int port = 9418;
    char *repo = "testrepo";

    int fd = connect_to_host(host, port);
    send_proto_request(fd, host, repo);
    //read_ref_announcement(fd);
    read_to_end(fd);
    send_flush_pkt(fd);
//    send_negotiation_request(fd);
//    read_pack_file(fd); //stores pack file in /tmp
    close(fd);
}
