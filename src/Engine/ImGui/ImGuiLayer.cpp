#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <Engine/ImGui/ImGuiLayer.hpp>
#include <Engine/Core/Application.hpp>


namespace Engine {

    void ImGuiLayer::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent& e) {
            return OnMouseButtonPressed(e);
        });
        dispatcher.Dispatch<MouseButtonReleasedEvent>([this](MouseButtonReleasedEvent& e) {
            return OnMouseButtonReleased(e);
        });
        dispatcher.Dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) {
            return OnMouseMoved(e);
        });
        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& e) {
            return OnMouseScrolled(e);
        });
    }

    void ImGuiLayer::OnImGuiRender() {
        static int location = 0;
        ImGuiIO& io = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (location >= 0)
        {
            const float PAD = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
            window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
            window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
            window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        else if (location == -2)
        {
            // Center window
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            window_flags |= ImGuiWindowFlags_NoMove;
        }
        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::Begin("Statistics", nullptr, window_flags);
        const auto stats = Application::Get().GetStats();
        ImGui::Text("%.1f fps (%.2fms)", stats.GetFPS(), stats.GetFrameTime());
        ImGui::End();
    }

    ImGuiLayer::ImGuiLayer() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        auto& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        ImGui_ImplOpenGL3_Init("#version 410");
    }

    bool ImGuiLayer::OnMouseMoved(MouseMovedEvent& event) {
        auto& io = ImGui::GetIO();
        io.MousePos = {event.GetX(), event.GetY()};
        return false;
    }

    bool ImGuiLayer::OnMouseButtonPressed(MouseButtonPressedEvent& event) {
        auto& io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = true;
        return false;
    }

    bool ImGuiLayer::OnMouseScrolled(MouseScrolledEvent& event) {
        auto& io = ImGui::GetIO();
        io.MouseWheel += event.GetYOffset();
        io.MouseWheelH += event.GetXOffset();
        return false;
    }

    bool ImGuiLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& event) {
        auto& io = ImGui::GetIO();
        io.MouseDown[event.GetMouseButton()] = false;
        return false;
    }

    ImGuiLayer::~ImGuiLayer() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::UpdateDisplaySize() {
        auto& io = ImGui::GetIO();
        const auto& window = Application::Get().GetWindow();
        const auto w = window.GetWidth();
        const auto h = window.GetHeight();
        io.DisplaySize = {static_cast<float>(w), static_cast<float>(h)};
        const int fbW = window.GetFramebufferWidth();
        const int fbH = window.GetFramebufferHeight();
        if (w > 0 && h > 0) {
            io.DisplayFramebufferScale = {
                static_cast<float>(fbW) / static_cast<float>(w), static_cast<float>(fbH) / static_cast<float>(h)
            };
        }
    }

    void ImGuiLayer::Begin(const float dt) {
        auto& io = ImGui::GetIO();
        io.DeltaTime = dt;
        UpdateDisplaySize();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
