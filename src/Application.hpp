// Application.hpp - 弹珠游戏实现 (单文件版本)
#pragma once
#include <imgui.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm> // 修复 std::clamp

namespace app
{
    // 游戏状态枚举
    enum class GameState {
        WAITING,    // 等待开始
        PLAYING,    // 游戏中
        GAME_OVER   // 游戏结束
    };

    // 定义游戏常量（可调整）
    constexpr float PADDLE_WIDTH = 120.0f;   // 横板宽度
    constexpr float PADDLE_HEIGHT = 12.0f;  // 横板高度
    constexpr float BALL_RADIUS = 8.0f;     // 弹珠半径
    constexpr float BALL_SPEED = 300.0f;    // 弹珠基础速度
    constexpr float PADDLE_SPEED = 450.0f;  // 横板移动速度

    // ImVec2 运算辅助函数
    inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
    inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }
    inline ImVec2 operator*(const ImVec2& a, float s) { return ImVec2(a.x * s, a.y * s); }
    inline ImVec2 operator*(float s, const ImVec2& a) { return ImVec2(a.x * s, a.y * s); }

    // 游戏核心逻辑和渲染
    void RenderUI()
    {
        // 静态变量保存游戏状态（保持帧间连续性）
        static GameState state = GameState::WAITING;
        static float paddleX = 500.0f; // 横板X位置（中心点）
        static int score = 0;          // 得分
        static float gameTime = 0.0f;  // 游戏运行时间
        static bool keys[2] = {false, false}; // 左/右移动状态 [0]=左, [1]=右

        // 可调参数
        static float ballSpeed = 300.0f;      // 球速
        static float paddleWidth = 180.0f;    // 板长

        // 多球支持
        struct Ball {
            ImVec2 pos;
            ImVec2 vel;
        };
        static std::vector<Ball> balls;

        // 初始化球
        auto resetBalls = [&](float paddleTop) {
            balls.clear();
            balls.push_back(Ball{
                ImVec2(paddleX, paddleTop - 40.0f),
                ImVec2(ballSpeed * 0.7f, -ballSpeed * 0.7f)
            });
        };

        // 获取ImGui系统信息
        ImGuiIO& io = ImGui::GetIO();
        const float deltaTime = io.DeltaTime;
        ImVec2 windowSize = ImGui::GetWindowSize();

        // 计算游戏区域（窗口内预留边距）
        const float margin = 20.0f;
        const float buttonAreaHeight = 160.0f; // 增大底部按钮区高度
        ImVec2 gameAreaMin = operator+(ImGui::GetCursorScreenPos(), ImVec2(margin, margin));
        ImVec2 gameAreaMax = operator+(gameAreaMin, ImVec2(
            windowSize.x - 2 * margin,
            windowSize.y - 2 * margin - buttonAreaHeight
        ));
        ImVec2 gameAreaSize = operator-(gameAreaMax, gameAreaMin);

        float paddleTop = gameAreaMax.y - PADDLE_HEIGHT;

        // 游戏开始时如果没有球，自动添加一个球
        if (state == GameState::PLAYING && balls.empty()) {
            balls.push_back(Ball{
                ImVec2(paddleX, paddleTop - 40.0f),
                ImVec2(ballSpeed * 0.7f, -ballSpeed * 0.7f)
            });
        }

        // =============== 游戏逻辑更新 ===============
        if (state == GameState::PLAYING) {
            gameTime += deltaTime;

            // 1. 更新横板位置（根据按键状态）
            if (keys[0]) paddleX -= PADDLE_SPEED * deltaTime;
            if (keys[1]) paddleX += PADDLE_SPEED * deltaTime;

            // 2. 限制横板在游戏区域内
            paddleX = std::max(gameAreaMin.x + paddleWidth / 2.0f,
                      std::min(gameAreaMax.x - paddleWidth / 2.0f, paddleX));

            // 3. 更新所有弹珠位置
            for (size_t i = 0; i < balls.size(); ++i) {
                Ball& ball = balls[i];
                ball.pos.x += ball.vel.x * deltaTime;
                ball.pos.y += ball.vel.y * deltaTime;

                // 4. 边界碰撞检测
                // 左右边界
                if (ball.pos.x <= gameAreaMin.x + BALL_RADIUS) {
                    ball.pos.x = gameAreaMin.x + BALL_RADIUS;
                    ball.vel.x = -ball.vel.x;
                }
                else if (ball.pos.x >= gameAreaMax.x - BALL_RADIUS) {
                    ball.pos.x = gameAreaMax.x - BALL_RADIUS;
                    ball.vel.x = -ball.vel.x;
                }
                // 顶部边界
                if (ball.pos.y <= gameAreaMin.y + BALL_RADIUS) {
                    ball.pos.y = gameAreaMin.y + BALL_RADIUS;
                    ball.vel.y = -ball.vel.y;
                }
            }

            // 5. 横板碰撞检测和得分
            for (size_t i = 0; i < balls.size(); ++i) {
                Ball& ball = balls[i];
                if (ball.pos.y >= paddleTop - BALL_RADIUS &&
                    ball.pos.y <= paddleTop + BALL_RADIUS &&
                    ball.pos.x >= paddleX - paddleWidth / 2.0f - BALL_RADIUS &&
                    ball.pos.x <= paddleX + paddleWidth / 2.0f + BALL_RADIUS &&
                    ball.vel.y > 0) {
                    float hitPos = (ball.pos.x - paddleX) / (paddleWidth / 2.0f);
                    hitPos = std::clamp(hitPos, -0.95f, 0.95f);
                    ball.vel.y = -std::abs(ball.vel.y);
                    ball.vel.x = ballSpeed * hitPos * 1.2f;
                    score++;
                }
            }

            // 6. 死亡检测（只移除掉落的球，全部掉落才 GAME_OVER）
            for (size_t i = 0; i < balls.size();) {
                if (balls[i].pos.y >= gameAreaMax.y - BALL_RADIUS) {
                    balls.erase(balls.begin() + i);
                } else {
                    ++i;
                }
            }
            if (balls.empty()) {
                state = GameState::GAME_OVER;
            }
        }

        // =============== UI 渲染 ===============
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.1f, 1.0f));
        ImGui::SetNextWindowSize(ImVec2(1000, 900), ImGuiCond_Always);

        if (ImGui::Begin("弹珠游戏", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            // 顶部状态栏
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.5f, 1.0f));
            ImGui::Text("得分: %d | 时间: %.1f秒 | 球数: %d", score, gameTime, (int)balls.size());
            ImGui::PopStyleColor();

            // 游戏区域
            ImGui::SetCursorScreenPos(gameAreaMin);
            ImGui::Dummy(gameAreaSize);

            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // 边界
            drawList->AddRect(gameAreaMin, gameAreaMax,
                ImColor(0.8f, 0.8f, 1.0f, 1.0f), 0.0f, 0, 2.0f);

            // 横板
            ImVec2 paddleMin = ImVec2(paddleX - paddleWidth/2, gameAreaMax.y - PADDLE_HEIGHT);
            ImVec2 paddleMax = ImVec2(paddleX + paddleWidth/2, gameAreaMax.y);
            drawList->AddRectFilled(paddleMin, paddleMax,
                ImColor(0.2f, 0.8f, 0.2f, 1.0f));

            // 所有弹珠
            for (const auto& ball : balls) {
                drawList->AddCircleFilled(ball.pos, BALL_RADIUS,
                    ImColor(1.0f, 0.3f, 0.3f, 1.0f));
            }

            // 状态文字
            if (state == GameState::WAITING) {
                ImVec2 center = operator+(gameAreaMin, operator*(gameAreaSize, 0.5f));
                drawList->AddText(operator-(center, ImVec2(80, 15)),
                    ImColor(1.0f, 1.0f, 0.7f, 1.0f),
                    "点击[开始游戏]!");
            }
            else if (state == GameState::GAME_OVER) {
                ImVec2 center = operator+(gameAreaMin, operator*(gameAreaSize, 0.5f));
                drawList->AddText(operator-(center, ImVec2(60, 15)),
                    ImColor(1.0f, 0.4f, 0.4f, 1.0f),
                    "游戏结束!");
                char scoreText[32];
                snprintf(scoreText, sizeof(scoreText), "得分: %d", score);
                drawList->AddText(operator-(center, ImVec2(70, 40)),
                    ImColor(1.0f, 0.8f, 0.3f, 1.0f),
                    scoreText);
            }

            // 控制按钮
            ImGui::SetCursorPosY(gameAreaMax.y + 10);
            ImGui::BeginGroup();
            ImVec2 btnSize = ImVec2(120, 40);

            // 开始/重置按钮
            if (state != GameState::PLAYING) {
                if (ImGui::Button(state == GameState::GAME_OVER ? "重新开始" : "开始游戏", btnSize)) {
                    score = 0;
                    gameTime = 0.0f;
                    paddleX = gameAreaSize.x * 0.5f + gameAreaMin.x;
                    state = GameState::PLAYING;
                    resetBalls(paddleTop); // 传入 paddleTop
                }
                ImGui::SameLine();
            }

            // 暂停/继续按钮
            if (state == GameState::PLAYING) {
                if (ImGui::Button("暂停", btnSize)) {
                    state = GameState::WAITING;
                }
                ImGui::SameLine();
            }

            // 增加一个球按钮（支持空格键）
            bool addBall = false;
            if (ImGui::Button("增加一个球 (空格)", btnSize)) {
                addBall = true;
            }
            ImGui::SameLine();
            if (state == GameState::PLAYING && ImGui::IsKeyPressed(ImGuiKey_Space)) {
                addBall = true;
            }
            if (addBall && state == GameState::PLAYING) {
                balls.push_back(Ball{
                    ImVec2(paddleX, paddleTop - 40.0f),
                    ImVec2(ballSpeed * 0.7f, -ballSpeed * 0.7f)
                });
            }

            // 退出按钮
            if (ImGui::Button("退出游戏", btnSize)) {
                state = GameState::WAITING;
                score = 0;
                gameTime = 0.0f;
                balls.clear();
            }
            ImGui::EndGroup();

            // 横板控制
            ImGui::SetCursorScreenPos(gameAreaMin);
            ImGui::InvisibleButton("GamePanel", gameAreaSize);

            if (ImGui::IsItemHovered() || ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
                keys[0] = ImGui::IsKeyDown(ImGuiKey_LeftArrow);
                keys[1] = ImGui::IsKeyDown(ImGuiKey_RightArrow);
            }

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                paddleX += io.MouseDelta.x;
            }

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
                io.MousePos.y > paddleTop &&
                io.MousePos.y < gameAreaMax.y) {
                paddleX = io.MousePos.x;
            }

            ImGui::SetCursorPosY(gameAreaMax.y + 60);
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), u8"控制: ← → 方向键 或 拖拽横板，空格键增加球");
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                u8"游戏规则: 用底部横板接住弹珠, 每次接住得1分。所有弹珠掉落则游戏结束。");

            // 球速和板长滑块控件只保留底部一组
            ImGui::SetCursorPosY(gameAreaMax.y + buttonAreaHeight - 60);
            ImGui::SliderFloat("球速", &ballSpeed, 100.0f, 800.0f, "%.0f");
            ImGui::SliderFloat("板长", &paddleWidth, 60.0f, 400.0f, "%.0f");
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
}