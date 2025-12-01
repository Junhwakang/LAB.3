#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main(void) {
    int sock;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE];

    // 1. 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(1);
    }

    // 3. 서버에 연결
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        exit(1);
    }

    printf("=== 채팅 클라이언트 시작 (서버 %s:%d) ===\n", SERVER_IP, PORT);
    printf("메시지를 입력하면 다른 클라이언트에게 전달됩니다. (Ctrl+D 로 종료)\n");

    // 4. fork로 송신/수신 프로세스 분리
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(sock);
        exit(1);
    }

    if (pid == 0) {
        // 자식 프로세스: 서버에서 오는 메시지를 계속 읽어서 출력 (수신 전담)
        while (1) {
            ssize_t n = read(sock, buf, sizeof(buf) - 1);
            if (n <= 0) {
                printf("서버와의 연결이 종료되었습니다.\n");
                break;
            }
            buf[n] = '\0';
            printf("[수신] %s", buf);
        }
        close(sock);
        exit(0);
    } else {
        // 부모 프로세스: 키보드 입력을 읽어서 서버로 전송 (송신 전담)
        while (fgets(buf, sizeof(buf), stdin) != NULL) {
            write(sock, buf, strlen(buf));
        }

        // 입력 종료(Ctrl+D) 시 소켓 닫고 자식 종료 기다림
        close(sock);
        printf("클라이언트 종료 요청. 자식 프로세스 종료 대기...\n");
        wait(NULL);
    }

    return 0;
}
