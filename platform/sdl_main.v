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

module main

import sdl
import gamesrc

// This is program entry point on SDL platform
struct SdlContext {
pub mut:
	//	VIDEO
	window   &sdl.Window   = sdl.null
	renderer &sdl.Renderer = sdl.null
	screen   &sdl.Texture  = sdl.null
	// AUDIO
	audio_device    &sdl.AudioSpec = sdl.null
	audio_device_id sdl.AudioDeviceID
}

struct SdlPlatformServices {
}

fn (s SdlPlatformServices) debug_read_entire_file(file_name string) (voidptr, u32) {
	rw_ops := sdl.rw_from_file(file_name.str, 'r'.str)
	if rw_ops == sdl.null {
		// TODO(Nathan) logging
		unsafe {
			return nil, 0
		}
	}
	defer {
		rw_ops.close()
	}
	file_size := usize(0)
	file_data := sdl.load_file_rw(rw_ops, &file_size, 0)
	if file_data == sdl.null {
		// TODO(Nathan) logging
		unsafe {
			return nil, 0
		}
	}
	return file_data, u32(file_size)
}

fn (s SdlPlatformServices) debug_free_file_memory(file_data voidptr) {
	sdl.free(file_data)
}

fn (s SdlPlatformServices) debug_write_entire_file(file_name string, file_size u32, data voidptr) bool {
	rw_ops := sdl.rw_from_file(file_name.str, 'w'.str)
	if rw_ops == sdl.null {
		// TODO(Nathan) logging
		return false
	}
	defer {
		rw_ops.close()
	}
	written := rw_ops.write(data, sizeof(u8), file_size)
	// TODO(Nathan) logging if errors
	return written == file_size
}

