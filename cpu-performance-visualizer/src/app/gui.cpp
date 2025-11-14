// gui.cpp
// SDL2 + OpenGL + ImGui

#include "gui.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <windows.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>

// ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

// Projektinterne Header
#include "measure.h"
#include "../core/measure_runner.h"


// genutzte Lernmaterialien:
// https://github.com/ocornut/imgui  Doku für die gui
// https://www.youtube.com/playlist?list=PLvv0ScY6vfd-p1gSnbQhY7vMe2rng0IL0 // sdl2 tutorial
// windows performance recorder um die CPU Theorie praktisch nachvollziehen zu können -> Analyse der ntoskrnl.exe Funktionen innerhalb der .etl -> Graphenanalyse
// https://www.youtube.com/watch?v=vLnPwxZdW4Y c++ syntax help
// https://www.youtube.com/watch?v=KJgsSFOSQv0 c syntax help
// https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md Testing (Doctest), lightweight googltest alternative google -> too heavy
// Windows Memory und mehr Wissen: Windows Internals 7th Edition: System architecture, processes, threads, memory management, and more, Part 1 von Pavel Yosifovich, Mark E. Russinovich, Alex Ionescu, David A. Solomon
namespace gui {
    // Globale Zustände

    static std::atomic<bool> measuring{false}; //atomic für parallelität

    static std::string console_output;

    // Hintergrund-Thread für Messung

    static void background_measure(size_t N) {
        measuring = true; // signal an GUI -> Messung läuft
        console_output.clear(); // alte Messdaten löschen

        run_measurements(N, [](const std::string &line) {
            //startet die Messlogik in measure_runner.cpp
            console_output += line; // gui console output
        });

        measuring = false;
        // nach Abschluss wird die gui aus dem Messzustand geholt um Buttons dynamisch gestalten zu könnenn
    }


    // Haupt-GUI-Schleife; initialisiert die sdl subsysteme die für die GUI benötigt werden

