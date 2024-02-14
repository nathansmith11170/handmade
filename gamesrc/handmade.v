/*
<name TBD>, a handmade game about simulating things in space
Copyright (C) 2024 Nathan Smith

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

module gamesrc

import math
import interfaces { Platform }

// The only public methods in the module go here
// Services provided by the game to the platform
// Also handle any use of platform services (network and file I/O, timing, etc)
// Audio and visual buffer output

pub struct Game {
pub mut:
	memory           GameMemory
	offscreen_buffer OffscreenBuffer
	sound_buffer     SoundBuffer
	key_input        KeyboardInput
}

pub struct GameMemory {
pub mut:
	permanent_store []u8
	transient_store []u8
	is_initialized  bool
}

pub struct OffscreenBuffer {
pub mut:
	pixels []u8
	width  int
	height int
	pitch  int
}

pub struct SoundBuffer {
pub mut:
	memory           []u8
	samples_per_sec  int
	t                u32
	bytes_per_sample int
	samples_needed   int
	enabled          bool
}

pub struct GameKeyState {
pub mut:
	half_transition_count int
	ended_down            bool
}

pub struct KeyboardInput {
pub mut:
	speed_up     GameKeyState
	speed_down   GameKeyState
	strafe_left  GameKeyState
	strafe_right GameKeyState
}

pub fn (mut g Game) game_update_fill_buffers(platform Platform) {
	unsafe {
		game_state := &GameState(g.memory.permanent_store[0..sizeof(GameState)].data)

		if !g.memory.is_initialized {
			game_state.tone_hz = 256
			g.memory.is_initialized = true
		}

		if g.key_input.speed_up.ended_down {
			game_state.green_offset++
		}
		if g.key_input.strafe_left.ended_down {
			game_state.blue_offset++
		}
		if g.key_input.speed_down.ended_down {
			game_state.green_offset--
		}
		if g.key_input.strafe_right.ended_down {
			game_state.blue_offset--
		}

		game_state.render_weird_gradient(mut g.offscreen_buffer)
		if g.sound_buffer.enabled {
			game_state.output_sine(mut g.sound_buffer)
		}
	}
}

struct GameState {
mut:
	blue_offset  int
	green_offset int
	tone_hz      int
}

fn (g GameState) output_sine(mut buf SoundBuffer) {
	unsafe {
		tone_volume := 9000
		wave_period := f64(buf.samples_per_sec) / f64(g.tone_hz)

		sample_out := &i16(buf.memory.data)
		for _ in 0 .. buf.samples_needed {
			sine_value := math.sin(math.tau * f64(buf.t) / wave_period)

			sample_val := i16(sine_value * tone_volume)

			*sample_out = sample_val
			sample_out++
			*sample_out = sample_val
			sample_out++

			buf.t++
			if f64(buf.t) > wave_period {
				buf.t = 1
			}
		}
	}
}

fn (g GameState) render_weird_gradient(mut buf OffscreenBuffer) {
	unsafe {
		row_ptr := &u8(buf.pixels.data)
		for y in 0 .. buf.height {
			pixel_ptr := &u32(row_ptr)
			for x in 0 .. buf.width {
				blue := u8(x + g.blue_offset)
				green := u8(y + g.green_offset)

				*pixel_ptr = (u32(green) << 8) | u32(blue)
				pixel_ptr++
			}
			row_ptr += buf.pitch
		}
	}
}
