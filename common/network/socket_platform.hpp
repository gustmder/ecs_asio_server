#pragma once

#if defined(_WIN32)

#include <WinSock2.h>
#include <ws2tcpip.h>

#ifdef PlaySound
#undef PlaySound  // Prevent name clash
#endif

#else // POSIX platforms

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define SOCKET int
#define WORD unsigned short

#endif