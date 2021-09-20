#include <netinet/ip.h>

/**
	Implements HS command of dm_protocol.
	@param portBuf pointer to buffer which will hold the port where the server will listen for client request
	@return 0 if a positive response is received, meaning that the server succesfully allocated resources to handle client request and opened a port for it, else -1
*/
int doHandShake(int sd, struct sockaddr *serverAddr, socklen_t addrlen, in_port_t *portBuf);