#if defined(_WIN32)

#include <winsock.h>

inline int setsockopt(SOCKET s, int level, int optname, void *val, int optlen)
{
	return ::setsockopt(s, level, optname, (const char*)val, optlen);
}

inline int close(int socket)
{
	return ::closesocket(socket);
}

inline int read(int socket, void *buf, int len)
{
	return ::recv(socket, (char*)buf, len, 0);
}

inline void usleep(DWORD us)
{
	Sleep(us / 1000);
}

#define signal(x, y) { }

typedef int socklen_t;

#else

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>

#endif
