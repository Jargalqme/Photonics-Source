#include "pch.h"
#include "InputManager.h"

//=============================================================================
// コンストラクタ
// 全ての入力オブジェクトとトラッカーを初期化
//=============================================================================
InputManager::InputManager()
    : m_mouseSensitivity(0.001f)
    , m_gamepadSensitivity(0.05f)
    , m_keyboardState{}
    , m_mouseState{}
    , m_gamePadState{}
    , m_prevMouseState{}
{
    // DirectXTK入力オブジェクト生成
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();
    m_gamePad = std::make_unique<GamePad>();

    // 状態トラッカー生成（エッジ検出用）
    m_keyboardTracker = std::make_unique<Keyboard::KeyboardStateTracker>();
    m_mouseTracker = std::make_unique<Mouse::ButtonStateTracker>();
    m_gamePadTracker = std::make_unique<GamePad::ButtonStateTracker>();
}

//=============================================================================
// 初期化
//=============================================================================
void InputManager::initialize(HWND window)
{
    // マウスにウィンドウハンドルを設定
    m_mouse->SetWindow(window);

    // 相対モード: カーソル非表示、無限移動（FPSカメラ用）
    // 絶対モード: カーソル表示、UI操作用
    m_mouse->SetMode(Mouse::MODE_RELATIVE);
}

void InputManager::update()
{
    // 前フレームの状態を保存（絶対モード時のマウスデルタ計算に使用）
    m_prevMouseState = m_mouseState;

    // 現在フレームの状態を取得
    m_keyboardState = m_keyboard->GetState();
    m_mouseState = m_mouse->GetState();
    m_gamePadState = m_gamePad->GetState(0);  // プレイヤー0

    // トラッカー更新（Pressed/Released判定用）
    m_keyboardTracker->Update(m_keyboardState);
    m_mouseTracker->Update(m_mouseState);
    m_gamePadTracker->Update(m_gamePadState);
}

//=============================================================================
// キーボード入力
//=============================================================================

// 継続判定: キーが押されている間ずっとtrue
bool InputManager::isKeyDown(Keyboard::Keys key) const
{
    return m_keyboardState.IsKeyDown(key);
}

// 単発判定: キーが押された瞬間のみtrue
bool InputManager::isKeyPressed(Keyboard::Keys key) const
{
    return m_keyboardTracker->IsKeyPressed(key);
}

// 単発判定: キーが離された瞬間のみtrue
bool InputManager::isKeyReleased(Keyboard::Keys key) const
{
    return m_keyboardTracker->IsKeyReleased(key);
}

//=============================================================================
// マウス入力
//=============================================================================

// マウス座標取得（絶対モード時のみ有効）
Vector2 InputManager::getMousePosition() const
{
    return Vector2(static_cast<float>(m_mouseState.x),
        static_cast<float>(m_mouseState.y));
}

// マウス移動量取得
// 相対モード: x,yがそのまま移動量
// 絶対モード: 前フレームとの差分を計算
Vector2 InputManager::getMouseDelta() const
{
    if (m_currentMouseMode == Mouse::MODE_RELATIVE)
    {
        // 相対モード: 値がそのまま移動量
        return Vector2(static_cast<float>(m_mouseState.x),
            static_cast<float>(m_mouseState.y));
    }
    else  // 絶対モード
    {
        // 絶対モード: 現在座標 - 前フレーム座標 = 移動量
        return Vector2(static_cast<float>(m_mouseState.x - m_prevMouseState.x),
            static_cast<float>(m_mouseState.y - m_prevMouseState.y));
    }
}

// マウスボタン - 継続判定
bool InputManager::isLeftMouseDown() const
{
    return m_mouseState.leftButton;
}

bool InputManager::isRightMouseDown() const
{
    return m_mouseState.rightButton;
}

bool InputManager::isMiddleMouseDown() const
{
    return m_mouseState.middleButton;
}

// マウスボタン - 単発判定
bool InputManager::isLeftMousePressed() const
{
    return m_mouseTracker->leftButton == Mouse::ButtonStateTracker::PRESSED;
}

bool InputManager::isRightMousePressed() const
{
    return m_mouseTracker->rightButton == Mouse::ButtonStateTracker::PRESSED;
}


