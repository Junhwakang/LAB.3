#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUF_SIZE 4096

void error_exit(const char* msg) {
    perror(msg);
    exit(1);
}

// 한 줄 읽기 (CRLF까지)
int readline(int fd, char* buf, int size) {
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n')) {
        n = read(fd, &c, 1);
        if (n <= 0) break;
        if (c == '\r') {
            // CR 다음에 LF가 오면 소비
            n = read(fd, &c, 1);
            if (n > 0 && c != '\n') {
                // LF가 아니면 버퍼에 넣고 루프 계속
                buf[i++] = c;
            } else {
                break;
            }
        } else {
            buf[i++] = c;
        }
    }
    buf[i] = '\0';
    return i;
}

// 간단한 HTTP 응답 헤더 전송
void send_headers(int client, const char* status, const char* content_type) {
    char buf[BUF_SIZE];
    snprintf(buf, sizeof(buf),
             "HTTP/1.0 %s\r\n"
             "Server: mini_httpd\r\n"
             "Content-Type: %s\r\n"
             "\r\n",
             status, content_type);
    write(client, buf, strlen(buf));
}

// 404 에러 응답
void not_found(int client) {
    send_headers(client, "404 NOT FOUND", "text/html");
    const char* body =
        "<html><head><title>404 Not Found</title></head>"
        "<body><h1>404 Not Found</h1></body></html>";
    write(client, body, strlen(body));
}

// 500 에러 응답
void server_error(int client) {
    send_headers(client, "500 Internal Server Error", "text/html");
    const char* body =
        "<html><head><title>500 Internal</title></head>"
        "<body><h1>500 Internal Server Error</h1></body></html>";
    write(client, body, strlen(body));
}

// 정적 파일 전송
void serve_file(int client, const char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        not_found(client);
        return;
    }

    // 간단하게 확장자로 content-type 추정
    const char* ext = strrchr(path, '.');
    const char* type = "text/plain";
    if (ext) {
        if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            type = "text/html";
        } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
            type = "image/jpeg";
        } else if (strcmp(ext, ".png") == 0) {
            type = "image/png";
        }
    }

    send_headers(client, "200 OK", type);

    char buf[BUF_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        write(client, buf, n);
    }
    fclose(fp);
}

// CGI 실행 (GET/POST 공통)
void execute_cgi(int client,
                 const char* path,
                 const char* method,
                 const char* query_string,
                 int content_length,
                 int client_fd_for_read) {

    int cgi_input[2];
    int cgi_output[2];

    const char *status_line = "HTTP/1.0 200 OK\r\n";
    write(client, status_line, strlen(status_line));


    if (pipe(cgi_input) < 0 || pipe(cgi_output) < 0) {
        server_error(client);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        server_error(client);
        return;
    }

    if (pid == 0) {
        // 자식: CGI 프로그램 실행
        char meth_env[255];
        char query_env[1024];
        char length_env[255];

        // 표준입력/출력을 파이프로 변경
        dup2(cgi_input[0], 0);
        dup2(cgi_output[1], 1);
        close(cgi_input[1]);
        close(cgi_output[0]);

        snprintf(meth_env, sizeof(meth_env), "REQUEST_METHOD=%s", method);
        putenv(meth_env);

        if (strcasecmp(method, "GET") == 0 && query_string) {
            snprintf(query_env, sizeof(query_env), "QUERY_STRING=%s", query_string);
            putenv(query_env);
        } else if (strcasecmp(method, "POST") == 0) {
            snprintf(length_env, sizeof(length_env), "CONTENT_LENGTH=%d", content_length);
            putenv(length_env);
        }

        execl(path, path, NULL);
        // execl 실패 시
        perror("execl");
        exit(1);
    } else {
        // 부모: CGI와 통신
        close(cgi_input[0]);
        close(cgi_output[1]);

        // POST라면 클라이언트에서 body 읽어서 CGI stdin으로 전달
        if (strcasecmp(method, "POST") == 0 && content_length > 0) {
            char c;
            for (int i = 0; i < content_length; i++) {
                if (read(client_fd_for_read, &c, 1) > 0) {
                    write(cgi_input[1], &c, 1);
                }
            }
        }
        close(cgi_input[1]);

        // CGI 프로그램의 출력 읽어서 클라이언트에게 그대로 전달
        char buf[BUF_SIZE];
        int n;
        while ((n = read(cgi_output[0], buf, sizeof(buf))) > 0) {
            write(client, buf, n);
        }
        close(cgi_output[0]);

        waitpid(pid, NULL, 0);
    }
}

// 한 HTTP 요청 처리
void handle_client(int client_fd) {
    char buf[BUF_SIZE];
    char method[16];
    char url[256];
    char path[512];
    char* query_string = NULL;
    int content_length = 0;
    int cgi = 0;

    // 1. 요청 라인 읽기: "GET /path HTTP/1.1"
    int n = readline(client_fd, buf, sizeof(buf));
    if (n <= 0) {
        return;
    }

    printf("[요청] %s\n", buf);

    // method, url 파싱
    sscanf(buf, "%15s %255s", method, url);

    // GET/POST만 지원
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST")) {
        send_headers(client_fd, "501 Not Implemented", "text/html");
        const char* body = "<h1>501 Not Implemented</h1>";
        write(client_fd, body, strlen(body));
        return;
    }

    // GET이면 쿼리스트링 분리
    if (strcasecmp(method, "GET") == 0) {
        char* p = strchr(url, '?');
        if (p) {
            *p = '\0';
            query_string = p + 1;
            cgi = 1;
        }
    }

    // 기본 경로 설정
    snprintf(path, sizeof(path), ".%s", url);
    if (path[strlen(path) - 1] == '/') {
        strcat(path, "index.html");
    }

    // 헤더 읽기 (POST면 content-length 읽기)
    while (1) {
        n = readline(client_fd, buf, sizeof(buf));
        if (n <= 0) break;
        if (strcmp(buf, "") == 0) break;  // 헤더 끝

        // Content-Length 파싱
        if (strncasecmp(buf, "Content-Length:", 15) == 0) {
            content_length = atoi(buf + 15);
        }
    }

    // CGI 여부 결정: /cgi-bin/ 아래는 무조건 CGI로 취급
    if (strncmp(path, "./cgi-bin/", 10) == 0) {
        cgi = 1;
    }

    struct stat st;
    if (stat(path, &st) < 0) {
        not_found(client_fd);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        strcat(path, "/index.html");
        if (stat(path, &st) < 0) {
            not_found(client_fd);
            return;
        }
    }

    if (cgi) {
        // CGI는 응답 헤더는 CGI 프로그램이 직접 출력한다고 가정
        // (혹은 여기서 200 OK 헤더를 먼저 보내도 됨)
        execute_cgi(client_fd, path, method, query_string, content_length, client_fd);
    } else {
        // 정적 파일 서비스
        serve_file(client_fd, path);
    }
}

int main(int argc, char* argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int port = 8080;

    if (argc == 2) {
        port = atoi(argv[1]);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        error_exit("socket");
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        error_exit("bind");
    }

    if (listen(server_fd, 5) == -1) {
        error_exit("listen");
    }

    printf("=== mini_httpd 실행 (포트 %d) ===\n", port);
    printf("정적 파일: 현재 디렉토리 기준\n");
    printf("CGI: ./cgi-bin/ 아래 실행 파일 (예: /cgi-bin/test.cgi)\n");

    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("클라이언트 접속: %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        handle_client(client_fd);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
