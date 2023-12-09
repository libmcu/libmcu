# Actor

This is an implementation of Actor Model which is a conceptual concurrent computation model. Check out what Actor Model is [here in wikipedia](https://en.wikipedia.org/wiki/Actor_model).

## Overview

- 하나의 큐에 여러개의 액터를 등록하면, pub/sub 구현이 가능
- 하나의 큐에 하나의 액터만 등록하면 구현과 설계가 간단
  - 하나의 메시지에 여러개의 액터가 행동해야 한다면, 하나의 액터가 그 일을 위임하게 하는 방법이 있음
- DSA 알고리즘 구현까지 하면 너무 복잡해지니 메시지 풀은 고정된 유닛으로 할당
- 하나의 액터가 여러 개의 큐를 구독할 필요가 있을까?
- 우선순위는 액터에게 있어야 하나? 메시지에 있어야 하나?
  - 메시지에 우선순위를 둔다면 큐에 삽입할 때 정렬해 삽입해야함
- 액터가 스케줄링 대상. 메시지가 아님

## Integration Guide

- ACTOR_DEFAULT_MESSAGE_SIZE
- ACTOR_PRIORITY_MAX 설정으로 실행 컨텍스트(스레드) 갯수를 지정할 수 있음. 디폴트는 1
- ACTOR_PRIORITY_BASE 를 기준으로 우선순위가 1씩 증가하거나 감소
- 낮은 번호가 높은 우선순위일 경우 ACTOR_PRIORITY_DESCENDING 정의
- 여러 액터가 하나의 큐를 사용해도 되지만, 이 경우 액터의 우선순위가 동일해야 함

```c
struct actor_msg {
    int id;
};

static uint8_t mem[256];
static uint8_t timer_mem[256];
static struct actor my_actor1;

static void my_actor1_handler(struct actor *actor, struct actor_msg *msg) {
    printf("actor called %p with msg id: %d", actor, msg->id);
    actor_free(msg);
}

int main(void) {
    actor_init(mem, sizeof(mem), 4096);
    actor_timer_init(timer_mem, sizeof(timer_mem));

    actor_set(&my_actor1, my_actor1_handler, 0);

    ...

    while (1) {
        struct actor_msg *msg = actor_alloc(sizeof(*msg));
        msg->id = 1;
        actor_send(&my_actor1, msg);

        sleep_ms(1000);
    }
}
```