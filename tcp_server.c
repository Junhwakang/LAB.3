#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUF_SIZE 1024

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buf[BUF_SIZE];

    // 1. 소켓 생성 (IPv4, TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    // 2. 서버 주소 구조체 초기화 및 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 모든 NIC에서 수신
    server_addr.sin_port = htons(PORT);        // 포트 번호 (네트워크 바이트 순서로)

    // 3. 소켓에 주소 바인딩
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    // 4. 클라이언트 접속 대기 상태로 전환 (listen)
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("=== TCP 에코 서버 시작 (포트 %d) ===\n", PORT);

    while (1) {
        client_len = sizeof(client_addr);

        // 5. 클라이언트 접속 수락 (accept)
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("클라이언트 접속: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // 6. 데이터 수신 및 에코
        while (1) {
            ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
            if (n <= 0) {
                printf("클라이언트 연결 종료\n");
                break;
            }

            buf[n] = '\0';
            printf("수신: %s", buf);   // buf에 개행이 포함되어 있을 수 있음

            // 받은 내용을 그대로 다시 전송 (echo)
            write(client_fd, buf, n);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
