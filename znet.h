
/*
 * znet.h — Cross-platform Networking (TCP, UDP, HTTP)
 * Part of Zen Development Kit (ZDK)
 *
 * Usage:
 * #define ZNET_IMPLEMENTATION
 * #include "znet.h"
 *
 * License: MIT
 * Author: Zuhaitz
 * Repository: https://github.com/z-libs/znet.h
 * Version: 1.0.0
 */

#ifndef ZNET_H
#define ZNET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Optional dependency: zstr.h (for HTTP). 
// If not available, HTTP functions will be disabled.
#if defined(__has_include)
#   if __has_include("zstr.h")
#       include "zstr.h"
#       define ZNET_HAS_ZSTR 1
#   endif
#elif defined(ZNET_USE_ZSTR)
#   include "zstr.h"
#   define ZNET_HAS_ZSTR 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Constants and types.

typedef enum 
{ 
    ZNET_UNSPEC = 0, 
    ZNET_IPV4   = 2, 
    ZNET_IPV6   = 23 
} znet_family;

typedef enum 
{
    ZNET_TCP,
    ZNET_UDP
} znet_proto;

// Opaque socket handle.
typedef struct 
{
    uintptr_t handle; 
    bool      valid;
} znet_socket;

// Universal IP address container.
typedef struct 
{
    znet_family family;
    uint16_t    port;
    uint32_t    scope_id; // For IPv6 link-local.
    union 
    {
        uint32_t v4;      
        uint8_t  v6[16];  
    } ip;
} znet_addr;

typedef enum 
{
    ZNET_POLL_READ  = 1 << 0,
    ZNET_POLL_WRITE = 1 << 1,
    ZNET_POLL_ERR   = 1 << 2
} znet_poll_flags;

#define ZNET_INVALID_SOCKET (znet_socket){ ~(uintptr_t)0, false }

#if defined(_WIN32)
    typedef int z_ssize_t;
#else
#   include <sys/types.h>
    typedef ssize_t z_ssize_t;
#endif

// Core lifecycle.

// Initializes the network subsystem (WSAStartup on Windows).
int znet_init(void);

// Shuts down the network subsystem.
void znet_term(void);

// Returns a string describing the last network error.
const char* znet_strerror(void);

// Address management.

// Parses an IP string ("127.0.0.1" or "::1") into an address.
bool znet_addr_from_str(const char *ip_str, uint16_t port, znet_addr *out);

// Converts an address back to a readable string.
void znet_addr_to_str(znet_addr addr, char *buf, size_t buf_len);

// Resolves a hostname (DNS) to an IP address.
int znet_resolve(const char *hostname, uint16_t port, znet_addr *out_addr);

// Socket operations.

// Creates a new socket.
znet_socket znet_socket_create(znet_family family, znet_proto type);

// Closes a socket and invalidates the handle.
void znet_close(znet_socket *s);

// Binds a socket to a local address/port.
int znet_bind(znet_socket s, znet_addr addr);

// Connects to a remote address (TCP).
int znet_connect(znet_socket s, znet_addr addr);

// Starts listening for incoming connections (TCP).
int znet_listen(znet_socket s, int backlog);

// Accepts a new connection (TCP).
znet_socket znet_accept(znet_socket s, znet_addr *out_client_addr);

// I/O operations.

// Sends data (TCP).
z_ssize_t znet_send(znet_socket s, const void *data, size_t len);

// Receives data (TCP).
z_ssize_t znet_recv(znet_socket s, void *buf, size_t len);

// Sends data to a specific address (UDP).
z_ssize_t znet_sendto(znet_socket s, const void *data, size_t len, znet_addr dest);

// Receives data and stores the sender's address (UDP).
z_ssize_t znet_recvfrom(znet_socket s, void *buf, size_t len, znet_addr *out_sender);

// Polls the socket for events. Returns >0 on event, 0 on timeout, -1 on error.
int znet_poll(znet_socket s, znet_poll_flags wait_for, int timeout_ms);

// Configuration.

int znet_set_timeout(znet_socket s, int ms);
int znet_set_nonblocking(znet_socket s, bool enable);
// Enable IPv4 mapping on IPv6 sockets.
int znet_set_dual_stack(znet_socket s, bool enable);

// HTTP

#ifdef ZNET_HAS_ZSTR
// Performs a simple blocking HTTP GET request.
zstr znet_http_get(const char *domain, const char *path, int timeout_ms);
#endif

