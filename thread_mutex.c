#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4
#define NUM_LOOP    1000000

long long counter = 0;             // 공유 변수
pthread_mutex_t counter_mutex;     // 뮤텍스

void* unsafe_worker(void* arg) {
    for (int i = 0; i < NUM_LOOP; i++) {
        // 보호 없이 증가 → race condition 발생 가능
        counter++;
    }
    return NULL;
}

void* safe_worker(void* arg) {
    for (int i = 0; i < NUM_LOOP; i++) {
        pthread_mutex_lock(&counter_mutex);   // 뮤텍스 잠금
        counter++;
        pthread_mutex_unlock(&counter_mutex); // 뮤텍스 해제
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    // 1단계: 동기화 없이 실행해 보기
    counter = 0;
    printf("=== 1단계: mutex 없이 counter 증가 ===\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, unsafe_worker, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("기대값: %d, 실제 counter: %lld\n",
           NUM_THREADS * NUM_LOOP, counter);

    // 2단계: mutex 사용해서 실행
    printf("\n=== 2단계: mutex 사용하여 counter 증가 ===\n");

    // 뮤텍스 초기화
    if (pthread_mutex_init(&counter_mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }

    counter = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, safe_worker, NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("기대값: %d, 실제 counter: %lld\n",
           NUM_THREADS * NUM_LOOP, counter);

    // 뮤텍스 제거
    pthread_mutex_destroy(&counter_mutex);

    return 0;
}
