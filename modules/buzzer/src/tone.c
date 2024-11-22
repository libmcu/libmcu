/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/tone.h"

tone_pitch_t tone_get(tone_note_t note, tone_octave_t octave)
{
	static const tone_pitch_t octaves[OCTAVE_MAX][NOTE_MAX] = {
		[OCTAVE_0] = {   16,   17,   18,   19,   21,   22,
			         23,   25,   26,   28,   29,   31 },
		[OCTAVE_1] = {   33,   35,   37,   39,   41,   44,
			         46,   49,   52,   55,   58,   62 },
		[OCTAVE_2] = {   65,   69,   73,   78,   82,   87,
			         93,   98,  104,  110,  117,  124 },
		[OCTAVE_3] = {  131,  139,  147,  156,  165,  175,
			        185,  196,  208,  220,  233,  247 },
		[OCTAVE_4] = {  262,  277,  294,  311,  330,  349,
			        370,  392,  415,  440,  466,  494 },
		[OCTAVE_5] = {  523,  554,  587,  622,  659,  699,
			        740,  784,  831,  880,  932,  988 },
		[OCTAVE_6] = { 1047, 1109, 1175, 1245, 1319, 1397,
			       1480, 1568, 1661, 1760, 1865, 1976 },
		[OCTAVE_7] = { 2093, 2217, 2349, 2489, 2637, 2794,
			       2960, 3136, 3322, 3520, 3729, 3951 },
		[OCTAVE_8] = { 4186, 4435, 4699, 4978, 5274, 5588,
			       5920, 6272, 6645, 7040, 7459, 7902 },
	};

	if (note >= NOTE_MAX || octave >= OCTAVE_MAX) {
		return 0;
	}

	return octaves[octave][note];
}
