/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_TONE_H
#define LIBMCU_TONE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef enum note {
	NOTE_C,
	NOTE_C_SHARP,
	NOTE_D,
	NOTE_D_SHARP,
	NOTE_E,
	NOTE_F,
	NOTE_F_SHARP,
	NOTE_G,
	NOTE_G_SHARP,
	NOTE_A,
	NOTE_A_SHARP,
	NOTE_B,
	NOTE_MAX,
	NOTE_REST,
} tone_note_t;

typedef enum octave {
	OCTAVE_0,
	OCTAVE_1,
	OCTAVE_2,
	OCTAVE_3,
	OCTAVE_4,
	OCTAVE_5,
	OCTAVE_6,
	OCTAVE_7,
	OCTAVE_8,
	OCTAVE_MAX,
} tone_octave_t;

typedef uint16_t tone_pitch_t;

struct tone {
	tone_note_t note;
	tone_octave_t octave;
	uint16_t len_ms;
#if 0
	intensity; /* amplitude */
	color; /* waveform */
#endif
};

tone_pitch_t tone_get(tone_note_t note, tone_octave_t octave);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TONE_H */
