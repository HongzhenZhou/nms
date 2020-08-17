
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>

int sharpd = 0;

void work() 
{
	printf("start nei-nms\n");
	system("/root/src/nms/bin/nei-nms");
	kill(getppid(), SIGUSR1);
	printf("nei-nms error termed\n");
	exit(0);
}

void sharp(int sig)
{
	sharpd = 1;	
}

int main()
{
	int fd;
	struct sockaddr_in addr;
	int val = 1;

	fd = socket(PF_INET, SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(33333);
	addr.sin_addr.s_addr = 0;	
	bind(fd, (struct sockaddr *)&addr, sizeof addr);	

	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(int));
	
	signal(SIGCHLD, SIG_IGN);
	
	for (;;) {
		struct sockaddr_in cli;
		struct pollfd pfd[1];
		int cfd, len = 0;
		pid_t pid;
		unsigned char *p = (unsigned char *)&cli.sin_addr.s_addr;
		
		memset(&cli, 0, sizeof cli);	
		cli.sin_family = AF_INET;
		len = sizeof cli;

		printf("we are listening\n");
		if (-1 == listen(fd, 1)) {
			printf("listen error\n");
			continue;
		}
			
		if (-1 == (cfd=accept(fd, (struct sockaddr *)&cli, &len))) {
			printf("accept error\n");
			continue;
		}

		printf("accept a connect from %d.%d.%d.%d:%d\n",
			p[0], p[1], p[2], p[3], ntohs(cli.sin_port));

		sharpd = 0;
		signal(SIGUSR1, sharp);
		
		if (0 == (pid=fork()))
			work();
		
		pfd[0].fd = cfd;
		pfd[0].events = POLLIN;
	
again:	
		if (sharpd) {
			close(cfd);
			continue;
		}
		poll(pfd, 1, -1);
		if (sharpd) {
			close(cfd);
			continue;
		}

		if (POLLIN & pfd[0].revents) {
			char buf[80];
			
			if (sharpd) {
				close(cfd);
				continue;
			}
			if (0 == read(cfd, buf, sizeof buf)) {
				close(cfd);
				printf("stop nei-nms\n");
				signal(SIGUSR1, SIG_IGN);
				system("killall -9 nei-nms");
				continue;
			} 
			printf("normal data\n");
		} 
		if (POLLERR & pfd[0].revents) {
			close(cfd);
			printf("stop nei-nms\n");
			signal(SIGUSR1, SIG_IGN);
			system("killall -9 nei-nms");
			continue;
		}
		
		goto again;
	}

	return 0;
}


