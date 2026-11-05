// Dear ImGui: standalone example application for SDL3 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp
#define NOMINMAX

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <cstddef>
#include <type_traits>
#include <SDL3/SDL.h>
#include <windows.h>

#include "fuzzy_session.h"
#include "poisson_generator.h"
#include "fuzzy_io.h"
#include "ipc.h"

using namespace ipc;

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

struct Filter {
    bool enabled = false;

    float centroid_min = 0.0f, centroid_max = 100.0f;
    float length_min = 0.0f, length_max = 100.0f;

    float alpha = 0.5f;
    float alpha_width_min = 0.0f, alpha_width_max = 100.0f;

    bool use_centroid = false;
    bool use_length = false;
    bool use_alpha_width = false;
};


static float alpha_width(const FuzzyValue& v, float alpha) {
    float xmin = std::numeric_limits<float>::max();
    float xmax = std::numeric_limits<float>::lowest();
    bool found = false;
    for (size_t i = 0; i < v.size(); ++i) {
        if (v.mus()[i] >= alpha) {
            xmin = std::min(xmin, static_cast<float>(v.xs()[i]));
            xmax = std::max(xmax, static_cast<float>(v.xs()[i]));
            found = true;
        }
    }
    return found ? (xmax - xmin) : 0.0f;
}

static float interval_length(const FuzzyValue& v) {
    if (v.size() < 2) return 0.0f;
    return static_cast<float>(v.xs().back() - v.xs().front());
}

static bool matches_filter(const FuzzyValue& v, const Filter& f) {
    if (!f.enabled) return true;
    if (f.use_centroid) {
        float c = static_cast<float>(v.centroid());
        if (c < f.centroid_min || c > f.centroid_max) return false;
    }
    if (f.use_length) {
        float len = interval_length(v);
        if (len < f.length_min || len > f.length_max) return false;
    }
    if (f.use_alpha_width) {
        float w = alpha_width(v, f.alpha);
        if (w < f.alpha_width_min || w > f.alpha_width_max) return false;
    }
    return true;
}

