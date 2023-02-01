/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/window.hpp"

#include "png2dds/project.hpp"

#include <SDL.h>

#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_sdlrenderer.h>
#include <fmt/format.h>
#include <imgui.h>

#include <array>

namespace {
SDL_Window* create_window() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) { return nullptr; }

	// Initialization of SDL2.
	const auto window_title = fmt::format("{:s} {:s}", png2dds::project::name(), png2dds::project::version());
	constexpr auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	return SDL_CreateWindow(window_title.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
}

SDL_Renderer* create_renderer(SDL_Window* window) {
	return (window != nullptr) ? SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED) :
															 nullptr;
}

} // anonymous namespace

namespace png2dds {
window::window()
	: _error{}
	, _window{create_window()}
	, _renderer{create_renderer(_window)} {
	if (_window == nullptr) {
		_error = fmt::format("SDL window initialization error: {:s}", SDL_GetError());
		return;
	}
	if (_renderer == nullptr) {
		_error = fmt::format("SDL rendering context creation error: {:s}", SDL_GetError());
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& imgui_io = ImGui::GetIO();
	imgui_io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;
	// ToDo Define a style.
	ImGui::StyleColorsDark();
	(void)ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
	(void)ImGui_ImplSDLRenderer_Init(_renderer);
	imgui_io.Fonts->AddFontFromFileTTF("Manrope-Regular.ttf", 20.0F);
}

window::~window() {
	if (_renderer != nullptr) {
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_DestroyRenderer(_renderer);
	}

	if (_window != nullptr) {
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}
}

const std::string& window::error() const noexcept { return _error; }

void window::main_loop() const {
	constexpr std::array<Uint8, 4U> clear_color{static_cast<unsigned char>(0.45F * 255),
		static_cast<unsigned char>(0.55F * 255), static_cast<unsigned char>(0.60F * 255),
		static_cast<unsigned char>(1.00F * 255)};

	bool done = false;
	const auto window_id = SDL_GetWindowID(_window);
	const ImGuiIO& imgui_io = ImGui::GetIO();

	while (!done) {
		SDL_Event event;
		while (SDL_PollEvent(&event) > 0) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			done = event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
																				 event.window.windowID == window_id;
		}

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// ToDo UI
		ImGui::Begin("Hello world!");
		ImGui::Text("Hello world!");
		ImGui::End();

		ImGui::Render();
		SDL_RenderSetScale(_renderer, imgui_io.DisplayFramebufferScale.x, imgui_io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColor(_renderer, clear_color[0U], clear_color[1U], clear_color[2U], clear_color[3U]);
		SDL_RenderClear(_renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(_renderer);
	}
}

} // namespace png2dds