// 移動入力をVector2として取得
// キーボード（WASD/矢印）とゲームパッド（左スティック）を統合
Vector2 InputManager::getMovement() const
{
    Vector2 movement = Vector2::Zero;

    //--- キーボード入力 ---
    if (m_keyboardState.W || m_keyboardState.Up)
        movement.y += 1.0f;
    if (m_keyboardState.S || m_keyboardState.Down)
        movement.y -= 1.0f;
    if (m_keyboardState.A || m_keyboardState.Left)
        movement.x -= 1.0f;
    if (m_keyboardState.D || m_keyboardState.Right)
        movement.x += 1.0f;

    // 斜め移動を正規化（斜めが速くならないように）
    if (movement.LengthSquared() > 0)
        movement.Normalize();

    //--- ゲームパッド入力を加算 ---
    if (m_gamePadState.IsConnected())
    {
        movement += Vector2(m_gamePadState.thumbSticks.leftX,
            m_gamePadState.thumbSticks.leftY);

        // 合計値が1を超えないように制限
        if (movement.LengthSquared() > 1)
            movement.Normalize();
    }

    return movement;
}

// 視点入力を取得（カメラ操作用）
// マウス移動量とゲームパッド右スティックを統合
Vector2 InputManager::getLookInput() const
{
    // マウス移動量 × 感度
    Vector2 look = getMouseDelta() * m_mouseSensitivity;

    // ゲームパッド右スティックを加算
    if (m_gamePadState.IsConnected())
    {
        look += Vector2(m_gamePadState.thumbSticks.rightX,
            -m_gamePadState.thumbSticks.rightY) * m_gamepadSensitivity;
    }

    return look;
}

void InputManager::setCursorVisible(bool visible)
{
    m_cursorVisible = visible;
    setMouseMode(visible ? Mouse::MODE_ABSOLUTE : Mouse::MODE_RELATIVE);
}

// マウスモード切替
void InputManager::setMouseMode(Mouse::Mode mode)
{
    m_currentMouseMode = mode;
    m_mouse->SetMode(mode);
}

//=============================================================================
// ゲームパッド入力
//=============================================================================

//--- 接続確認 ---
bool InputManager::isGamePadConnected(int player) const
{
    return m_gamePad->GetState(player).IsConnected();
}

//--- アナログスティック ---
Vector2 InputManager::getGamePadLeftStick(int player) const
{
    auto state = m_gamePad->GetState(player);
    return Vector2(state.thumbSticks.leftX, state.thumbSticks.leftY);
}

Vector2 InputManager::getGamePadRightStick(int player) const
{
    auto state = m_gamePad->GetState(player);
    return Vector2(state.thumbSticks.rightX, state.thumbSticks.rightY);
}

//--- アナログトリガー（0.0 ～ 1.0）---
float InputManager::getLeftTrigger() const
{
    return m_gamePadState.IsConnected() ? m_gamePadState.triggers.left : 0.0f;
}

float InputManager::getRightTrigger() const
{
    return m_gamePadState.IsConnected() ? m_gamePadState.triggers.right : 0.0f;
}

bool InputManager::getGamePadLeftTrigger() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->leftTrigger == GamePad::ButtonStateTracker::PRESSED;
}

//--- ボタン継続判定（押している間true）---
bool InputManager::isGamePadADown() const
{
    return m_gamePadState.IsConnected() && m_gamePadState.buttons.a;
}

bool InputManager::isGamePadBDown() const
{
    return m_gamePadState.IsConnected() && m_gamePadState.buttons.b;
}

bool InputManager::isGamePadXDown() const
{
    return m_gamePadState.IsConnected() && m_gamePadState.buttons.x;
}

bool InputManager::isGamePadYDown() const
{
    return m_gamePadState.IsConnected() && m_gamePadState.buttons.y;
}

bool InputManager::isGamePadLBDown() const
{
    return m_gamePadState.IsConnected() && m_gamePadState.buttons.leftShoulder;
}

bool InputManager::isGamePadRBDown() const
{
    return m_gamePadState.IsConnected() && m_gamePadState.buttons.rightShoulder;
}

//--- ボタン単発判定（押した瞬間のみtrue）---
bool InputManager::isGamePadAPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->a == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadBPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->b == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadXPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->x == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadYPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->y == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadLBPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->leftShoulder == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadRBPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->rightShoulder == GamePad::ButtonStateTracker::PRESSED;
}

//--- D-Pad 単発判定（押した瞬間のみtrue）---
bool InputManager::isGamePadDPadUpPressed() const 
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->dpadUp == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadDPadDownPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->dpadDown == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadDPadLeftPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->dpadLeft == GamePad::ButtonStateTracker::PRESSED;
}

bool InputManager::isGamePadDPadRightPressed() const
{
    return m_gamePadState.IsConnected() && m_gamePadTracker->dpadRight == GamePad::ButtonStateTracker::PRESSED;
}

void InputManager::setVibration(float leftMotor, float rightMotor, int player)
{
    if (m_gamePad->GetState(player).IsConnected())
    {
        m_gamePad->SetVibration(player, leftMotor, rightMotor);
    }
}

void InputManager::stopVibration(int player)
{
    if (m_gamePad->GetState(player).IsConnected())
    {
        m_gamePad->SetVibration(player, 0.0f, 0.0f);
    }
}