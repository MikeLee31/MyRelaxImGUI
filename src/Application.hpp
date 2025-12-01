// Application.hpp - 在此自由设计界面
#pragma once
#include <imgui.h>
namespace app
{
    void RenderUI()
    {
        ImGui::Begin("My First GUI");

        // 示例：创建一个红色按钮
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1));
        if (ImGui::Button("Click for a Surprise"))
        {
            // 在此添加点击事件
        }
        ImGui::PopStyleColor();

        ImGui::End();
    }
}
