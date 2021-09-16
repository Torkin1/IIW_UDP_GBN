#include <netinet/ip.h>

int doHandShake(int sd, struct sockaddr *serverAddr, socklen_t addrlen, in_port_t *portBuf);