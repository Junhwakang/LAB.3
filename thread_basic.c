#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 5

// 쓰레드 관련 함수들을 사용하여 프로그램을 작성하고 실행하여 보고 익숙해지도록 사용해 본다.

void* worker(void* arg) {
    int thread_id = *(int*)arg;

    for (int i = 0; i < 3; i++) {
        printf("[Thread %d] iteration %d\n", thread_id, i);
        // 잠깐 쉬어서 출력이 섞이는 걸 확인
        usleep(100 * 1000);  // 100ms
    }

    printf("[Thread %d] 종료\n", thread_id);
    return NULL; // pthread_exit(NULL) 과 동일
}

int main(void) {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    printf("메인 쓰레드: %d개의 쓰레드 생성 시작\n", NUM_THREADS);

    // 1. 쓰레드 생성
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        int ret = pthread_create(&threads[i], NULL, worker, &ids[i]);
        if (ret != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    // 2. 쓰레드 종료 대기 (join)
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("메인 쓰레드: 모든 쓰레드 종료, 프로그램 끝\n");
    return 0;
}
