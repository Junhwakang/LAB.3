#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUF_SIZE 1024

int main(void) {
    int sock;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE];

    // 1. 소켓 생성 (IPv4, TCP)
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

    // 3. 서버에 연결 (connect)
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sock);
        exit(1);
    }

    printf("=== TCP 클라이언트 시작 (서버 %s:%d) ===\n", SERVER_IP, PORT);
    printf("보낼 메시지를 입력하세요 (Ctrl+D 로 종료)\n");

    // 4. 표준입력에서 한 줄씩 읽어 서버로 전송, 다시 에코된 데이터 수신
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 서버로 전송
        write(sock, buf, strlen(buf));

        // 서버로부터 에코된 데이터 수신
        ssize_t n = read(sock, buf, sizeof(buf) - 1);
        if (n <= 0) {
            printf("서버 연결 종료\n");
            break;
        }
        buf[n] = '\0';
        printf("서버로부터 에코: %s", buf);
    }

    close(sock);
    printf("클라이언트 종료\n");
    return 0;
}