// Main code
int main(int, char**)
{
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + generally GLSL 150
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + generally GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", (int)(1280 * main_scale), (int)(800 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    //style.FontSizeBase = 16.0f;           // Default to 13.0f when unspecified, which allows crisp legacy font for non-4K screens.
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;

    // Setup scaling
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    //Shared session
    const std::size_t SHARED_BUFFER_SIZE = 1024 * 1024;
    ipc::SharedSession shared;
    try {
        shared = ipc::SharedSession::create("FuzzySession", SHARED_BUFFER_SIZE);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to create shared session: " << e.what() << "\n";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    //FuzzySession
    fuzzy_session::Session session;
    std::vector<std::string> plot_labels;

    auto read_session_from_shared = [&]() {
        shared.lock();
        try {
            session.deserialize_from_buffer(shared.data(), shared.size());
            plot_labels.resize(session.size());
            for (size_t i = 0; i < session.size(); ++i) {
                if (plot_labels[i].empty()) {  
                    plot_labels[i] = "Value " + std::to_string(i);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "deserialize error: " << e.what() << "\n";
        }
        shared.unlock();
        };

    auto write_session_to_shared = [&]() {
        shared.lock();
        try {
            session.serialize_to_buffer(shared.data(), shared.size());
        }
        catch (const std::exception& e) {
            std::cerr << "serialize error: " << e.what() << "\n";
        }
        shared.unlock();
    };

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //Filter
    Filter filter;


    //Custom ColorMap

    const int COLOR_COUNT = 400;
    std::vector<ImVec4> custom_colors;
    custom_colors.reserve(COLOR_COUNT);

    for (int i = 0; i < COLOR_COUNT; ++i) {
        float hue = std::fmod(i * 0.618034f, 1.0f);
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(hue, 0.7f, 0.9f, r, g, b);
        custom_colors.push_back(ImVec4(r, g, b, 1.0f));
    }
    ImPlotColormap custom_map = ImPlot::AddColormap("CustomFuzzy", custom_colors.data(), COLOR_COUNT);

    // Main loop
    HANDLE shutdown_event = CreateEventA(nullptr, TRUE, FALSE, "Local\\FuzzySession_Shutdown"); 
    bool done = false;
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!done)
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        read_session_from_shared();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        //ImGui
        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
        ImVec2 viewportCenter = ImGui::GetMainViewport()->GetCenter();

        // Graphics Area
        ImVec2 graphicsSize(viewportSize.x * 0.6f, viewportSize.y * 0.5f);
        ImGui::SetNextWindowSize(graphicsSize, ImGuiCond_Always);
        ImGui::SetNextWindowPos(viewportCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGuiWindowFlags commonFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

        if (ImGui::Begin("Graphics Area", nullptr, commonFlags)) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 plotSize(avail.x * 0.9f, avail.y * 0.9f);
            ImGui::SetCursorPos(ImVec2(avail.x * 0.05f, avail.y * 0.05f));           
            if (ImPlot::BeginPlot("Fuzzy Values", plotSize, ImPlotFlags_NoLegend)) {
                ImPlot::SetupAxesLimits(0.0, 20.0, 0.0, 1.0, ImPlotCond_Once);
                ImPlot::SetupAxes("x", "mu(x)",
                    ImPlotAxisFlags_RangeFit,
                    ImPlotAxisFlags_RangeFit);

                for (size_t i = 0; i < session.size(); ++i) {
                    const auto& v = session.get(i);
                    bool matches = matches_filter(v, filter);

                    ImPlotSpec spec;
                    if (matches) {
                        spec.LineColor = ImPlot::GetColormapColor(static_cast<int>(i), custom_map);
                        spec.LineWeight = 2.0f;
                    }
                    else {
                        spec.LineColor = ImVec4(0.3f, 0.3f, 0.3f, 0.3f);  //grey
                        spec.LineWeight = 1.0f;
                    }
                    const char* label = (i < plot_labels.size()) ? plot_labels[i].c_str() : "Value";
                    ImPlot::PlotLine(label, v.xs().data(), v.mus().data(),
                        static_cast<int>(v.size()), spec);
                }
                ImPlot::EndPlot();
            }
        }
        ImGui::End();

        ImVec2 graphicsTopLeft(viewportCenter.x - graphicsSize.x * 0.5f, viewportCenter.y - graphicsSize.y * 0.5f);

        // Buttons
        float panelWidth = viewportSize.x * 0.17f;

        ImVec2 buttonsPos(graphicsTopLeft.x - panelWidth - 10.0f, graphicsTopLeft.y);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, graphicsSize.y), ImGuiCond_Always);
        ImGui::SetNextWindowPos(buttonsPos, ImGuiCond_Always);

        if (ImGui::Begin("Controls", nullptr, commonFlags)) {
            float btnW = ImGui::GetContentRegionAvail().x * 0.8f;
            float btnH = ImGui::GetFrameHeight();
            int btnCount = 5;

            float centerX = (ImGui::GetContentRegionAvail().x - btnW) / 2.0f;

            float availY = ImGui::GetContentRegionAvail().y;
            float totalBtnH = btnH * btnCount;
            float gap = (availY - totalBtnH) / (btnCount + 1);
            float maxGap = 20.0f;
            if (gap > maxGap) gap = maxGap;

            float groupHeight = totalBtnH + gap * (btnCount - 1);
            float startY = (availY - groupHeight) / 2.0f;
            if (startY < 0) startY = 0; 

            ImGui::Dummy(ImVec2(0, startY));

            ImGui::SetCursorPosX(centerX);
            if (ImGui::Button("Generate", ImVec2(btnW, 0))) {
                try {
                    auto set = fuzzy_generator::generate_poisson_set(50, 10.0, 20.0, 1000);
                    size_t old_size = session.size();
                    session.add(std::move(set));

                    for (size_t i = old_size; i < session.size(); ++i) {
                        plot_labels.push_back("Value " + std::to_string(i));
                    }
                    write_session_to_shared();
                }
                catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what();
                }
            }
            ImGui::Dummy(ImVec2(0, gap));

            ImGui::SetCursorPosX(centerX);
            if (ImGui::Button("Command prompt", ImVec2(btnW, 0))) {
                STARTUPINFOA si = {};
                si.cb = sizeof(si);
                PROCESS_INFORMATION pi = {};
                if (!CreateProcessA(
                    nullptr,
                    (LPSTR)"fuzzy_cli.exe",
                    nullptr, nullptr, FALSE,
                    CREATE_NEW_CONSOLE,
                    nullptr, nullptr,
                    &si, &pi))
                {
                    std::cerr << "Failed to launch fuzzy_cli.exe\n";
                }
                else {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
            }
            ImGui::Dummy(ImVec2(0, gap));

            ImGui::SetCursorPosX(centerX);
            if (ImGui::Button("Clear", ImVec2(btnW, 0))) {
                session.clear();
                plot_labels.clear();
                write_session_to_shared();
            }
            ImGui::Dummy(ImVec2(0, gap));

            ImGui::SetCursorPosX(centerX);
            if (ImGui::Button("Minimize", ImVec2(btnW, 0))) {
                SDL_MinimizeWindow(window);
            }
            ImGui::Dummy(ImVec2(0, gap));

            ImGui::SetCursorPosX(centerX);
            if (ImGui::Button("Exit", ImVec2(btnW, 0))) {
                done = true;
            }
        }
        ImGui::End();

        // Logs
        ImVec2 logPos(graphicsTopLeft.x + graphicsSize.x + 10.0f, graphicsTopLeft.y);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, graphicsSize.y), ImGuiCond_Always);
        ImGui::SetNextWindowPos(logPos, ImGuiCond_Always);
        if (ImGui::Begin("Log", nullptr, commonFlags)) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImGui::Text("Info text:\n");
            for (size_t i = 0; i < session.size(); ++i) {
                const auto& v = session.get(i);
                bool matches = matches_filter(v, filter);
                ImVec4 color = ImPlot::GetColormapColor(static_cast<int>(i), custom_map);
                if (!matches) color.w = 0.3f;  //grey

                ImU32 col_u32 = ImGui::ColorConvertFloat4ToU32(color);
                ImVec2 p = ImGui::GetCursorScreenPos();
                float square_size = ImGui::GetTextLineHeight() * 0.8f;
                draw_list->AddRectFilled(p, ImVec2(p.x + square_size, p.y + square_size), col_u32);
                ImGui::Dummy(ImVec2(square_size + 4.0f, square_size));
                ImGui::SameLine();

                if (matches) {
                    ImGui::Text("[%zu] size=%zu centroid=%.3f", i, v.size(), v.centroid());
                }
                else {
                    ImGui::TextDisabled("[%zu] size=%zu centroid=%.3f", i, v.size(), v.centroid());
                }
            }
        }
        ImGui::End();

        //Filter panel
        ImVec2 filterPos(graphicsTopLeft.x, graphicsTopLeft.y + graphicsSize.y + 10.0f);
        ImGui::SetNextWindowSize(ImVec2(graphicsSize.x, viewportSize.y * 0.2f), ImGuiCond_Always);
        ImGui::SetNextWindowPos(filterPos, ImGuiCond_Always);

        if (ImGui::Begin("Filters", nullptr, commonFlags)) {
            ImGui::Checkbox("Enable filter", &filter.enabled);
            ImGui::Separator();

            //Centroid
            ImGui::Checkbox("##use_centroid", &filter.use_centroid);
            ImGui::SameLine();
            ImGui::Text("Centroid:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("##cmin", &filter.centroid_min, 0.1f, 0.0f, 100.0f, "min %.2f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("##cmax", &filter.centroid_max, 0.1f, 0.0f, 100.0f, "max %.2f");

            //Length
            ImGui::Checkbox("##use_length", &filter.use_length);
            ImGui::SameLine();
            ImGui::Text("Length:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("##lmin", &filter.length_min, 0.1f, 0.0f, 100.0f, "min %.2f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("##lmax", &filter.length_max, 0.1f, 0.0f, 100.0f, "max %.2f");

            //Alpha width
            ImGui::Checkbox("##use_alpha", &filter.use_alpha_width);
            ImGui::SameLine();
            ImGui::Text("Alpha width:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("##amin", &filter.alpha_width_min, 0.1f, 0.0f, 100.0f, "min %.2f");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100);
            ImGui::DragFloat("##amax", &filter.alpha_width_max, 0.1f, 0.0f, 100.0f, "max %.2f");

            ImGui::Separator();
            ImGui::SetNextItemWidth(200);
            ImGui::SliderFloat("Alpha level", &filter.alpha, 0.0f, 1.0f, "%.2f");

            //Reset filters
            if (ImGui::Button("Reset filter")) {
                filter = Filter{};
            }
        }
        ImGui::End();

        // Hello Text 
        ImGui::SetNextWindowPos(ImVec2(viewportCenter.x, graphicsTopLeft.y - 30.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        if (ImGui::Begin("HelloText", nullptr, commonFlags | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Fuzzy Complex GUI");
        }
        ImGui::End();
        ImGui::PopStyleColor();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]

    SetEvent(shutdown_event);
    CloseHandle(shutdown_event);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}