fn main() {
	mut sdlc := SdlContext{}
	mut result_code := sdl.init(sdl.init_video | sdl.init_audio)
	if result_code < 0 {
		err := sdl.get_error()
		println('Error initializing SDL: ${err}')
		return
	}

	// TODO(Nathan): Handle window resolution (size) in a config file
	sdlc.window = sdl.create_window('Handmade'.str, sdl.windowpos_undefined, sdl.windowpos_undefined,
		1024, 768, u32(sdl.WindowFlags.shown))
	sdlc.renderer = sdl.create_renderer(sdlc.window, 0, u32(sdl.RendererFlags.accelerated))
	sdlc.screen = sdl.create_texture(sdlc.renderer, .argb8888, .streaming, 1024, 768)

	request_spec := sdl.AudioSpec{
		freq: 44100
		format: sdl.audio_s16lsb
		channels: 2
		samples: 0
		callback: sdl.null
		userdata: sdl.null
	}
	sdlc.audio_device = &request_spec
	sdlc.audio_device_id = sdl.open_audio_device(sdl.null, 0, sdlc.audio_device, sdl.null,
		0)
	if sdlc.audio_device_id == 0 {
		err := sdl.get_error()
		println('Error opening audio device: ${err}')
	} else if sdlc.audio_device.format != sdl.audio_s16lsb {
		println('Wrong audio format retrieved! Disabling audio.')
		sdl.close_audio_device(sdlc.audio_device_id)
		sdlc.audio_device_id = 0
	} else {
		println('Opened audio device at ${sdlc.audio_device.freq}hz, ${sdlc.audio_device.channels} channels')
	}

	defer {
		if sdlc.audio_device_id != 0 {
			sdl.close_audio_device(sdlc.audio_device_id)
		}
		sdl.destroy_window(sdlc.window)
		sdl.destroy_renderer(sdlc.renderer)
		sdl.destroy_texture(sdlc.screen)
		sdl.quit()
	}

	mut game := gamesrc.Game{
		memory: &gamesrc.GameMemory{
			permanent_store: []u8{len: 64 * 1024 * 1024}
			transient_store: []u8{len: 4 * 1024 * 1024 * 1024}
			is_initialized: false
		}
		offscreen_buffer: &gamesrc.OffscreenBuffer{
			width: 1024
			height: 768
			pitch: 4 * 1024
			pixels: []u8{len: 4 * 1024 * 768}
		}
		sound_buffer: &gamesrc.SoundBuffer{
			memory: []u8{len: sdlc.audio_device.freq * sdlc.audio_device.channels * sdl.audio_bitsize(sdlc.audio_device.format) / 8}
			samples_per_sec: sdlc.audio_device.freq
			t: 0
			bytes_per_sample: sdlc.audio_device.channels * sdl.audio_bitsize(sdlc.audio_device.format) / 8
			samples_needed: 0
			enabled: sdlc.audio_device_id != 0
		}
		key_input: &gamesrc.KeyboardInput{
			speed_up: gamesrc.GameKeyState{
				half_transition_count: 0
				ended_down: false
			}
			speed_down: gamesrc.GameKeyState{
				half_transition_count: 0
				ended_down: false
			}
			strafe_left: gamesrc.GameKeyState{
				half_transition_count: 0
				ended_down: false
			}
			strafe_right: gamesrc.GameKeyState{
				half_transition_count: 0
				ended_down: false
			}
		}
	}

	platform_services := SdlPlatformServices{}

	mut should_close := false
	game_tick_seconds := f32(.03333333)
	mut game_clock := gamesrc.Time64{
		whole_seconds: 0
		fraction: 0
	}
	mut accumulator := f32(0.0)

	mut last_counter := sdl.get_performance_counter()
	mut is_audio_paused := true
	game.update(platform_services, game_clock)
	for {
		evt := sdl.Event{}
		for 0 < sdl.poll_event(&evt) {
			match evt.@type {
				.quit {
					should_close = true
				}
				.keydown {
					key := unsafe { sdl.KeyCode(evt.key.keysym.sym) }
					if key == sdl.KeyCode.escape {
						should_close = true
						break
					}
					if key == sdl.KeyCode.w {
						game.key_input.speed_up.half_transition_count++
						game.key_input.speed_up.ended_down = true
					}
					if key == sdl.KeyCode.a {
						game.key_input.strafe_left.half_transition_count++
						game.key_input.strafe_left.ended_down = true
					}
					if key == sdl.KeyCode.s {
						game.key_input.speed_down.half_transition_count++
						game.key_input.speed_down.ended_down = true
					}
					if key == sdl.KeyCode.d {
						game.key_input.strafe_right.half_transition_count++
						game.key_input.strafe_right.ended_down = true
					}
					// TODO(Nathan) handle keys
				}
				.keyup {
					key := unsafe { sdl.KeyCode(evt.key.keysym.sym) }
					if key == sdl.KeyCode.w {
						game.key_input.speed_up.half_transition_count++
						game.key_input.speed_up.ended_down = false
					}
					if key == sdl.KeyCode.a {
						game.key_input.strafe_left.half_transition_count++
						game.key_input.strafe_left.ended_down = false
					}
					if key == sdl.KeyCode.s {
						game.key_input.speed_down.half_transition_count++
						game.key_input.speed_down.ended_down = false
					}
					if key == sdl.KeyCode.d {
						game.key_input.strafe_right.half_transition_count++
						game.key_input.strafe_right.ended_down = false
					}
				}
				.windowevent {
					match evt.window.event {
						u8(sdl.WindowEventID.size_changed) {
							println('Window size changed (${evt.window.data1}, ${evt.window.data2})')
						}
						else {}
					}
				}
				else {}
			}
		}
		// TODO(Nathan) get frame time, add to accumulator, while accumulator has more time than delta-t, run game ticks, advance clock
		current_counter := sdl.get_performance_counter()
		frame_time := f32(current_counter - last_counter) / f32(sdl.get_performance_frequency())
		last_counter = current_counter
		accumulator += frame_time

		for accumulator > game_tick_seconds {
			game.update(platform_services, game_clock)
			game_clock = game_clock.add(game_tick_seconds)
			accumulator -= game_tick_seconds
		}
		// TODO(Nathan) If window is not in focus, cap frames to save system resources

		// TODO(Nathan) For now, there may be some ms latency in audio
		target_queue_bytes := int(
			f32(game.sound_buffer.samples_per_sec * game.sound_buffer.bytes_per_sample * game_tick_seconds * 2) +
			0.5)
		bytes_to_write := target_queue_bytes - sdl.get_queued_audio_size(sdlc.audio_device_id)
		game.sound_buffer.samples_needed = int(bytes_to_write / game.sound_buffer.bytes_per_sample)
		game.fill_buffers(platform_services)
		if game.sound_buffer.enabled {
			sdl.queue_audio(sdlc.audio_device_id, game.sound_buffer.memory.data, u32(bytes_to_write))
		}

		sdlc.draw_begin()
		sdlc.draw_frame(game.offscreen_buffer)
		sdlc.draw_end()

		if is_audio_paused {
			is_audio_paused = false
			sdl.pause_audio_device(sdlc.audio_device_id, int(is_audio_paused))
		}

		if should_close {
			break
		}
	}
}

fn (mut sdlc SdlContext) draw_begin() {
	sdl.render_clear(sdlc.renderer)
}

fn (mut sdlc SdlContext) draw_frame(offscreen_buffer gamesrc.OffscreenBuffer) {
	unsafe {
		screen_pixels := voidptr(nil)
		screen_pitch := 0
		screen_width := 0
		screen_height := 0
		sdl.query_texture(sdlc.screen, sdl.null, sdl.null, &screen_width, &screen_height)
		sdl.lock_texture(sdlc.screen, sdl.null, &screen_pixels, &screen_pitch)
		vmemcpy(screen_pixels, offscreen_buffer.pixels.data, screen_height * screen_pitch)
		sdl.unlock_texture(sdlc.screen)
	}
	sdl.render_copy(sdlc.renderer, sdlc.screen, sdl.null, sdl.null)
}

fn (mut sdlc SdlContext) draw_end() {
	sdl.render_present(sdlc.renderer)
}
