/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

struct SDL_Window;
struct SDL_Renderer;
namespace png2dds {

/** RAII wrapper around SDL2/Dear ImGui initialization, main loop and cleanup code. */
class window final {
public:
	window();

	window(const window&) = delete;
	window(window&&) = delete;
	window& operator=(const window&) = delete;
	window& operator=(window&&) = delete;

	~window();

	/**
	 * Check and display window initialization errors.
	 * @return Empty string if there were no errors, a C-style string describing the error otherwise.
	 */
	[[nodiscard]] const std::string& error() const noexcept;

	void main_loop() const;

private:
	std::string _error;
	SDL_Window* _window;
	SDL_Renderer* _renderer;
};

} // namespace png2dds