#ifdef __cplusplus
}
#endif

#endif // ZNET_H

#ifdef ZNET_IMPLEMENTATION
#ifndef ZNET_IMPLEMENTATION_GUARD
#define ZNET_IMPLEMENTATION_GUARD

// Platform adaptation layer.

#if !defined(_WIN32)
#   ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#   endif
#   ifndef _DEFAULT_SOURCE
#   define _DEFAULT_SOURCE
#   endif
#endif

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#   pragma comment(lib, "ws2_32.lib")
#   include <winsock2.h>
#   include <ws2tcpip.h>
    
    typedef int socklen_t;
#   define ZNET__CLOSE_FN      closesocket
#   define ZNET__ERRNO         WSAGetLastError()
#   define ZNET__WOULDBLOCK    WSAEWOULDBLOCK
#   define ZNET__SOCKET        SOCKET
#   define ZNET__INVALID       INVALID_SOCKET
#   define ZNET__ERROR         SOCKET_ERROR
#else
#   include <sys/socket.h>
#   include <sys/select.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <fcntl.h>
#   include <errno.h>
    
#   define ZNET__CLOSE_FN      close
#   define ZNET__ERRNO         errno
#   define ZNET__WOULDBLOCK    EWOULDBLOCK
#   define ZNET__SOCKET        int
#   define ZNET__INVALID       -1
#   define ZNET__ERROR         -1
#endif

// Internal macros for result codes.
#ifndef Z_OK
#define Z_OK 0
#define Z_ERR -1
#endif

// Internal helpers.

static inline void znet__to_sys(znet_addr a, struct sockaddr_storage *out, socklen_t *len) 
{
    memset(out, 0, sizeof(*out));
    if (ZNET_IPV4 == a.family) 
    {
        struct sockaddr_in *s4 = (struct sockaddr_in*)out;
        s4->sin_family = AF_INET;
        s4->sin_port = htons(a.port);
        s4->sin_addr.s_addr = htonl(a.ip.v4);
        *len = sizeof(struct sockaddr_in);
    } 
    else 
    {
        struct sockaddr_in6 *s6 = (struct sockaddr_in6*)out;
        s6->sin6_family = AF_INET6;
        s6->sin6_port = htons(a.port);
        memcpy(s6->sin6_addr.s6_addr, a.ip.v6, 16);
        s6->sin6_scope_id = a.scope_id;
        *len = sizeof(struct sockaddr_in6);
    }
}

static inline znet_addr znet__from_sys(const struct sockaddr_storage *ss) 
{
    znet_addr a = {0};
    if (AF_INET == ss->ss_family) 
    {
        struct sockaddr_in *s4 = (struct sockaddr_in*)ss;
        a.family = ZNET_IPV4;
        a.port = ntohs(s4->sin_port);
        a.ip.v4 = ntohl(s4->sin_addr.s_addr);
    } 
    else if (AF_INET6 == ss->ss_family) 
    {
        struct sockaddr_in6 *s6 = (struct sockaddr_in6*)ss;
        a.family = ZNET_IPV6;
        a.port = ntohs(s6->sin6_port);
        memcpy(a.ip.v6, s6->sin6_addr.s6_addr, 16);
        a.scope_id = s6->sin6_scope_id;
    }
    return a;
}

// Core implementation.

int znet_init(void) 
{
#ifdef _WIN32
    WSADATA wsa; 
    return WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    return 0; 
#endif
}

void znet_term(void) 
{
#   ifdef _WIN32
    WSACleanup();
#   endif
}

const char* znet_strerror(void) 
{
#   ifdef _WIN32
    static char buf[64];
    sprintf(buf, "WSA Error Code: %d", ZNET__ERRNO);
    return buf;
#   else
    return strerror(errno);
#   endif
}

// Address logic.

bool znet_addr_from_str(const char *ip_str, uint16_t port, znet_addr *out) 
{
    struct addrinfo hints = {0}, *res;
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    
    if (0 != getaddrinfo(ip_str, NULL, &hints, &res)) 
    {
        return false;
    }
    
    struct sockaddr_storage ss;
    memcpy(&ss, res->ai_addr, res->ai_addrlen);
    *out = znet__from_sys(&ss);
    out->port = port;
    freeaddrinfo(res);
    return true;
}

