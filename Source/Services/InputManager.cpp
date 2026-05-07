#include "pch.h"
#include "InputManager.h"

using namespace DirectX;

// === 初期化 ===
InputManager::InputManager()
    : m_mouseSensitivity(0.001f)
    , m_keyboardState{}
    , m_mouseState{}
    , m_prevMouseState{}
{
    // DirectXTK入力オブジェクト生成
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();

    // 状態トラッカー生成（エッジ検出用）
    m_keyboardTracker = std::make_unique<Keyboard::KeyboardStateTracker>();
    m_mouseTracker = std::make_unique<Mouse::ButtonStateTracker>();
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

    // トラッカー更新（Pressed/Released判定用）
    m_keyboardTracker->Update(m_keyboardState);
    m_mouseTracker->Update(m_mouseState);
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


// 移動入力をVector2として取得（WASD/矢印キー）
Vector2 InputManager::getMovement() const
{
    Vector2 movement = Vector2::Zero;

    if (m_keyboardState.W || m_keyboardState.Up)
    {
        movement.y += 1.0f;
    }
    if (m_keyboardState.S || m_keyboardState.Down)
    {
        movement.y -= 1.0f;
    }
    if (m_keyboardState.A || m_keyboardState.Left)
    {
        movement.x -= 1.0f;
    }
    if (m_keyboardState.D || m_keyboardState.Right)
    {
        movement.x += 1.0f;
    }

    // 斜め移動を正規化（斜めが速くならないように）
    if (movement.LengthSquared() > 0)
    {
        movement.Normalize();
    }

    return movement;
}

// 視点入力（マウス移動量 × 感度）
Vector2 InputManager::getLookInput() const
{
    return getMouseDelta() * m_mouseSensitivity;
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
