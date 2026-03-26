//
// Created by gyankos on 26/03/26.
//

#include <jackbergus/networking/udp/UDPClient.h>
#include <jackbergus/networking/udp/UDPServer.h>

#ifndef GENERALFRAMEWORK_UDP_H
#define GENERALFRAMEWORK_UDP_H

#if defined(__vxworks)
#include "vxWorks.h"
#include "sockLib.h"
#include "inetLib.h"
#include "stdioLib.h"
#include "strLib.h"
#include "hostLib.h"
#include "ioLib.h"
#include "udpExample.h"
#else
#include <bits/stdc++.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <errno.h>
#endif


#define MAXLINE     (1024)
#define SERVER_PORT (5004)


static inline int GetWSASocketError(int sockfd)
{
    int ris, OptVal = 1;
    printf ("setsockopt()\n");
    ris = setsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&OptVal, sizeof(OptVal));
    return(ris);
}

#endif //GENERALFRAMEWORK_UDP_H