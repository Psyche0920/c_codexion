*This project has been created as part of the 42 curriculum by wehan.*
# Codexion
## Description

Codexion simulates multiple coders competing for shared USB dongles in a circular co-working hub. Each coder must acquire two adjacent dongles to compile, then debug and refactor in cycles.

The simulation ends when a coder burns out or when all coders complete the required number of compiles.

Key techniques demonstrated:

- POSIX threads and mutex-based synchronization
- FIFO and EDF scheduling
- Binary min-heap priority queues
- Dongle cooldown handling
- Condition-variable waiting and wakeup
- Burnout detection with a monitor thread
- Serialized logging
- Deadlock and starvation prevention

---

## Instructions

### Compilation

```bash
make
```

The project compiles with:
```bash
-Wall -Wextra -Werror -pthread
```

Binary name:
```bash
./codexion
```

### Execution
```bash
./codexion <number_of_coders> <time_to_burnout> <time_to_compile> \
           <time_to_debug> <time_to_refactor> \
           <number_of_compiles_required> <dongle_cooldown> <scheduler>
```

Arguments:

- `number_of_coders`: number of coder threads and dongles
- `time_to_burnout`: max ms since the beginning of the last compile before burnout
- `time_to_compile`: ms spent compiling while holding two dongles
- `time_to_debug`: ms spent debugging
- `time_to_refactor`: ms spent refactoring
- `number_of_compiles_required`: compiles each coder must complete
- `dongle_cooldown`: ms after release before a dongle can be reacquired
- `scheduler`: `fifo` or `edf`

### Output Format
```text
<timestamp_ms> <coder_id> has taken a dongle
<timestamp_ms> <coder_id> has taken a dongle
<timestamp_ms> <coder_id> is compiling
<timestamp_ms> <coder_id> is debugging
<timestamp_ms> <coder_id> is refactoring
<timestamp_ms> <coder_id> burned out
```

Timestamps are measured in milliseconds since simulation start.

### cleanup
```bash
make fclean
```

---

## Scheduler
Each dongle owns a request heap. A request stores:
```c
coder_id
arrival_time
deadline
```

### FIFO
FIFO uses the actual request arrival time:
```c
arrival_time = get_time_ms();
```

Because threads run concurrently, FIFO does not mean coder IDs are served in order. It means the request that actually entered the dongle heap first has higher priority.

### EDF
EDF uses the burnout deadline:
```c
deadline = last_compile_start + time_to_burnout;
```

The coder closest to burnout receives higher priority.

A coder can still burn out under FIFO, because FIFO does not care who is closest to burnout. EDF reduces this risk when parameters are feasible, but no scheduler can avoid burnout if the timing is impossible.

---

## Binary Heap Priority Queque

A binary min-heap stores requests with the highest-priority request at index `0`.

Heap relations:
```text
parent      = (i - 1) / 2
left child  = 2i + 1
right child = 2i + 2
```

Opeartions implemented:
- `heap_push`
- `heap_peek`
- `heap_remove`
- `heap_up`
- `heap_down`

This heap is stored in a malloc-allocated array. Each dongle has:
- `size`: current number of requests
- `capacity`: maximum number of requests, set to `number_of_coders`
This avoids linnk-node complexity and keeps heap storage contiguous.

---

## Blocking Cases Handled

### Deadlock Prevention
Dongles are requested in fixed_index order:
```text
lower-index dongle first
```

This avoids circular wait.

The single-coder case is handled separately. With one coder there is only one dongle, so the coder can take one dongle but cannot compile. This avoids locking the same mutex twice and self-deadlocking.

### Dongle Duplication Prevention
Each dongle owns a mutex protecting:
- `occupied`
- `available_at`
- request heap
- condition variable state

This prevents two coders from taking or modifying the same dongle at the same time.

### Starvation Prevention
A coder must acquire two dongles to compile. A previous issue was that a coder could hold one dongle while waiting too long for the second.

The solution:
- try to acquire the first dongle
- try to actquire the second dongle with a short timed wait
- if the second dongle cannot be acquired, release the first dongle
- retry later

This avoids holding one dongle forever.

### Cooldown Handling
When a dongle is released:
```c
dongle->available_at = get_time_ms() + dongle_cooldown;
```

A coder can only take it when:

```c
get_time_ms() >= dongle->available_at
```

Cooldown itself does not automatically wake a waiting thread.
Threads wake through broadcast or timed wait timeout, then recheck the cooldown condition.