void znet_addr_to_str(znet_addr addr, char *buf, size_t buf_len) 
{
    if (ZNET_IPV4 == addr.family) 
    {
        struct in_addr ia; 
        ia.s_addr = htonl(addr.ip.v4);
        inet_ntop(AF_INET, &ia, buf, (socklen_t)buf_len);
    } 
    else 
    {
        struct in6_addr ia; 
        memcpy(ia.s6_addr, addr.ip.v6, 16);
        inet_ntop(AF_INET6, &ia, buf, (socklen_t)buf_len);
    }
}

int znet_resolve(const char *hostname, uint16_t port, znet_addr *out_addr) 
{
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    char port_str[8]; 
    sprintf(port_str, "%u", port);
    
    if (0 != getaddrinfo(hostname, port_str, &hints, &res)) 
    {
        return Z_ERR;
    }
    
    struct sockaddr_storage ss;
    memcpy(&ss, res->ai_addr, res->ai_addrlen);
    *out_addr = znet__from_sys(&ss);
    freeaddrinfo(res);
    return Z_OK;
}

// Socket logic.

znet_socket znet_socket_create(znet_family family, znet_proto type) 
{
    int af = (family == ZNET_IPV4) ? AF_INET : AF_INET6;
    int st = (type == ZNET_TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int proto = (type == ZNET_TCP) ? IPPROTO_TCP : IPPROTO_UDP;
    
    ZNET__SOCKET s = socket(af, st, proto);
    if (ZNET__INVALID == s) 
    {
        return ZNET_INVALID_SOCKET;
    }
    return (znet_socket){ (uintptr_t)s, true };
}

void znet_close(znet_socket *s) 
{
    if (s->valid) 
    { 
        ZNET__CLOSE_FN((ZNET__SOCKET)s->handle); 
        s->valid = false; 
    }
}

int znet_bind(znet_socket s, znet_addr addr) 
{
    struct sockaddr_storage ss; socklen_t len;
    znet__to_sys(addr, &ss, &len);
    if (ZNET__ERROR == bind((ZNET__SOCKET)s.handle, (struct sockaddr*)&ss, len)) 
    {
        return Z_ERR;
    }
    return Z_OK;
}

int znet_connect(znet_socket s, znet_addr addr) 
{
    struct sockaddr_storage ss; socklen_t len;
    znet__to_sys(addr, &ss, &len);
    if (ZNET__ERROR == connect((ZNET__SOCKET)s.handle, (struct sockaddr*)&ss, len)) 
    {
        return Z_ERR;
    }
    return Z_OK;
}

int znet_listen(znet_socket s, int backlog) 
{
    if (ZNET__ERROR == listen((ZNET__SOCKET)s.handle, backlog)) 
    {
        return Z_ERR;
    }
    return Z_OK;
}

znet_socket znet_accept(znet_socket s, znet_addr *out_client_addr) 
{
    struct sockaddr_storage ss; 
    socklen_t len = sizeof(ss);
    ZNET__SOCKET c = accept((ZNET__SOCKET)s.handle, (struct sockaddr*)&ss, &len);
    
    if (ZNET__INVALID == c) 
    {
        return ZNET_INVALID_SOCKET;
    }
    if (out_client_addr) 
    {
        *out_client_addr = znet__from_sys(&ss);
    }
    
    return (znet_socket){ (uintptr_t)c, true };
}

// I/O logic.

z_ssize_t znet_send(znet_socket s, const void *data, size_t len) 
{
    return send((ZNET__SOCKET)s.handle, (const char*)data, (int)len, 0);
}

z_ssize_t znet_recv(znet_socket s, void *buf, size_t len) 
{
    return recv((ZNET__SOCKET)s.handle, (char*)buf, (int)len, 0);
}

z_ssize_t znet_sendto(znet_socket s, const void *data, size_t len, znet_addr dest) 
{
    struct sockaddr_storage ss; socklen_t slen;
    znet__to_sys(dest, &ss, &slen);
    return sendto((ZNET__SOCKET)s.handle, (const char*)data, (int)len, 0, (struct sockaddr*)&ss, slen);
}

z_ssize_t znet_recvfrom(znet_socket s, void *buf, size_t len, znet_addr *out_sender) 
{
    struct sockaddr_storage ss; 
    socklen_t slen = sizeof(ss);
    z_ssize_t res = recvfrom((ZNET__SOCKET)s.handle, (char*)buf, (int)len, 0, (struct sockaddr*)&ss, &slen);
    
    if (res > 0 && out_sender) 
    {
        *out_sender = znet__from_sys(&ss);
    }
    return res;
}

int znet_poll(znet_socket s, znet_poll_flags wait_for, int timeout_ms) 
{
    if (!s.valid) 
    {
        return -1;
    }
    fd_set read_fds, write_fds, err_fds;
    FD_ZERO(&read_fds); FD_ZERO(&write_fds); FD_ZERO(&err_fds);

    if (wait_for & ZNET_POLL_READ)  
    {
        FD_SET((ZNET__SOCKET)s.handle, &read_fds);
    }

    if (wait_for & ZNET_POLL_WRITE) 
    {
        FD_SET((ZNET__SOCKET)s.handle, &write_fds);
    }

    if (wait_for & ZNET_POLL_ERR)   
    {
        FD_SET((ZNET__SOCKET)s.handle, &err_fds);
    }

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    // First arg (maxfd) is ignored on Windows, but is required on POSIX.
    int max_fd = (int)s.handle + 1;
    int res = select(max_fd, &read_fds, &write_fds, &err_fds, timeout_ms < 0 ? NULL : &tv);

    if (res <= 0) 
    {
        return res; // 0 = Timeout, -1 = Error
    }

    int ret_flags = 0;
    if (FD_ISSET((ZNET__SOCKET)s.handle, &read_fds)) 
    {
        ret_flags |= ZNET_POLL_READ;
    }

    if (FD_ISSET((ZNET__SOCKET)s.handle, &write_fds)) 
    {
        ret_flags |= ZNET_POLL_WRITE;
    }
    if (FD_ISSET((ZNET__SOCKET)s.handle, &err_fds))
    {
        ret_flags |= ZNET_POLL_ERR;
    }
    
    return ret_flags;
}

// Configuration logic.

int znet_set_timeout(znet_socket s, int ms) 
{
#   ifdef _WIN32
    DWORD t = ms;
    setsockopt((ZNET__SOCKET)s.handle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&t, sizeof(t));
    setsockopt((ZNET__SOCKET)s.handle, SOL_SOCKET, SO_SNDTIMEO, (const char*)&t, sizeof(t));
#   else
    struct timeval tv = { ms / 1000, (ms % 1000) * 1000 };
    setsockopt((ZNET__SOCKET)s.handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt((ZNET__SOCKET)s.handle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#   endif
    return Z_OK;
}

int znet_set_nonblocking(znet_socket s, bool enable) 
{
#   ifdef _WIN32
    u_long mode = enable; 
    ioctlsocket((ZNET__SOCKET)s.handle, FIONBIO, &mode);
#   else
    int f = fcntl((ZNET__SOCKET)s.handle, F_GETFL, 0);
    fcntl((ZNET__SOCKET)s.handle, F_SETFL, enable ? (f | O_NONBLOCK) : (f & ~O_NONBLOCK));
#   endif
    return Z_OK;
}

int znet_set_dual_stack(znet_socket s, bool enable) 
{
    if (!s.valid) 
    {
        return Z_ERR;
    }
    int opt = enable ? 0 : 1; 
    return setsockopt((ZNET__SOCKET)s.handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&opt, sizeof(opt));
}

// HTTP extensions.

#ifdef ZNET_HAS_ZSTR
zstr znet_http_get(const char *domain, const char *path, int timeout_ms) 
{
    zstr resp = zstr_init();
    znet_addr addr;
    if (Z_OK != znet_resolve(domain, 80, &addr)) 
    {
        return resp;
    }
    
    znet_socket s = znet_socket_create(addr.family, ZNET_TCP);
    if (timeout_ms > 0) 
    {
        znet_set_timeout(s, timeout_ms);
    }
    
    if (Z_OK != znet_connect(s, addr)) 
    {
        znet_close(&s); 
        return resp; 
    }
    
    zstr req = zstr_init();
    zstr_fmt(&req, 
        "GET %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "User-Agent: znet/1.1\r\n"
        "Connection: close\r\n\r\n", 
        path, domain
    );
    
    znet_send(s, zstr_cstr(&req), zstr_len(&req));
    zstr_free(&req);
    
    char buf[4096]; 
    z_ssize_t n;
    while ((n = znet_recv(s, buf, sizeof(buf))) > 0) 
    {
        zstr_cat_len(&resp, buf, n);
    }
    
    znet_close(&s);
    return resp;
}
#endif

#endif // ZNET_IMPLEMENTATION_GUARD
#endif // ZNET_IMPLEMENTATION
