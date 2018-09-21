#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
int evutil_socketpair(int family, int type, int protocol, int fd[2])
{
	/* This code is originally from Tor.  Used with permission. */
	/* This socketpair does not work when localhost is down. So
	 * it's really not the same thing at all. But it's close enough
	 * for now, and really, when localhost is down sometimes, we
	 * have other problems too.
	 */
	int listener = -1;
	int connector = -1;
	int acceptor = -1;
	struct sockaddr_in listen_addr;
	struct sockaddr_in connect_addr;
	int size;
	int saved_errno = -1;

	if (protocol
#ifdef AF_UNIX
		|| family != AF_UNIX
#endif
		) {
		return -1;
	}
	if (!fd) {
		return -1;
	}

	listener = socket(AF_INET, type, 0);
	if (listener < 0)
		return -1;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	listen_addr.sin_port = 0;	/* kernel chooses port.	 */

	if (bind(listener, (struct sockaddr *) &listen_addr, sizeof (listen_addr))
		== -1)
		goto tidy_up_and_fail;
	printf("listen_addr.sin_addr.s_addr=0x%x string=%s--listen_addr.sin_port=%d\n", listen_addr.sin_addr.s_addr, (char*)inet_ntoa(listen_addr.sin_addr), listen_addr.sin_port);
	if (listen(listener, 1) == -1)
		goto tidy_up_and_fail;

	connector = socket(AF_INET, type, 0);
	if (connector < 0)
		goto tidy_up_and_fail;
	/* We want to find out the port number to connect to.  */
	size = sizeof(connect_addr);
	if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;
	printf("connect_addr.sin_addr.s_addr=0x%x string=%s--connect_addr.sin_port=%d\n", connect_addr.sin_addr.s_addr, (char*)inet_ntoa(connect_addr.sin_addr), connect_addr.sin_port);
	if (size != sizeof (connect_addr))
		goto abort_tidy_up_and_fail;
	if (connect(connector, (struct sockaddr *) &connect_addr,
				sizeof(connect_addr)) == -1)
		goto tidy_up_and_fail;

	size = sizeof(listen_addr);
	acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
	if (acceptor < 0)
		goto tidy_up_and_fail;
	if (size != sizeof(listen_addr))
		goto abort_tidy_up_and_fail;
	/* Now check we are talking to ourself by matching port and host on the
	   two sockets.	 */
	printf("listen_addr.sin_addr.s_addr=0x%x string=%s--listen_addr.sin_port=%d\n", listen_addr.sin_addr.s_addr, (char*)inet_ntoa(listen_addr.sin_addr), listen_addr.sin_port);
	if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;
	printf("connect_addr.sin_addr.s_addr=0x%x string=%s--connect_addr.sin_port=%d\n", connect_addr.sin_addr.s_addr, (char*)inet_ntoa(connect_addr.sin_addr), connect_addr.sin_port);
	if (size != sizeof (connect_addr)
		|| listen_addr.sin_family != connect_addr.sin_family
		|| listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
		|| listen_addr.sin_port != connect_addr.sin_port)
		goto abort_tidy_up_and_fail;
	fd[0] = connector;
	fd[1] = acceptor;

	return 0;

 abort_tidy_up_and_fail:

 tidy_up_and_fail:

 return -1;
}


void* work_thread(void* argc)
{
	int* fd = (int*)argc;
	while(1)
	{
		char c[50];
		int n = recv(fd[1], &c, sizeof(c), 0);
		printf("recv%dbyte=%s\n", n, c);
		//	sleep(1);
	   	char b[]="Goodbye!";
		n = send(fd[1], &b, sizeof(b), 0);
		printf("send%dbyte=%s\n", n, b);
	}
}


int main(int argc, char* argv[])
{
   int socket_pair[2];
   pthread_t tid;
   /**
   if (evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair) == -1)
   {
      perror("create socketpair failed\n");
	  return -1;
   }
**/
   if(socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair) == -1)
   {
	   perror("create socketpair failed\n");
	   return -1;
   }

   pthread_create(&tid, NULL,
				  work_thread, &socket_pair[0]);
   while(1)
   {
	   char c[] = "Hello,thread!";
	   int n = send(socket_pair[0], &c, sizeof(c), 0);
	   printf("++send%dbyte=%s\n", n, c);
	   sleep(1);
	   n = recv(socket_pair[0], &c, sizeof(c), 0);
	   printf("++recv%dbyte=%s\n", n, c);
   }
   pthread_join(tid, NULL);
   close(socket_pair[0]);
   close(socket_pair[1]);
   return 0;
}
