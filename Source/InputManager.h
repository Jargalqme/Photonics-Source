#pragma once

#include <Keyboard.h>
#include <Mouse.h>
#include <GamePad.h>
#include <memory>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class InputManager {
public:

    // コンストラクタ・デストラクタ
    InputManager();
    ~InputManager() = default;

    //-------------------------------------------------------------------------
    // 初期化
    // @param window: マウス入力を受け取るウィンドウハンドル
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
    // ゲームパッド入力
    //=========================================================================

    // 接続確認
    bool isGamePadConnected(int player = 0) const;

    // アナログスティック（-1.0 ～ +1.0）
    Vector2 getGamePadLeftStick(int player = 0) const;
    Vector2 getGamePadRightStick(int player = 0) const;

    // アナログトリガー（0.0 ～ 1.0）
    float getLeftTrigger() const;
    float getRightTrigger() const;

    bool getGamePadLeftTrigger() const;

    // ボタン - 継続判定（押している間ずっとtrue）
    bool isGamePadADown() const;
    bool isGamePadBDown() const;
    bool isGamePadXDown() const;
    bool isGamePadYDown() const;
    bool isGamePadLBDown() const;  // 左バンパー/ショルダー
    bool isGamePadRBDown() const;  // 右バンパー/ショルダー

    // ボタン - 単発判定（押した瞬間のみtrue）
    bool isGamePadAPressed() const;
    bool isGamePadBPressed() const;
    bool isGamePadXPressed() const;
    bool isGamePadYPressed() const;
    bool isGamePadLBPressed() const;
    bool isGamePadRBPressed() const;

    // D-Pad - 単発判定
    bool isGamePadDPadUpPressed() const;
    bool isGamePadDPadDownPressed() const;
    bool isGamePadDPadLeftPressed() const;
    bool isGamePadDPadRightPressed() const;

    //=========================================================================
    // 設定
    //=========================================================================

    void setMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
    void setGamePadSensitivity(float sensitivity) { m_gamepadSensitivity = sensitivity; }
    void setVibration(float leftMotor, float rightMotor, int player = 0);
    void stopVibration(int player = 0);

    // Cursor visibility (also switches mouse mode)
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
    std::unique_ptr<GamePad> m_gamePad;

    //-------------------------------------------------------------------------
    // 入力状態（現在フレーム）
    //-------------------------------------------------------------------------
    Keyboard::State m_keyboardState;
    Mouse::State m_mouseState;
    GamePad::State m_gamePadState;

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
    std::unique_ptr<GamePad::ButtonStateTracker> m_gamePadTracker;

    //-------------------------------------------------------------------------
    // 設定値
    //-------------------------------------------------------------------------
    float m_mouseSensitivity;
    float m_gamepadSensitivity;
    Mouse::Mode m_currentMouseMode = Mouse::MODE_RELATIVE;
    bool m_cursorVisible = false;
};




