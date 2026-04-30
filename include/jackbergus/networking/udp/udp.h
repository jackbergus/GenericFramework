// udp.h
// This file is part of GeneralFramework
//
// Copyright (C)  2026 - Giacomo Bergami
//
// GeneralFramework is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  GeneralFramework is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GeneralFramework. If not, see <http://www.gnu.org/licenses/>.
//
// Created by gyankos on 26/03/26.
//


#if defined(__vxworks)
#include "vxWorks.h"
#include "sockLib.h"
#include "inetLib.h"
#include "stdioLib.h"
#include "strLib.h"
#include "hostLib.h"
#include "ioLib.h"
#include "udpExample.h"
#elif (defined(WIN32)|defined(_WIN64))
#include <winsock2.h>
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

#ifndef JACKBERGUS_UDP
#define JACKBERGUS_UDP

#define MAXLINE     (1024)
#define SERVER_PORT (5004)


static inline int GetWSASocketError(int sockfd)
{
    int ris, OptVal = 1;
    //printf ("setsockopt()\n");
    ris = setsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&OptVal, sizeof(OptVal));
    return(ris);
}

#endif