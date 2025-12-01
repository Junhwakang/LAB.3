#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 12345
#define BUF_SIZE 1024
#define MAX_CLIENTS  FD_SETSIZE   // select가 감시할 수 있는 최대 fd 수

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buf[BUF_SIZE];

    int client_socks[MAX_CLIENTS];
    int max_fd;          // select에서 사용할 현재 최대 fd 값
    fd_set read_fds;     // 읽기 이벤트 감시용 fd_set
    int i;

    // 1. 서버 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(1);
    }

    // 소켓 옵션: TIME_WAIT 시 재사용 가능하게
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("=== 멀티클라이언트 채팅 서버 시작 (포트 %d) ===\n", PORT);

    // 클라이언트 소켓 배열 초기화 (-1: 비어있음 표시)
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socks[i] = -1;
    }

    max_fd = server_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);  // 서버 소켓 감시

        // 연결된 모든 클라이언트 소켓을 read_fds에 추가
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_socks[i] != -1) {
                FD_SET(client_socks[i], &read_fds);
                if (client_socks[i] > max_fd) {
                    max_fd = client_socks[i];
                }
            }
        }

        // 3. select 호출: 읽기 가능한 소켓이 생길 때까지 대기
        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            break;
        }

        // 4-1. 새 클라이언트 접속 처리
        if (FD_ISSET(server_fd, &read_fds)) {
            client_len = sizeof(client_addr);
            client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd == -1) {
                perror("accept");
            } else {
                // 빈 자리 찾아서 등록
                int j;
                for (j = 0; j < MAX_CLIENTS; j++) {
                    if (client_socks[j] == -1) {
                        client_socks[j] = client_fd;
                        printf("[서버] 새 클라이언트 접속: fd=%d (%s:%d)\n",
                               client_fd,
                               inet_ntoa(client_addr.sin_addr),
                               ntohs(client_addr.sin_port));
                        break;
                    }
                }

                if (j == MAX_CLIENTS) {
                    printf("[서버] 클라이언트가 너무 많아 접속 거부\n");
                    close(client_fd);
                }
            }

            if (--ready <= 0) {
                continue;
            }
        }

        // 4-2. 기존 클라이언트들의 메시지 처리
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sock = client_socks[i];
            if (sock == -1) continue;

            if (FD_ISSET(sock, &read_fds)) {
                ssize_t n = read(sock, buf, sizeof(buf) - 1);
                if (n <= 0) {
                    // 연결 종료
                    printf("[서버] 클라이언트(fd=%d) 연결 종료\n", sock);
                    close(sock);
                    client_socks[i] = -1;
                } else {
                    buf[n] = '\0';
                    printf("[서버] 수신(fd=%d): %s", sock, buf);

                    // 받은 메시지를 모든 클라이언트에게 브로드캐스트
                    int k;
                    for (k = 0; k < MAX_CLIENTS; k++) {
                        int cs = client_socks[k];
                        if (cs != -1 && cs != sock) {
                            write(cs, buf, n);
                        }
                    }
                }

                if (--ready <= 0) {
                    break;
                }
            }
        }
    }

    // 정리
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_socks[i] != -1) {
            close(client_socks[i]);
        }
    }
    close(server_fd);
    return 0;
}
