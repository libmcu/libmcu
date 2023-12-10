# Actor

This is an implementation of Actor Model which is a conceptual concurrent computation model. Check out what Actor Model is [here in wikipedia](https://en.wikipedia.org/wiki/Actor_model).

All actors with the same priority share one execution context. So every actor should be implemented in the way of run-to-completion. This implementation decision is maid for the resource-constrained microcontrollers. You can run actors in another separated execution context as giving it a different priority.

## Integration Guide

- ACTOR_DEFAULT_MESSAGE_SIZE
    - Memory is allocated with fixed-size blocks defined by `ACTOR_DEFAULT_MESSAGE_SIZE` to avoid external fragmentation.
- ACTOR_PRIORITY_MAX
    - Threads are created according to the number of priorities. The default is 1.
- ACTOR_PRIORITY_BASE
    - Priority increases or decreases by 1 based on it. The default is 0.
    - If the lower number the higher priority, then define ACTOR_PRIORITY_DESCENDING. The default is ascending.

### Example

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
