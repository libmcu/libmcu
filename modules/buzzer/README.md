# Buzzer Module
The Buzzer module in libmcu provides a flexible interface for controlling buzzer devices in embedded systems. It supports the playback of melodies through passive buzzers using PWM (Pulse Width Modulation) signals.

## Features
- Melody Playback: Play sequences of notes with specified frequencies and durations.
- Non-Blocking Operation: Utilizes timer callbacks to manage note transitions without blocking the main program flow.
- Configurable Callbacks: Allows users to define custom callbacks for events such as the completion of melody playback.

## Dependencies
This module depends on the following components:

- [PWM Interface](../../interfaces/pwm): Ensure that the PWM interface is properly configured and initialized.
- [Timer Interface](../../interfaces/timer): Ensure that the timer interface is properly configured and initialized.

## Getting Started
### Initialization
Before using the buzzer, initialize the module with the appropriate configuration:

```c
#include "libmcu/buzzer.h"
#include "libmcu/pwm.h"

#define PWM_CHANNEL 1
#define PWM_PIN     1

struct pwm_channel *pwm_ch = pwm_create_channel(pwm, PWM_CHANNEL, PWM_PIN);

buzzer_init(pwm_ch, 0, 0);
```

### Playing a Melody
To play a melody, define the sequence of notes and their durations:

```c
#include "libmcu/buzzer.h"

const struct tone my_melody_notes[] = {
	{ .note = NOTE_C,       .octave = OCTAVE_7, .len_ms = 100 },
	{ .note = NOTE_D,       .octave = OCTAVE_7, .len_ms = 100 },
	{ .note = NOTE_E,       .octave = OCTAVE_7, .len_ms = 150 },
};

const struct melody my_melody = {
    .notes = my_melody_notes,
    .num_notes = sizeof(my_melody_notes) / sizeof(my_melody_notes[0]),
};

buzzer_play(&my_melody);
```

### Stopping the Melody
To stop the currently playing melody:

```c
buzzer_stop();
```
