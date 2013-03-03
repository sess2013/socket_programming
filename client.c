#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>

#define ECHO_PORT 4000

int tty_cbreak(int fd, int set);
void main(int argc, char *argv[])
{
	int sock,child_pid;
	struct hostent *host;
	struct sockaddr_in serv_addr;
	int port;
	char addr[50];
	char hostname[50];
	char buf[100];
	if(argc != 2)
	{
		fprintf(stderr,"usage: %s<address>＼n",argv[0]);
		exit(1);
	}
	strcpy(addr,argv[1]);
	port = ECHO_PORT;

	bzero((char *)&serv_addr, sizeof(serv_addr));	
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port = htons(port);
	/*숫자와 점으로 이루어진 주소인지 체크한다.*/
	if ((serv_addr.sin_addr.s_addr=inet_addr(addr))!=INADDR_NONE)

	{
		strcpy(hostname,addr);
	}
	else
	{/*문자로 이루어진 주소값 이므로, 데이터베이스에서 주소를 찾는다.*/
		if ((host=gethostbyname(addr))==NULL)
		{
			herror("host name error");
			exit(1);
		}
		bcopy(host->h_addr,(char *)&serv_addr.sin_addr,
				host->h_length);
		strcpy(hostname, host->h_name);
	}
	/*소켓 생성*/
	if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		exit(1);
	}
	printf("Trying %s...＼n",inet_ntoa(serv_addr.sin_addr));
	/*서버로 연결을 시도한다. */
	if(connect(sock,(struct sockaddr*) &serv_addr, sizeof(serv_addr))
			<0)
	{
		close(sock);
		perror("connect");
		exit(1);
	}
	printf("Connected to %s.＼", hostname);
	if((child_pid = fork()) ==0)
	{/*this is the child process*/
		int n;
		while((n = read(sock, buf, 100)) !=0)
			write(1,buf,n);
		if(kill(getppid(), SIGKILL)<0)
			perror("kill");
	}
	else
	{
		/*this is the parent process*/
		int n;
		/* cbreak-mode로 만든다.*/
		tty_cbreak(fileno(stdin),1);
		while((n = read(fileno(stdin),buf,1)) == 1)
		{
			if((n = write(sock, buf, 1)) <0)
				break;
		}
		if(n<0)
			fprintf(stderr, "error!＼n");
		/*terminal mode 복귀시킨다.*/
		tty_cbreak(fileno(stdin),0);
		kill(child_pid, SIGKILL);
	}
	close(sock);
	printf("Connection closed.＼n");
}
/*cbreak mode*/
int tty_cbreak(int fd, int set)
{
	static struct termios save_termios;
	struct termios buf;
	/*set이 1이면 cbreak mode로 전환, 0이면 이전의 모드로 되돌림*/
	if(!set)
	{
		if(tcsetattr(fd, TCSAFLUSH, &save_termios)<0)
			return -1;
		return 0;
	}
	if(tcgetattr(fd, &save_termios)<0)
		return -1;
	buf = save_termios;
	buf.c_lflag &=~(ECHO | ICANON);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
	if(tcsetattr(fd, TCSAFLUSH, &buf)<0)
		return -1;
	return 0;
}
