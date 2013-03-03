/*echo server*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define ECHO_PORT 4000

int newsock;
void quit(int);
void main()
{
     int sock, clilen;
     struct sockaddr_in cli_addr,serv_addr;
     char buf[100];
     int n;
     /* internet 가족의 stream socket을 할당받는다. */
     if((sock = socket(AF_INET,SOCK_STREAM,0)) <0)
     {
         perror("socket");
         exit(1);
     }
     /*인터럽트(CNTL-C)시에 종료작업을 할 수 있게 핸들러를 설치한다.*/
     signal(SIGINT,quit);
     /*sockaddr_in 구조체를 사용하기 전엔 0으로 초기화 해주어야 한다. */
     bzero((char*) &serv_addr,sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     /* INADDR_ANY: 0의 값 -> 자신의 주소를 컴퓨터가 찾아준다. */
     serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     serv_addr.sin_port = htons(ECHO_PORT);
     /*bind local address */
     if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) <
     0)
     {
          perror("bind");
          close(sock);
          exit(1);
     }
     listen(sock, 0);
     clilen = sizeof(cli_addr);
     printf("accepting...\n");
     /* 여기서 연결요청이 올 때까지 block 된다. */
     newsock = accept(sock, (struct sockaddr*) & cli_addr, &clilen);
     close(sock);
     if(newsock<0)
     {
          perror("accept");
          exit(1);
     }
     printf("Connected from %s.\n", inet_ntoa(cli_addr.sin_addr));
     while((n = read(newsock, buf, 100)) != 0)
     {
          if(n < 0)
         {
                perror("read");
                close(newsock);
                exit(1);
          }
          write(newsock, buf, n);
          write(1, buf, n);
     }
     close(newsock);
     printf("Connection closed by the peer.\n");
}
void quit(int signum)
{
     close(newsock);
     printf("interrupted\n");
     exit(1);
}
