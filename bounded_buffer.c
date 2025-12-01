#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5          // 제한 버퍼 크기
#define NUM_PRODUCERS 2        // 생산자 쓰레드 수
#define NUM_CONSUMERS 2        // 소비자 쓰레드 수
#define ITEMS_PER_PRODUCER 10  // 생산자 1명당 생산할 아이템 개수
// 쓰레드를 사용하여 생산자 소비자 문제를 해결하는 제한버퍼를 생성하고 활용하는 프로그램을 구현하시오.
// 총 아이템 개수 = 생산자 수 * 생산자당 아이템 개수
#define TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

// 공유 버퍼 및 관련 변수
int buffer[BUFFER_SIZE];
int in = 0;      // 다음에 쓸 위치 (생산자 인덱스)
int out = 0;     // 다음에 읽을 위치 (소비자 인덱스)
int count = 0;   // 버퍼에 현재 들어있는 아이템 개수

// 동기화 도구
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_not_empty = PTHREAD_COND_INITIALIZER;

void* producer(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        // 생산할 데이터 (간단히 id와 i를 이용)
        int item = id * 100 + i;

        // 버퍼에 넣기 위해 lock
        pthread_mutex_lock(&mutex);

        // 버퍼가 가득 차면, 비워질 때까지 대기
        while (count == BUFFER_SIZE) {
            printf("[생산자 %d] 버퍼 가득 참, 대기 중...\n", id);
            pthread_cond_wait(&cond_not_full, &mutex);
        }

        // 실제로 버퍼에 아이템 삽입
        buffer[in] = item;
        printf("[생산자 %d] 생산: %d (buffer[%d])\n", id, item, in);
        in = (in + 1) % BUFFER_SIZE;
        count++;

        // 소비자에게 "비어있지 않다"는 신호 전달
        pthread_cond_signal(&cond_not_empty);

        // lock 해제
        pthread_mutex_unlock(&mutex);

        // 보기 좋게 약간 쉬면서 실행
        usleep(100 * 1000); // 100ms
    }

    printf("[생산자 %d] 생산 종료\n", id);
    return NULL;
}

void* consumer(void* arg) {
    int id = *(int*)arg;
    int items_to_consume = TOTAL_ITEMS / NUM_CONSUMERS;

    for (int i = 0; i < items_to_consume; i++) {
        pthread_mutex_lock(&mutex);

        // 버퍼가 비어있으면, 채워질 때까지 대기
        while (count == 0) {
            printf("    [소비자 %d] 버퍼 비어 있음, 대기 중...\n", id);
            pthread_cond_wait(&cond_not_empty, &mutex);
        }

        // 버퍼에서 아이템 꺼내기
        int item = buffer[out];
        printf("    [소비자 %d] 소비: %d (buffer[%d])\n", id, item, out);
        out = (out + 1) % BUFFER_SIZE;
        count--;

        // 생산자에게 "가득 차 있지 않다"는 신호 전달
        pthread_cond_signal(&cond_not_full);

        pthread_mutex_unlock(&mutex);

        usleep(150 * 1000); // 150ms
    }

    printf("    [소비자 %d] 소비 종료\n", id);
    return NULL;
}

int main(void) {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];

    printf("=== 제한 버퍼(BOUND BUFFER) 생산자-소비자 프로그램 시작 ===\n");
    printf("버퍼 크기: %d, 생산자: %d, 소비자: %d, 총 아이템: %d\n\n",
           BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS, TOTAL_ITEMS);

    // 생산자 쓰레드 생성
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer, &producer_ids[i]) != 0) {
            perror("pthread_create(producer)");
            exit(1);
        }
    }

    // 소비자 쓰레드 생성
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]) != 0) {
            perror("pthread_create(consumer)");
            exit(1);
        }
    }

    // 모든 생산자 종료 대기
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    // 모든 소비자 종료 대기
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    printf("\n=== 모든 생산자/소비자 종료, 프로그램 끝 ===\n");
    return 0;
}
