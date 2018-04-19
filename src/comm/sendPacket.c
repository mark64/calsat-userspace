#include <polysat.h>
#include "comms.h"

#define SAT_CMD_RECV_PORT 5001
#define SAT_IP_ADDRESS "127.0.0.1"
#define GROUND_RECV_PORT  5000

int send_to_sockaddr(char *data, uint16_t dataLen, struct sockaddr_in *addr) {
    int sockfd, result;
    // Ideally this is where we would encode the data with encodePacket
    // But we can't really do that while we're still using poly sat library
    if ((sockfd = socket_init(0)) < 0) {
        return -1;
    }
    result = socket_write(sockfd, data, dataLen, addr);
    socket_close(sockfd);
    return result;
}

struct sockaddr_in *make_sockaddr(char *ip_address, int port) {
    struct sockaddr_in *addr = calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET; // Set IPv4
    addr->sin_port = htons(port); // Set the port
    inet_pton(AF_INET, ip_address, &(addr->sin_addr)); // Set the ip address
    return addr;
}

int send_to_sat(char *data, uint16_t dataLen) {
    struct sockaddr_in *addr = make_sockaddr(SAT_IP_ADDRESS, SAT_CMD_RECV_PORT);
    int res = send_to_sockaddr(data, dataLen, addr);
    free(addr);
    return res;
}

int send_to_ground(char *data, uint16_t dataLen, struct sockaddr_in *addr) {
    addr->sin_port = htons(GROUND_RECV_PORT);
    int res = send_to_sockaddr(data, dataLen, addr);
    return res;
}
