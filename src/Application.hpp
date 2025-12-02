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
    enum class GameState
    {
        WAITING,  // 等待开始
        PLAYING,  // 游戏中
        GAME_OVER // 游戏结束
    };

    // 定义游戏常量（可调整）
    constexpr float PADDLE_WIDTH = 120.0f; // 横板宽度
    constexpr float PADDLE_HEIGHT = 12.0f; // 横板高度
    constexpr float BALL_RADIUS = 8.0f;    // 弹珠半径
    constexpr float BALL_SPEED = 300.0f;   // 弹珠基础速度
    constexpr float PADDLE_SPEED = 450.0f; // 横板移动速度

    // ImVec2 运算辅助函数
    inline ImVec2 operator+(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x + b.x, a.y + b.y); }
    inline ImVec2 operator-(const ImVec2 &a, const ImVec2 &b) { return ImVec2(a.x - b.x, a.y - b.y); }
    inline ImVec2 operator*(const ImVec2 &a, float s) { return ImVec2(a.x * s, a.y * s); }
    inline ImVec2 operator*(float s, const ImVec2 &a) { return ImVec2(a.x * s, a.y * s); }

    // 游戏核心逻辑和渲染
    void RenderUI()
    {
        static GameState state = GameState::WAITING;
        static float paddleX = 500.0f;
        static int score = 0;
        static float gameTime = 0.0f;
        static bool keys[2] = {false, false};

        // 可调参数
        static float ballSpeed = 300.0f;
        static float paddleWidth = 180.0f;

        // 多球支持
        struct Ball
        {
            ImVec2 pos;
            ImVec2 vel;
        };
        static std::vector<Ball> balls;

        auto resetBalls = [&](float paddleTop)
        {
            balls.clear();
            balls.push_back(Ball{
                ImVec2(paddleX, paddleTop - 40.0f),
                ImVec2(ballSpeed * 0.7f, -ballSpeed * 0.7f)});
        };

        ImGuiIO &io = ImGui::GetIO();
        const float deltaTime = io.DeltaTime;

        // ================== 【新增】固定游戏画布尺寸 ==================
        const ImVec2 GAME_CANVAS_SIZE = ImVec2(900, 600); // 你可以自由调整这个值
        const float margin = 10.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.1f, 1.0f));

        // 设置窗口大小（比画布大一点，留出UI空间）
        ImGui::SetNextWindowSize(ImVec2(1000, 800), ImGuiCond_Always);

        if (ImGui::Begin("弹珠游戏", nullptr,
                         ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_AlwaysAutoResize))
        {
            // ========== 【新增】顶部状态栏 + FPS ==========
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.5f, 1.0f));
            float fps = io.Framerate;
            ImGui::Text("得分: %d | 时间: %.1f秒 | 球数: %d | FPS: %.1f",
                        score, gameTime, (int)balls.size(), fps);
            ImGui::PopStyleColor();

            // ========== 【核心修改】游戏画布区域 ==========
            // 使用 Dummy 占位 + 绝对坐标绘制
            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            ImGui::Dummy(GAME_CANVAS_SIZE); // 占位，确保画布有固定大小

            ImDrawList *drawList = ImGui::GetWindowDrawList();

            // 游戏边界
            ImVec2 gameAreaMin = canvasPos;
            ImVec2 gameAreaMax = operator+(canvasPos, GAME_CANVAS_SIZE);

            // 边界框
            drawList->AddRect(gameAreaMin, gameAreaMax,
                              ImColor(0.8f, 0.8f, 1.0f, 1.0f), 0.0f, 0, 2.0f);

            float paddleTop = gameAreaMax.y - PADDLE_HEIGHT;

            // 初始化球（如果为空）
            if (state == GameState::PLAYING && balls.empty())
            {
                balls.push_back(Ball{
                    ImVec2(paddleX, paddleTop - 40.0f),
                    ImVec2(ballSpeed * 0.7f, -ballSpeed * 0.7f)});
            }

            // =============== 游戏逻辑更新 ===============
            if (state == GameState::PLAYING)
            {
                gameTime += deltaTime;

                // 更新横板
                if (keys[0])
                    paddleX -= PADDLE_SPEED * deltaTime;
                if (keys[1])
                    paddleX += PADDLE_SPEED * deltaTime;

                paddleX = std::max(gameAreaMin.x + paddleWidth / 2.0f,
                                   std::min(gameAreaMax.x - paddleWidth / 2.0f, paddleX));

                // 更新所有弹珠
                for (size_t i = 0; i < balls.size(); ++i)
                {
                    Ball &ball = balls[i];
                    ball.pos.x += ball.vel.x * deltaTime;
                    ball.pos.y += ball.vel.y * deltaTime;

                    // 边界碰撞
                    if (ball.pos.x <= gameAreaMin.x + BALL_RADIUS)
                    {
                        ball.pos.x = gameAreaMin.x + BALL_RADIUS;
                        ball.vel.x = -ball.vel.x;
                    }
                    else if (ball.pos.x >= gameAreaMax.x - BALL_RADIUS)
                    {
                        ball.pos.x = gameAreaMax.x - BALL_RADIUS;
                        ball.vel.x = -ball.vel.x;
                    }
                    if (ball.pos.y <= gameAreaMin.y + BALL_RADIUS)
                    {
                        ball.pos.y = gameAreaMin.y + BALL_RADIUS;
                        ball.vel.y = -ball.vel.y;
                    }
                }

                // 横板碰撞
                for (size_t i = 0; i < balls.size(); ++i)
                {
                    Ball &ball = balls[i];
                    if (ball.pos.y >= paddleTop - BALL_RADIUS &&
                        ball.pos.y <= paddleTop + BALL_RADIUS &&
                        ball.pos.x >= paddleX - paddleWidth / 2.0f - BALL_RADIUS &&
                        ball.pos.x <= paddleX + paddleWidth / 2.0f + BALL_RADIUS &&
                        ball.vel.y > 0)
                    {
                        float hitPos = (ball.pos.x - paddleX) / (paddleWidth / 2.0f);
                        hitPos = std::clamp(hitPos, -0.95f, 0.95f);
                        ball.vel.y = -std::abs(ball.vel.y);
                        ball.vel.x = ballSpeed * hitPos * 1.2f;
                        score++;
                    }
                }

                // 死亡检测
                for (size_t i = 0; i < balls.size();)
                {
                    if (balls[i].pos.y >= gameAreaMax.y - BALL_RADIUS)
                    {
                        balls.erase(balls.begin() + i);
                    }
                    else
                    {
                        ++i;
                    }
                }
                if (balls.empty())
                {
                    state = GameState::GAME_OVER;
                }
            }

            // 绘制横板
            ImVec2 paddleMin = ImVec2(paddleX - paddleWidth / 2, gameAreaMax.y - PADDLE_HEIGHT);
            ImVec2 paddleMax = ImVec2(paddleX + paddleWidth / 2, gameAreaMax.y);
            drawList->AddRectFilled(paddleMin, paddleMax,
                                    ImColor(0.2f, 0.8f, 0.2f, 1.0f));

            // 绘制所有弹珠
            for (const auto &ball : balls)
            {
                drawList->AddCircleFilled(ball.pos, BALL_RADIUS,
                                          ImColor(1.0f, 0.3f, 0.3f, 1.0f));
            }

            // 状态文字
            if (state == GameState::WAITING)
            {
                ImVec2 center = operator+(gameAreaMin, operator*(GAME_CANVAS_SIZE, 0.5f));
                drawList->AddText(operator-(center, ImVec2(80, 15)),
                                  ImColor(1.0f, 1.0f, 0.7f, 1.0f),
                                  "点击[开始游戏]!");
            }
            else if (state == GameState::GAME_OVER)
            {
                ImVec2 center = operator+(gameAreaMin, operator*(GAME_CANVAS_SIZE, 0.5f));
                drawList->AddText(operator-(center, ImVec2(60, 15)),
                                  ImColor(1.0f, 0.4f, 0.4f, 1.0f),
                                  "游戏结束!");
                char scoreText[32];
                snprintf(scoreText, sizeof(scoreText), "得分: %d", score);
                drawList->AddText(operator-(center, ImVec2(70, 40)),
                                  ImColor(1.0f, 0.8f, 0.3f, 1.0f),
                                  scoreText);
            }

            // ========== 【新增】控制按钮组 ==========
            ImGui::SetCursorPosY(canvasPos.y + GAME_CANVAS_SIZE.y + 10);
            ImGui::BeginGroup();
            ImVec2 btnSize = ImVec2(120, 40);

            if (state != GameState::PLAYING)
            {
                if (ImGui::Button(state == GameState::GAME_OVER ? "重新开始" : "开始游戏", btnSize))
                {
                    score = 0;
                    gameTime = 0.0f;
                    paddleX = gameAreaMin.x + GAME_CANVAS_SIZE.x * 0.5f;
                    state = GameState::PLAYING;
                    resetBalls(paddleTop);
                }
                ImGui::SameLine();
            }

            if (state == GameState::PLAYING)
            {
                if (ImGui::Button("暂停", btnSize))
                {
                    state = GameState::WAITING;
                }
                ImGui::SameLine();
            }

            bool addBall = false;
            if (ImGui::Button("增加一个球 (空格)", btnSize))
            {
                addBall = true;
            }
            ImGui::SameLine();
            if (state == GameState::PLAYING && ImGui::IsKeyPressed(ImGuiKey_Space))
            {
                addBall = true;
            }
            if (addBall && state == GameState::PLAYING)
            {
                balls.push_back(Ball{
                    ImVec2(paddleX, paddleTop - 40.0f),
                    ImVec2(ballSpeed * 0.7f, -ballSpeed * 0.7f)});
            }

            if (ImGui::Button("退出游戏", btnSize))
            {
                state = GameState::WAITING;
                score = 0;
                gameTime = 0.0f;
                balls.clear();
            }
            ImGui::EndGroup();

            // ========== 【修复】控制说明与规则文字分离 ==========
            ImGui::SetCursorPosY(canvasPos.y + GAME_CANVAS_SIZE.y + 70);
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), u8"控制: ← → 方向键 或 拖拽横板，空格键增加球");
            ImGui::SetCursorPosY(canvasPos.y + GAME_CANVAS_SIZE.y + 100);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f),
                               u8"游戏规则: 用底部横板接住弹珠, 每次接住得1分。所有弹珠掉落则游戏结束。");

            // ========== 【新增】滑块控件放在底部，不与文字重叠 ==========
            ImGui::SetCursorPosY(canvasPos.y + GAME_CANVAS_SIZE.y + 140);
            ImGui::SliderFloat("球速", &ballSpeed, 100.0f, 800.0f, "%.0f");
            ImGui::SliderFloat("板长", &paddleWidth, 60.0f, 400.0f, "%.0f");

            // ========== 【新增】鼠标/键盘控制绑定 ==========
            ImGui::SetCursorScreenPos(canvasPos);
            ImGui::InvisibleButton("GamePanel", GAME_CANVAS_SIZE);

            if (ImGui::IsItemHovered() || ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
            {
                keys[0] = ImGui::IsKeyDown(ImGuiKey_LeftArrow);
                keys[1] = ImGui::IsKeyDown(ImGuiKey_RightArrow);
            }

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                paddleX += io.MouseDelta.x;
            }

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
                io.MousePos.y > paddleTop &&
                io.MousePos.y < gameAreaMax.y)
            {
                paddleX = io.MousePos.x;
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
}