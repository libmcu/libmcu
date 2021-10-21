# apptimer

## Overview
It implements hierarchical timing wheels. Insertion and deletion is worst case
O(1). Per-tick bookkeeping is also O(1), but is a little tricky because every
time wheel unit time passing, all the slots of lower wheels get updated, which
is not cache friendly.

Adjusting the number of wheels and slots, you might meet the requirements. e.g.
it would be simple timing wheels when `NR_WHEELS=1` with timeout limitation.
There is space-time tradeoff. The more slots the faster while the more slots the
more memory.

A timer takes 25 bytes on 32-bit system. 4 bytes more can be saved replacing
doubly linked list with singly linked list. There would be no performance
penalty to use singly linked list instead as long as slots are big enough for
timers to be well distributed.

## Integration Guide