    int run() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            SDL_Log("SDL_Init Error: %s", SDL_GetError());
            return 1;
        }

        // --- OpenGL-Kontext konfigurieren; best practice; buffering etc---
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        // --- Fenster erzeugen mit Größe und nicht Vollbild ---
        SDL_Window *window = SDL_CreateWindow(
            "CPU Performance Visualizer",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            1350, 780,
            SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS
        );

        SDL_GLContext gl_context = SDL_GL_CreateContext(window);
        SDL_GL_SetSwapInterval(1); // VSync aktivieren

        // --- ImGui initialisieren ---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
        ImGui_ImplOpenGL3_Init("#version 130");
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // --- Fonts ---
        io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 18.0f);
        io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/consola.ttf", 16.0f);

        // --- Style, standard ColorsDark mit eigenen Akzenten wie Rot zB, wird später in Logik nochmal verwendet für dynamische buttonfarben---
        ImGui::StyleColorsDark();
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Button] = ImVec4(0.80f, 0.25f, 0.25f, 0.40f); // normal rot
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.30f, 0.30f, 1.00f); // hover rot
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.20f, 0.20f, 1.00f); // gedrückt rot
        style.WindowRounding = 5.0f;
        style.FrameRounding = 3.0f;
        style.FramePadding = ImVec2(8, 5);
        style.ItemSpacing = ImVec2(10, 6);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.11f, 1.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.478f, 0.478f, 0.478f, 1.0f); // grau
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.655f, 0.655f, 0.655f, 1.0f); // grau
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.655f, 0.655f, 0.655f, 1.0f); //grau
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);


        // --- Fensterhandle für Dragging ohne laggs ---
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(window, &wmInfo);
        HWND hwnd = wmInfo.info.win.window;

        bool running = true;


        // Hauptloop

        while (running) {
            // so lange gui läuft werden SDL events verarbeitet
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                if (e.type == SDL_QUIT) running = false;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            }

            ImGui_ImplOpenGL3_NewFrame(); // backend Aufrufe für die frames (render/shader/aktualisiert EIngaben)
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            int window_width, window_height;
            SDL_GetWindowSize(window, &window_width, &window_height);

            // Title-Bar

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2((float) window_width, 30));
            ImGui::Begin("TitleBar", nullptr,
                         ImGuiWindowFlags_NoDecoration |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::Text("CPU Performance Visualizer");

            float button_size = 22.0f;
            float spacing = 5.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - (button_size + spacing) * 2);

            if (ImGui::Button("_", ImVec2(button_size, button_size))) // minimieren button oben rechts
                SDL_MinimizeWindow(window);

            ImGui::SameLine(); // heißt in der gui dass das nächste Element in der selben Zeile angeordnet werden soll
            if (ImGui::Button("X", ImVec2(button_size, button_size))) {
                // schließen button oben rechts
                SDL_Event quit;
                quit.type = SDL_QUIT;
                SDL_PushEvent(&quit);
            }

            // Draggen on click nach dem oben extrahiert wurde
            if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ReleaseCapture();
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }

            ImGui::End();


            // Hauptbereich

            ImGui::SetNextWindowPos(ImVec2(0, 30));
            ImGui::SetNextWindowSize(ImVec2((float) window_width, (float) window_height - 30));
            ImGui::Begin("CPU Cache Visualizer", nullptr,
                         ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoMove);

            // --- Matrixgröße Slider ---
            static int N_exp = 7;

            ImGui::SliderInt("Matrixgröße (2^x)", &N_exp, 8, 13, "2^%d");
            // diese Switch bestimmt die Farben je nach ausgewählter Größer der Matrix in Grün-Gelb-Rot
            switch (N_exp) {
                case 8:
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
                    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
                    break;
                case 9:
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
                    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
                    break;
                case 10:
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
                    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
                    break;
                case 11:
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.9f, 0.3f, 1.0f);
                    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.9f, 0.3f, 1.0f);
                    break;
                case 12:
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.20f, 0.20f, 1.00f);
                    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.20f, 0.20f, 1.00f);
                    break;
                case 13:
                    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.70f, 0.20f, 0.20f, 1.00f);
                    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.70f, 0.20f, 0.20f, 1.00f);
                    break;
            }
            size_t N = 1ULL << N_exp; // Berechnung der Matrix
            double megabytes = (double) (N * N * sizeof(int)) / (1024.0 * 1024.0); // Berechnung speicherverbrauch in MB
            std::string speicherform[4] = {"L1 Cache", "L2 Cache", "L3 Cache", "RAM-Speicher"};
            // Liste auf die gleich zugegriffen wird
            int level = 0;
            if (megabytes < 0.384) level = 0; // passt in L1
            else if (megabytes < 6.0) level = 1; // passt in L2
            else if (megabytes < 32.0) level = 2; // passt in L3
            else level = 3; // Ram Beteiligung
            ImGui::Text("Aktuelle Matrix Größe: %zu x %zu (~%.2f MB) Speicher: %s", N, N, megabytes, //Ausgabe gui
                        speicherform[level].c_str()
            );

            // --- Startbutton ---
            if (!measuring) {
                if (ImGui::Button("Messung starten", ImVec2(180, 40)))
                    std::thread([N]() { background_measure(N); }).detach();
            } else {
                ImGui::Text("Messung läuft...");
            }
            ImGui::SameLine();

            if (ImGui::Button("Hinweis", ImVec2(100, 40))) {
                ImGui::OpenPopup("Hinweis1");
            }
            // --- PopUp ---
            if (ImGui::BeginPopupModal("Hinweis1", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Warum Spaltenzugriffe mehr CPU-Zyklen benötigen\n");
                ImGui::Separator();
                ImGui::BeginChild("Hinweis2", ImVec2(400, 200), true, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextWrapped(
                    "Hinweis:\n"
                    "Dieses Messprogramm wurde auf einem AMD Ryzen 5 7600X 6-Core Processor "
                    "entwickelt und optimiert. Deine L1/L2/L3 Caches könnten mehr oder weniger Speichergröße haben. Die CPU Taktzyklen sind allerdings immer deine aktuell gelaufenen."
                    ".\n\n");

                ImGui::EndChild();

                ImGui::Separator();
                if (ImGui::Button("Schließen", ImVec2(120, 0)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            // --- Button und Popup für ausführliche Erklärung ---
            if (ImGui::Button("Warum gibt es einen zeitlichen Unterschied?", ImVec2(300, 40))) {
                ImGui::OpenPopup("Technische Erklärung");
            }

            if (ImGui::BeginPopupModal("Technische Erklärung", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Warum Spaltenzugriffe mehr CPU-Zyklen benötigen\n");
                ImGui::Separator();
                ImGui::BeginChild("explanation_scroll", ImVec2(800, 500), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextWrapped(
                    "Wie Arrays im Speicher liegen: Das Row-Major-Prinzip\n\n"

                    "In C und C++ werden mehrdimensionale Arrays (z. B. int A[N][M]) im linearen "
                    "Arbeitsspeicher (RAM) zeilenweise gespeichert. Dieses Prinzip nennt man Row-Major.\n\n"

                    "Das bedeutet:\n"
                    "Alle Elemente einer Zeile liegen direkt nebeneinander im Speicher. "
                    "Die nächste Zeile folgt erst danach.\n\n"

                    "Wenn du also zeilenweise iterierst (A[0][0], A[0][1], A[0][2], ... A[1][0], ...), "
                    "bewegst du dich linear durch den Speicher. Das nutzt den Cache optimal.\n"
                    "Beim spaltenweisen Zugriff (A[0][0], A[1][0], A[2][0], ...) springst du dagegen weite Adressabstände. "
                    "Das verursacht viele Cache-Misses.\n\n"

                    "Die Kühlschrank-Analogie (Cacheline)\n\n"

                    "Diese Analogie zeigt, warum zeilenweises Laden so viel effizienter ist (C++/C):\n\n"

                    "1. Das Setup\n"
                    "- Annahme: Das Beispiel behandelt eine 6x6 Matrix"
                    "- Der Supermarkt (RAM): Jede Fahrt (Zugriff) kostet viel Zeit.\n"
                    "- Der Kühlschrank (CPU-Cache): Der kleinere aber lokal, schnellerer Zugriffsort.\n"
                    "- Der Sechserpack Joghurt (Cacheline): Eine feste Einheit von 64 Byte, "
                    "die die CPU immer komplett vom RAM in den Cache lädt.\n\n"

                    "2. Zeilenweiser Zugriff (effizient)\n"
                    "Du willst mehrere Becher aus demselben Sechserpack essen, also Daten, die nebeneinander liegen.\n"
                    "- Erster Zugriff: Du fährst in den Supermarkt (RAM), kaufst (lädst) ein 6er Pack Joghurt (Cacheline) und legst es in den Kühlschrank (CPU-Cache).\n"
                    " Die CPU lädt automatisch das ganze Sechserpack (A[0][0] bis A[0][6]) in den Cache.\n"
                    "- Weitere Zugriffe: Wenn du wieder einen Joghurt essen (Daten laden) möchtest sind schon im Kühlschrank (CPU-Cache)."
                    "Du kannst sie direkt verwenden.\n"
                    "- Keine erneuten RAM-Zugriffe (Supermarkt Fahrten), hohe Geschwindigkeit.\n\n"

                    "3. Spaltenweiser Zugriff (ineffizient)\n"
                    "Jetzt willst du nur den ersten Becher jedes Sechserpacks essen, also A[0][0], A[1][0], A[2][0] ...\n"
                    "- Erster Zugriff: Du fährst erneut in den Supermarkt (RAM), kaufst (lädst) ein 6er Pack Joghurt (Cacheline)\n"
                    " und legst es in den Kühlschrank (CPU-Cache) und isst den ersten Joghurt (A[0][0]).\n"
                    "- Zweiter Zugriff: .\n"
                    "- Nun möchtest du wieder den ersten Joghurt eines Sechserpacks essen, dieses Sechserpack A[1][0] liegt nicht in der CPU (Kühlschrank) \n"
                    " und du musst erneut zum Supermarkt (RAM) fahren, um den nächsten Joghurt essen zu können.\n"
                    "- Fast jeder Zugriff verursacht eine neue, langsame Fahrt zum RAM – viele Cache-Misses.\n\n"

                    "4. Fazit\n"
                    "Beim zeilenweisen Zugriff nutzt du jede geladene Cacheline vollständig, "
                    "wie beim Verwenden aller Becher eines Sechserpacks.\n"
                    "Beim spaltenweisen Zugriff nutzt du immer nur einen Becher und wirfst den Rest weg, "
                    "die CPU lädt ständig neu.\n\n"

                    "Das ist der Grund, warum Zeilen im Speicherzugriff deutlich schneller sind als Spalten und wieso es wichtig ist wie Datenstrukturen im Sepicher angeordnet und durchlaufen werden. \n"
                    "Ein falsches Zugriffsmuster kann selbst bei identischer Logik den Unterschied zwischen Millisekunden und Sekundenlanger Ausführung bedeuten."
                );

                ImGui::EndChild();

                ImGui::Separator();
                if (ImGui::Button("Schließen", ImVec2(120, 0)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            // --- Button und Popup für ausführliche Erklärung ---
            if (ImGui::Button("Wie können wir beweisen, dass Spaltenzugriffe länger dauern?", ImVec2(400, 40))) {
                ImGui::OpenPopup("Erklärung");
            }

            if (ImGui::BeginPopupModal("Erklärung", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextWrapped("Warum Spaltenzugriffe mehr CPU-Zyklen benötigen\n");
                ImGui::Separator();
                ImGui::BeginChild("explanation_scroll", ImVec2(800, 500), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextWrapped(
                    "Wie können wir beweisen, dass Spaltenzugriffe langsamer sind?\n\n"

                    "1) Messung auf CPU-Ebene (Taktzyklen)\n"
                    "Das Programm misst Zeilen- und Spaltenzugriffe direkt mit der RDTSCP-"
                    "Instruktion. Zeilenweise Zugriffe laufen linear im Speicher und nutzen "
                    "Cachelines effizient aus. Spaltenzugriffe springen hingegen in große "
                    "Abstände (i*N), verursachen viele Cache-Misses und benötigen dadurch "
                    "mehr CPU-Zyklen. Sobald die Matrix nicht mehr vollständig in den Cache "
                    "passt, steigt die Zugriffszeit für Spalten stark an.\n\n"

                    "2) Analyse auf Betriebssystem-Ebene (Windows Kernel)\n"
                    "Mit WPR (Windows Performance Recorder) und WPA (Windows Performance "
                    "Analyzer) kann sichtbar gemacht werden, welche Kernel-Funktionen "
                    "bei beiden Zugriffsmustern ausgeführt werden. Spaltenzugriffe "
                    "verursachen signifikant mehr Ereignisse im Windows-Kernel, unter "
                    "anderem in ntoskrnl.exe.\n\n"

                    "Typische Funktionen, die bei Spaltenzugriff häufiger auftreten:\n"
                    "- RtlpLookupFunctionEntryForStackWalks\n"
                    "- RtlpUnwindPrologue\n"
                    "- MiPageFault / MiResolveDemandZeroFault\n"
                    "- MiZeroPhysicalPage\n"
                    "- MiConvertLargeActivePageToChain\n"
                    "- KeZeroPages\n"
                    "- SwapContext / KiSwapThread\n"
                    "- ExAllocateHeapPool\n\n"

                    "Diese Funktionen stehen unter anderem für Page-Fault-Behandlung, "
                    "Speicherverwaltung, TLB-Aktualisierungen und Scheduling. Zeilenzugriffe "
                    "lösen diese Ereignisse kaum aus, Spaltenzugriffe dagegen in großer Zahl.\n\n"

                    "Fazit:\n"
                    "Die längere Ausführungszeit von Spaltenzugriffen lässt sich sowohl auf "
                    "CPU-Ebene (mehr Taktzyklen) als auch auf Kernel-Ebene (mehr Page Faults "
                    "und Speicherverwaltungsfunktionen) eindeutig nachweisen."
                );

                ImGui::EndChild();

                ImGui::Separator(); // serperator linie
                if (ImGui::Button("Schließen", ImVec2(120, 0)))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }


            // --- Ausgabe ---
            ImGui::SeparatorText("Ausgabe");

            float total_w = ImGui::GetContentRegionAvail().x;
            float total_h = ImGui::GetContentRegionAvail().y;


            // Spalte LINKS (zweigeteilt)
            ImGui::BeginGroup();
            {
                // --- Konsole oben ---
                ImGui::BeginChild("console", ImVec2(total_w, total_h * 0.55f), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextUnformatted(console_output.c_str());
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
                ImGui::EndChild();

                // --- Anleitung unten ---
                ImGui::BeginChild("instructions", ImVec2(total_w, total_h * 0.40f), true);
                ImGui::SeparatorText("Anleitung");
                ImGui::BulletText("1. Wähle mit dem Regler oben eine Matrixgröße (2^x).");
                ImGui::BulletText("2. Starte die Messung, um die CPU-Zugriffszeiten zu vergleichen.");
                ImGui::BulletText("3. Beobachte, wie Zeilen- und Spaltenzugriffe unterschiedlich effizient sind.");
                ImGui::Text("   Zeilenweise Zugriffe sind cachefreundlich (linear im Speicher).");
                ImGui::Text("   Spaltenweise Zugriffe verursachen mehr Cache-Misses (nicht-linear).");
                ImGui::EndChild();
            }
            ImGui::EndGroup();


            ImGui::End();

            // --- Rendering ---
            ImGui::Render();
            int dw, dh;
            SDL_GL_GetDrawableSize(window, &dw, &dh);
            glViewport(0, 0, dw, dh);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);
        }

        // --- Cleanup Phase nach Beenden der Hauptschleife = Programm schließen ---
        ImGui_ImplOpenGL3_Shutdown(); // Backend shutdown

        ImGui_ImplSDL2_Shutdown(); // Backend shutdown


        ImGui::DestroyContext(); // haupt imgui kontext schließen

        SDL_GL_DeleteContext(gl_context); //opengl kontext schließen
        SDL_DestroyWindow(window); // sdl schließen; Systemressourcen freigeben

        SDL_Quit(); // sdl herunterfahren

        return
                0; // return 0 wenn Gui erfolgreich gelebt hat
    }
} // namespace gui
