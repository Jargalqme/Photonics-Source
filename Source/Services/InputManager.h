#pragma once

#include <Keyboard.h>
#include <Mouse.h>
#include <memory>

using DirectX::SimpleMath::Vector2;
using DirectX::Keyboard;
using DirectX::Mouse;

class InputManager
{
public:
    InputManager();
    ~InputManager() = default;

    //-------------------------------------------------------------------------
    // 初期化
    //-------------------------------------------------------------------------
    void initialize(HWND window);

    void update();

    //=========================================================================
    // キーボード入力
    //=========================================================================

    // キーが押されているか（継続判定）
    bool isKeyDown(Keyboard::Keys key) const;

    // キーが今フレーム押されたか（単発判定）
    bool isKeyPressed(Keyboard::Keys key) const;

    // キーが今フレーム離されたか（単発判定）
    bool isKeyReleased(Keyboard::Keys key) const;

    //=========================================================================
    // マウス入力
    //=========================================================================

    // マウス座標（絶対モード時のスクリーン座標）
    Vector2 getMousePosition() const;

    // マウス移動量（相対モード時はそのまま、絶対モード時は差分計算）
    Vector2 getMouseDelta() const;

    // マウスボタン状態（継続判定）
    bool isLeftMouseDown() const;
    bool isRightMouseDown() const;
    bool isMiddleMouseDown() const;

    // マウスボタン状態（単発判定）
    bool isLeftMousePressed() const;
    bool isRightMousePressed() const;

    //=========================================================================
    // 高レベル入力（ゲーム用に抽象化）
    //=========================================================================

    // 移動入力: WASD/矢印キー → Vector2に変換
    // 斜め移動時は正規化される
    Vector2 getMovement() const;

    // 視点入力: マウス移動量 × 感度
    Vector2 getLookInput() const;

    //=========================================================================
    // 設定
    //=========================================================================

    void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

    // カーソル表示切替（マウスモードも連動）
    void setCursorVisible(bool visible);
    bool isCursorVisible() const { return m_cursorVisible; }

    // マウスモード切替
    // MODE_RELATIVE: カーソル非表示、FPSカメラ向け
    // MODE_ABSOLUTE: カーソル表示、UI操作向け
    void setMouseMode(Mouse::Mode mode);

private:
    //-------------------------------------------------------------------------
    // DirectXTK入力オブジェクト
    //-------------------------------------------------------------------------
    std::unique_ptr<Keyboard> m_keyboard;
    std::unique_ptr<Mouse> m_mouse;

    //-------------------------------------------------------------------------
    // 入力状態（現在フレーム）
    //-------------------------------------------------------------------------
    Keyboard::State m_keyboardState;
    Mouse::State m_mouseState;

    //-------------------------------------------------------------------------
    // 入力状態（前フレーム）- マウスデルタ計算用（絶対モード時）
    //-------------------------------------------------------------------------
    Mouse::State m_prevMouseState;

    //-------------------------------------------------------------------------
    // 状態トラッカー（Pressed/Released判定用）
    // DirectXTKが提供するヘルパークラス
    //-------------------------------------------------------------------------
    std::unique_ptr<Keyboard::KeyboardStateTracker> m_keyboardTracker;
    std::unique_ptr<Mouse::ButtonStateTracker> m_mouseTracker;

    //-------------------------------------------------------------------------
    // 設定値
    //-------------------------------------------------------------------------
    float m_mouseSensitivity;
    Mouse::Mode m_currentMouseMode = Mouse::MODE_RELATIVE;
    bool m_cursorVisible = false;
};