### Burnout Detection
Each coder stores:
```c
last_compile_start
```
At initialization, this is set to the beginning of the simulation. Each time the coder starts compiling, it is updated again.

This monitor checks:
```c
get_time_ms() - last_compile_start > time_to_burnout
```

The timer start from the beginning of the last compile, not the end.
```c
usleep(1000);
```

This is used to keep burnout detection within the required 10ms tolerance, allowing small OS scheduling variation.

### Stop Wakeup
If the simulation stops while coders are waiting in 
`pthread_cond_wait()` or `pthread_cond_timedwait()`, they do not wake automatically.

Therefore Therefore `set_stop()` broadcasts to all dongle condition variables so waiting coders can wake, recheck `stop`, and exit safely.

### Log Serialization
All output is protected by `log_mutex`, so log lines do not interleave.

---

## Thread Synchronization Mechanisms
### Mutexes

Three mutex responsibilities are used:
- `dongle->mutex`: protects one dongle's state and heap
- `state_mutex`: protects `stop`, `last_compile_start`, and shared simulation state
- `log_mutex`: protects output

### Condition Variables
Each dongle has a condition variable.

Coders wait when:
- the dongle is occupied
- cooldown has not passed
- another coder is at the heap root
- the simulation is still running

A wakeup does not mean the coder can immediately take the dongle. The condition is always checked again in a loop.

### Timed Wait
`pthread_cond_timedwait()` is used when trying to avoid holding one dongle forever.

It expects a `struct timespec` absolute timeout:
 ```c
struct timespec ts;
```

`timespec` uses:

- seconds
- nanoseconds

---

## Memory Management
Dynamic allocation is used because `number_of_coders` is only known at runtime.

Allocated structures include:
- coder array
- dongle array
- one request heap per dongle

All allocated memory is freed before exit.

Example Valgrind result:
```text
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors
```

---

## Testing
### Normal FIFO completion

```bash
./codexion 4 5000 200 300 500 3 5 fifo
```

Expected: all coders complete 3 compiles without burnout.

Check compile count:

```bash
./codexion 4 5000 200 300 500 3 5 fifo | grep "is compiling" | wc -l
```

Expected output:

```text
12
```

### EDF Scheduling
```bash
./codexion 4 5000 200 300 500 3 5 edf
```

Expected: feasible EDF execution without starvation.

### Single Coder

```bash
./codexion 1 300 200 300 500 3 5 fifo
```

Expected: the coder takes one dongle and eventually burns out without self-deadlock.

### Burnout Case
```bash
./codexion 4 300 200 300 500 3 5 fifo
```

Expected: at least one coder burns out because the timing is infeasible. The exact coder ID may vary because thread scheduling is nondeterministic.

### Valgrind
```bash
valgrind --leak-check=full ./codexion 4 5000 200 300 500 3 5 fifo
```

---

## Resources
## Resources
- Bilibili pthread / synchronization learning videos:
  https://www.bilibili.com/video/BV1Ev411G7i3/
  https://www.bilibili.com/video/BV1fz4y1D7tU/
  
- POSIX Threads / pthreads:
  https://en.wikipedia.org/wiki/Pthreads

- Linux pthreads manual:
  https://man7.org/linux/man-pages/man7/pthreads.7.html

- pthread condition variables:
  https://man7.org/linux/man-pages/man3/pthread_cond_wait.3.html

- GCC warning options:
  https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html

- Dining Philosophers problem:
  https://en.wikipedia.org/wiki/Dining_philosophers_problem

- Priority queue:
  https://en.wikipedia.org/wiki/Priority_queue

- Binary heap:
  https://en.wikipedia.org/wiki/Binary_heap

- Earliest Deadline First scheduling:
  https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling

- FIFO:
  https://en.wikipedia.org/wiki/FIFO_(computing_and_electronics)

- gettimeofday:
  https://man7.org/linux/man-pages/man2/gettimeofday.2.html

---

## AI Usage
AI tools are used for:

- understanding pthread mutexes and condition variables
- reasoning about deadlock and starvation
- designing and debugging the heap priority queue
- explaining FIFO and EDF scheduling
- clarifying timing, cooldown, and burnout logic
- preparing README and defense explanations

All AI-assisted explanations and code ideas were manually reviewed, tested, and rewritten where necessary. The final implementation was validated with compilation tests, runtime tests, and Valgrind.
