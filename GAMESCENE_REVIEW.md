# GameScene.h/cpp — Code Review Fixes

> Work through these one by one. Check off items as you go.

---

## Priority 1: Magic Numbers

These are pervasive and the single biggest readability/maintainability win.

---

### 1.1 Camera projection parameters (duplicated)

- **File:** `GameScene.cpp` lines 28, 635
- **Problem:** `45.0f, 0.1f, 1000.0f` (FOV, near clip, far clip) are hardcoded in two separate places. Change one and you'll forget the other.
- **Fix:** Add `constexpr` at the top of the file or in the header:

```cpp
static constexpr float kDefaultFOV = 45.0f;
static constexpr float kNearClip   = 0.1f;
static constexpr float kFarClip    = 1000.0f;
```

- [ ] Done

---

### 1.2 Core positions, colors, and health (repeated 6x)

- **File:** `GameScene.cpp` lines 57–73, 184–186
- **Problem:** Each core is set up with near-identical copy-pasted blocks. Health `150.0f` appears 6 times.
- **Fix:** Define a config struct and a data array:

```cpp
struct CoreConfig {
    Vector3 position;
    Color   color;
    float   health;
};

static constexpr float kCoreHealth = 150.0f;

static const CoreConfig kCoreConfigs[] = {
    { Vector3( 0.0f, 1.0f,  85.0f), Color(1,0,0), kCoreHealth },
    { Vector3(-66.0f, 1.0f, -50.0f), Color(0,1,0), kCoreHealth },
    { Vector3( 66.0f, 1.0f, -50.0f), Color(0,0,1), kCoreHealth },
};
```

Store cores in `std::array<std::unique_ptr<Core>, 3>` and initialize in a loop.

- [ ] Done

---

### 1.3 Death beam pool config

- **File:** `GameScene.cpp` lines 88–91
- **Problem:** `20`, `30.0f`, `0.5f` — pool size, height, duration are unnamed.
- **Fix:**

```cpp
static constexpr int   kDeathBeamPoolSize = 20;
static constexpr float kDeathBeamHeight   = 30.0f;
static constexpr float kDeathBeamDuration = 0.5f;
```

- [ ] Done

---

### 1.4 Music/beat config

- **File:** `GameScene.cpp` lines 104–106
- **Problem:** `120.0f` BPM, `15.931f` start delay, `150.0f` song duration are hardcoded.
- **Fix:**

```cpp
static constexpr float kBPM             = 120.0f;
static constexpr float kMusicStartDelay = 15.931f;
static constexpr float kSongDuration    = 150.0f;
```

- [ ] Done

---

### 1.5 Floating shapes parameters

- **File:** `GameScene.cpp` lines 134–168
- **Problem:** Shape count (`18`), size ranges, position ranges, `628/100.0f` (approximating 2pi), speed ranges — all unnamed. The `628` trick is especially obscure.
- **Fix:** Use named constants. Replace `628/100.0f` with `XM_2PI`:

```cpp
static constexpr int   kFloatingShapeCount     = 18;
static constexpr float kShapeMinDistance        = 60.0f;
static constexpr float kShapeDistanceRange      = 120.0f;
static constexpr float kShapeMinHeight          = 30.0f;
static constexpr float kShapeHeightRange        = 80.0f;
static constexpr float kShapeMinRotationSpeed   = 0.1f;
static constexpr float kShapeRotationSpeedRange = 0.3f;
```

- [ ] Done

---

### 1.6 Combat / weapon numbers

- **File:** `GameScene.cpp` lines 311, 315, 322
- **Problem:** Projectile Y offset (`1.0f`), aim distance (`150.0f`), shake intensity/duration (`2.0f, 0.1f`).
- **Fix:**

```cpp
static constexpr float kProjectileSpawnYOffset = 1.0f;
static constexpr float kAimRayLength           = 150.0f;
static constexpr float kFireShakeIntensity     = 2.0f;
static constexpr float kFireShakeDuration      = 0.1f;
```

- [ ] Done

---

### 1.7 Vibration / rumble

- **File:** `GameScene.cpp` lines 436–437
- **Problem:** `0.7f, 0.4f` vibration strengths and `0.2f` rumble timer are unnamed.
- **Fix:**

```cpp
static constexpr float kBoostVibrationLeft  = 0.7f;
static constexpr float kBoostVibrationRight = 0.4f;
static constexpr float kBoostRumbleDuration = 0.2f;
```

- [ ] Done

---

### 1.8 Billboard config

- **File:** `GameScene.cpp` lines 82–84
- **Problem:** Position `(0, 90, 700)`, size `900.0f`, frame rate `60.0f` are hardcoded.
- **Fix:** Same pattern — named constants.

- [ ] Done

---

### 1.9 Shape colors in Render

- **File:** `GameScene.cpp` lines 586–588
- **Problem:** Cyan `(0, 0.8, 1, 0.7)` and purple `(0.6, 0, 1, 0.7)` are inline.
- **Fix:**

```cpp
static const Color kShapeColorCyan   = Color(0.0f, 0.8f, 1.0f, 0.7f);
static const Color kShapeColorPurple = Color(0.6f, 0.0f, 1.0f, 0.7f);
```

- [ ] Done

---

### 1.10 Input thresholds

- **File:** `GameScene.cpp` lines 334, 386, 393
- **Problem:** `0.7f` Y-rotation multiplier, `0.01f` stick deadzone, `0.001f` movement threshold.
- **Fix:**

```cpp
static constexpr float kStickDeadzone       = 0.01f;
static constexpr float kMovementThreshold   = 0.001f;
static constexpr float kShapeYRotationScale = 0.7f;
```

