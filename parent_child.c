#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PRINTS 5   // 부모/자식이 각각 몇 번씩 출력할지

// 0이면 부모 차례, 1이면 자식 차례인 이진 플래그
int turn = 0;

// 동기화를 위한 뮤텍스와 조건변수
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;

// 자식 쓰레드 함수
void* child_thread(void* arg) {
    for (int i = 0; i < NUM_PRINTS; i++) {
        // 임계구역 진입을 위한 락
        pthread_mutex_lock(&mutex);

        // 내 차례(자식 차례 = 1)가 아니면 조건변수에서 대기
        while (turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }

        // 여기까지 왔다는 것은 자식 차례라는 뜻
        printf("hello child\n");

        // 이제 부모 차례로 넘김
        turn = 0;

        // 부모를 깨움
        pthread_cond_signal(&cond);

        // 임계구역 종료
        pthread_mutex_unlock(&mutex);

        // 1초 대기 (번갈아가며 1초 간격으로 출력)
        sleep(1);
    }

    return NULL;
}

int main(void) {
    pthread_t child;

    printf("=== 부모/자식 쓰레드 번갈아 인사 프로그램 시작 ===\n");

    // 자식 쓰레드 생성
    if (pthread_create(&child, NULL, child_thread, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    // 메인 쓰레드가 부모 역할을 수행
    for (int i = 0; i < NUM_PRINTS; i++) {
        // 임계구역 진입
        pthread_mutex_lock(&mutex);

        // 내 차례(부모 차례 = 0)가 아니면 기다림
        while (turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }

        // 부모 차례이므로 출력
        printf("hello parent\n");

        // 이제 자식 차례로 넘김
        turn = 1;

        // 자식을 깨움
        pthread_cond_signal(&cond);

        // 임계구역 종료
        pthread_mutex_unlock(&mutex);

        // 1초 대기
        sleep(1);
    }

    // 자식 쓰레드 종료 대기
    pthread_join(child, NULL);

    printf("=== 프로그램 종료 ===\n");
    return 0;
}
