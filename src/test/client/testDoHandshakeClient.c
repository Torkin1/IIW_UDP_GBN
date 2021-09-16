#include "client/hsController.h"
#include "dm_protocol/dm_protocol.h"
#include "../testSuites.h"
#include <string.h>
#include <unistd.h>
#include "logger/logger.h"

#define EXPECTED_PORT 50321

void testDoHandShakeClient()
{

    int sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    in_port_t expected, actual = -1;
    expected = EXPECTED_PORT;

    // client

    char errBuf[1024];
    sprintf(errBuf, "testDoHandshake: client: expected %d, actual %d\n", expected, actual);

    doHandShake(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in), &actual);
    assertEquals(&actual, &expected, sizeof(in_port_t), errBuf, NULL);

}