- [ ] Done

---

## Priority 2: Bad Patterns

---

### 2.1 Replace `rand()` with `<random>`

- **File:** `GameScene.cpp` lines 138, 141–145, 150–152, 161–163, 168, 504
- **Problem:** `rand()` is low quality, has modulo bias, and is not thread-safe.
- **Fix:** Add a member `std::mt19937 m_rng;` seeded in the constructor. Use distributions:

```cpp
// In header
std::mt19937 m_rng{ std::random_device{}() };

// Usage
std::uniform_int_distribution<int> shapeDist(0, 2);
std::uniform_real_distribution<float> angleDist(0.0f, XM_2PI);
std::uniform_real_distribution<float> distanceDist(kShapeMinDistance, kShapeMinDistance + kShapeDistanceRange);
```

- [ ] Done

---

### 2.2 Float equality on Vector3 for retargeting

- **File:** `GameScene.cpp` lines 487–492
- **Problem:** `targetPos == m_coreRed->GetPosition()` — exact float comparison is fragile. One frame of accumulated drift and the comparison silently fails.
- **Fix:** Give enemies a target core pointer or ID instead of comparing positions:

```cpp
// In Enemy class, store a Core* or core index
if (!enemy->GetTargetCore()->IsAlive())
    needsRetarget = true;
```

- [ ] Done

---

### 2.3 Parallel arrays → struct

- **File:** `GameScene.h` lines 82–85, used throughout `.cpp`
- **Problem:** Four parallel vectors (`m_floatingShapes`, `m_shapePositions`, `m_shapeRotations`, `m_shapeSpeeds`) must stay in sync manually. Adding/removing an element in one but not the others = silent bug.
- **Fix:** Define a struct:

```cpp
struct FloatingShape {
    std::unique_ptr<GeometricPrimitive> primitive;
    Vector3 position;
    Vector3 rotation;
    float   speed;
};

std::vector<FloatingShape> m_floatingShapes;
```

- [ ] Done

---

### 2.4 `m_renderer` lifecycle is fragile

- **File:** `GameScene.cpp` line 565 (set), lines 197–202, 211–216, 539–554 (used)
- **Problem:** `m_renderer` is assigned inside `Render()` but read in `Enter()`, `Exit()`, and `CheckGameConditions()`. On the very first `Enter()` call, it is still `nullptr`, so the color mask reset silently does nothing.
- **Fix:** Pass `Renderer*` via `Initialize()` or store it from the scene manager. Don't rely on render-time side effects for game logic.

- [ ] Done

---

## Priority 3: Structural Issues

---

### 3.1 Escape key calls `PostQuitMessage(0)`

- **File:** `GameScene.cpp` line 246
- **Problem:** Immediately kills the process. No save, no confirmation, no pause menu.
- **Fix:** Transition to a pause scene:

```cpp
m_sceneManager->TransitionTo("PauseMenu");
```

- [ ] Done

---

### 3.2 Inconsistent naming conventions

- **Problem:** Mixed across the codebase:

| Pattern    | Examples                                     |
|------------|----------------------------------------------|
| PascalCase | `Initialize()`, `Render()`, `Reset()`        |
| camelCase  | `initialize()`, `render()`, `reset()`, `finalize()` |

- **Fix:** Pick one convention (PascalCase for public methods is the DirectX/Windows norm) and apply project-wide. This is a larger refactor — do it in a dedicated pass.

- [ ] Done

---

### 3.3 Redundant `Cleanup()` method

- **File:** `GameScene.cpp` lines 219–238
- **Problem:** Every member is a `unique_ptr`. The destructor cleans them up automatically. The manual `.reset()` calls add nothing unless destruction order matters (which is undocumented). Also `m_animatedBillboard.reset()` is commented out with no explanation.
- **Fix:** If order doesn't matter, delete the body. If it does, document **why** with comments.

- [ ] Done

---

### 3.4 Commented-out dead code

- **File:** `GameScene.cpp` lines 237, 325–327, 571–572, 662
- **Problem:** `m_animatedBillboard` and `m_terrain` render/cleanup calls are scattered as commented-out code. This rots over time and confuses readers.
- **Fix:** Remove the commented code entirely. If it's a planned feature, track it in an issue tracker, not inline.

- [ ] Done

---

### 3.5 Division by zero risk

- **File:** `GameScene.cpp` lines 27, 635
- **Problem:** `float(size.bottom)` or `float(height)` could be zero (e.g., minimized window), causing undefined behavior.
- **Fix:** Guard with a minimum:

```cpp
float aspect = (height > 0) ? float(width) / float(height) : 1.0f;
```

- [ ] Done

---

### 3.6 Misaligned indentation on `playSound`

- **File:** `GameScene.cpp` lines 319–320
- **Problem:**

```cpp
if (m_audioManager)
m_audioManager->playSound("shoot");
```

Looks like `playSound` is outside the `if` guard at a glance.

- **Fix:** Put on one line or use braces:

```cpp
if (m_audioManager)
    m_audioManager->playSound("shoot");
```

- [ ] Done

---

### 3.7 Cache repeated calls in `Render()`

- **File:** `GameScene.cpp` lines 568–621
- **Problem:** `getViewMatrix()` and `getProjectionMatrix()` are called ~12 times each per frame.
- **Fix:** Cache at the top of `Render()`:

```cpp
auto view = m_camera->getViewMatrix();
auto proj = m_camera->getProjectionMatrix();
```

Then pass `view`/`proj` everywhere.

- [ ] Done
