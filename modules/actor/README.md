# Actor

This is an implementation of Actor Model which is a conceptual concurrent computation model. Check out what Actor Model is [here in wikipedia](https://en.wikipedia.org/wiki/Actor_model).

## Overview

Two ways to send a message to actors:
1. by actor: single actor
2. by queue: multiple actors

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

```c
struct actor_msg {
    int id;
};

struct actor my_actor1;
struct actor_queue my_queue1;
uint8_t buf[256];

static void my_actor1_handler(struct actor *actor, struct actor_msg *msg) {
    struct my_msg *my_msg = (struct my_msg *)msg;
    printf("actor called %p with msg id: %d", actor, my_msg->id);
}

actor_boot(buf, sizeof(buf));
actor_queue_init(&my_queue1);
actor_init(&my_actor1, my_actor1_handler, 0, &my_queue1);

struct actor_msg *first_msg = actor_alloc(sizeof(*first_msg));
first_msg->id = 1;
actor_send(&my_actor1, first_msg);
```
