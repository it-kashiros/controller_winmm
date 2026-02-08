/*********************************************************************
 * \file   game_controller.h
 * \brief  ゲームコントローラー入力管理（WinMM版）
 *********************************************************************/
#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <cmath>

#pragma comment(lib, "winmm.lib")

 // コントローラーの入力状態
struct GamepadState {
    // 左スティック（-1.0?1.0、デッドゾーン適用）
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;

    // 右スティック（-1.0?1.0、デッドゾーン適用）
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;

    // トリガー（0.0?1.0）
    float triggerL = 0.0f;
    float triggerR = 0.0f;

    // 十字キー
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;

    // 十字キーの値（0?35900、未入力時は65535）
    int povValue = -1;

    // メインボタン（A/B/X/Y、◯/×/□/△など）
    bool buttonDown = false;   // A、×、B
    bool buttonRight = false;  // B、◯、A
    bool buttonLeft = false;   // X、□、Y
    bool buttonUp = false;     // Y、△、X

    // ショルダーボタン（L1/R1、LB/RB）
    bool buttonL1 = false;
    bool buttonR1 = false;

    // トリガーボタン（L2/R2、LT/RT）※閾値50%でオン判定
    bool buttonL2 = false;
    bool buttonR2 = false;

    // システムボタン
    bool buttonStart = false;   // Start、Options、+
    bool buttonSelect = false;  // Select、Share、-

    // スティック押し込み
    bool buttonL3 = false;
    bool buttonR3 = false;

    // その他のボタン（PSボタン、Xboxボタンなど）
    bool buttonExtra1 = false;
    bool buttonExtra2 = false;

    // ボタンのビットフラグ（デバッグ用）
    unsigned int buttonsRaw = 0;

    // 接続状態
    bool connected = false;

    // 各軸の値（0?65535、デバッグ用）
    int axisLeftX = 0;
    int axisLeftY = 0;
    int axisRightX = 0;
    int axisRightY = 0;
    int axisTriggerL = 0;
    int axisTriggerR = 0;

    // いずれかのボタンが押されているか
    bool IsAnyButtonPressed() const {
        return buttonDown || buttonRight || buttonLeft || buttonUp ||
            buttonL1 || buttonR1 || buttonL2 || buttonR2 ||
            buttonL3 || buttonR3 ||
            buttonStart || buttonSelect ||
            buttonExtra1 || buttonExtra2 ||
            dpadUp || dpadDown || dpadLeft || dpadRight;
    }

    // デッドゾーン適用
    static float ApplyDeadzone(float value, float deadzone = 0.15f) {
        if (fabs(value) < deadzone) return 0.0f;

        float sign = (value > 0) ? 1.0f : -1.0f;
        float adjustedValue = (fabs(value) - deadzone) / (1.0f - deadzone);
        return sign * adjustedValue;
    }
};

// コントローラーのデバイス情報
struct GamepadCaps {
    // 有効かどうか
    bool valid = false;

    // 製造者ID
    unsigned short manufacturerId = 0;

    // 製品ID
    unsigned short productId = 0;

    // 製品名
    char productName[32] = {};

    // 軸の数
    int numAxes = 0;

    // ボタンの数
    int numButtons = 0;

    // X軸（左スティックX）の範囲
    unsigned int xMin = 0;
    unsigned int xMax = 0;

    // Y軸（左スティックY）の範囲
    unsigned int yMin = 0;
    unsigned int yMax = 0;

    // Z軸（L2トリガー）の範囲
    unsigned int zMin = 0;
    unsigned int zMax = 0;

    // R軸（右スティックX）の範囲
    unsigned int rMin = 0;
    unsigned int rMax = 0;

    // U軸（右スティックY）の範囲
    unsigned int uMin = 0;
    unsigned int uMax = 0;

    // V軸（R2トリガー）の範囲
    unsigned int vMin = 0;
    unsigned int vMax = 0;

    // 十字キー（POV）の数
    int numPov = 0;

    // Z軸があるか
    bool hasZ = false;

    // R軸があるか
    bool hasR = false;

    // U軸があるか
    bool hasU = false;

    // V軸があるか
    bool hasV = false;

    // 十字キー（POV）があるか
    bool hasPov = false;
};

class GameController {
private:
    // 接続中のコントローラーID（-1は未接続）
    static int s_workingControllerId;

    // 現在フレームの状態
    static GamepadState s_currentState;

    // 前フレームの状態
    static GamepadState s_prevState;

    // デバイス情報
    static GamepadCaps s_caps;

    // 指定した軸の値を取得
    static int GetGamepadValue(int id, int func);

    // 状態を更新
    static bool UpdateState();

    // デバイス情報を更新
    static void UpdateCaps(int id);

public:
    // 初期化
    static bool Initialize() {
        s_workingControllerId = -1;
        s_currentState = {};
        s_prevState = {};
        s_caps = {};
        return true;
    }

    // 毎フレーム呼ぶ
    static void Update() {
        UpdateState();
    }

    // 現在の状態を取得
    static const GamepadState& GetCurrentState() { return s_currentState; }

    // 前フレームの状態を取得
    static const GamepadState& GetPrevState() { return s_prevState; }

    // デバイス情報を取得
    static const GamepadCaps& GetCaps() { return s_caps; }

    // コントローラーIDを取得
    static int GetControllerId() { return s_workingControllerId; }

    // ========================================
    // Press判定（押している間ずっとtrue）
    // ========================================

    // メインボタン
    static bool IsPressed_ButtonDown() { return s_currentState.buttonDown; }
    static bool IsPressed_ButtonRight() { return s_currentState.buttonRight; }
    static bool IsPressed_ButtonLeft() { return s_currentState.buttonLeft; }
    static bool IsPressed_ButtonUp() { return s_currentState.buttonUp; }

    // ショルダー・トリガー
    static bool IsPressed_L1() { return s_currentState.buttonL1; }
    static bool IsPressed_R1() { return s_currentState.buttonR1; }
    static bool IsPressed_L2() { return s_currentState.buttonL2; }
    static bool IsPressed_R2() { return s_currentState.buttonR2; }

    // スティック押し込み
    static bool IsPressed_L3() { return s_currentState.buttonL3; }
    static bool IsPressed_R3() { return s_currentState.buttonR3; }

    // システムボタン
    static bool IsPressed_Start() { return s_currentState.buttonStart; }
    static bool IsPressed_Select() { return s_currentState.buttonSelect; }

    // その他ボタン
    static bool IsPressed_Extra1() { return s_currentState.buttonExtra1; }
    static bool IsPressed_Extra2() { return s_currentState.buttonExtra2; }

    // 十字キー
    static bool IsPressed_DpadUp() { return s_currentState.dpadUp; }
    static bool IsPressed_DpadDown() { return s_currentState.dpadDown; }
    static bool IsPressed_DpadLeft() { return s_currentState.dpadLeft; }
    static bool IsPressed_DpadRight() { return s_currentState.dpadRight; }

    // ========================================
    // Trigger判定（押した瞬間だけtrue）
    // ========================================

    // メインボタン
    static bool IsTrigger_ButtonDown() { return s_currentState.buttonDown && !s_prevState.buttonDown; }
    static bool IsTrigger_ButtonRight() { return s_currentState.buttonRight && !s_prevState.buttonRight; }
    static bool IsTrigger_ButtonLeft() { return s_currentState.buttonLeft && !s_prevState.buttonLeft; }
    static bool IsTrigger_ButtonUp() { return s_currentState.buttonUp && !s_prevState.buttonUp; }

    // ショルダー・トリガー
    static bool IsTrigger_L1() { return s_currentState.buttonL1 && !s_prevState.buttonL1; }
    static bool IsTrigger_R1() { return s_currentState.buttonR1 && !s_prevState.buttonR1; }
    static bool IsTrigger_L2() { return s_currentState.buttonL2 && !s_prevState.buttonL2; }
    static bool IsTrigger_R2() { return s_currentState.buttonR2 && !s_prevState.buttonR2; }

    // スティック押し込み
    static bool IsTrigger_L3() { return s_currentState.buttonL3 && !s_prevState.buttonL3; }
    static bool IsTrigger_R3() { return s_currentState.buttonR3 && !s_prevState.buttonR3; }

    // システムボタン
    static bool IsTrigger_Start() { return s_currentState.buttonStart && !s_prevState.buttonStart; }
    static bool IsTrigger_Select() { return s_currentState.buttonSelect && !s_prevState.buttonSelect; }

    // その他ボタン
    static bool IsTrigger_Extra1() { return s_currentState.buttonExtra1 && !s_prevState.buttonExtra1; }
    static bool IsTrigger_Extra2() { return s_currentState.buttonExtra2 && !s_prevState.buttonExtra2; }

    // 十字キー
    static bool IsTrigger_DpadUp() { return s_currentState.dpadUp && !s_prevState.dpadUp; }
    static bool IsTrigger_DpadDown() { return s_currentState.dpadDown && !s_prevState.dpadDown; }
    static bool IsTrigger_DpadLeft() { return s_currentState.dpadLeft && !s_prevState.dpadLeft; }
    static bool IsTrigger_DpadRight() { return s_currentState.dpadRight && !s_prevState.dpadRight; }

    // ========================================
    // Release判定（離した瞬間だけtrue）
    // ========================================

    // メインボタン
    static bool IsRelease_ButtonDown() { return !s_currentState.buttonDown && s_prevState.buttonDown; }
    static bool IsRelease_ButtonRight() { return !s_currentState.buttonRight && s_prevState.buttonRight; }
    static bool IsRelease_ButtonLeft() { return !s_currentState.buttonLeft && s_prevState.buttonLeft; }
    static bool IsRelease_ButtonUp() { return !s_currentState.buttonUp && s_prevState.buttonUp; }

    // ショルダー・トリガー
    static bool IsRelease_L1() { return !s_currentState.buttonL1 && s_prevState.buttonL1; }
    static bool IsRelease_R1() { return !s_currentState.buttonR1 && s_prevState.buttonR1; }
    static bool IsRelease_L2() { return !s_currentState.buttonL2 && s_prevState.buttonL2; }
    static bool IsRelease_R2() { return !s_currentState.buttonR2 && s_prevState.buttonR2; }

    // スティック押し込み
    static bool IsRelease_L3() { return !s_currentState.buttonL3 && s_prevState.buttonL3; }
    static bool IsRelease_R3() { return !s_currentState.buttonR3 && s_prevState.buttonR3; }

    // システムボタン
    static bool IsRelease_Start() { return !s_currentState.buttonStart && s_prevState.buttonStart; }
    static bool IsRelease_Select() { return !s_currentState.buttonSelect && s_prevState.buttonSelect; }

    // その他ボタン
    static bool IsRelease_Extra1() { return !s_currentState.buttonExtra1 && s_prevState.buttonExtra1; }
    static bool IsRelease_Extra2() { return !s_currentState.buttonExtra2 && s_prevState.buttonExtra2; }

    // 十字キー
    static bool IsRelease_DpadUp() { return !s_currentState.dpadUp && s_prevState.dpadUp; }
    static bool IsRelease_DpadDown() { return !s_currentState.dpadDown && s_prevState.dpadDown; }
    static bool IsRelease_DpadLeft() { return !s_currentState.dpadLeft && s_prevState.dpadLeft; }
    static bool IsRelease_DpadRight() { return !s_currentState.dpadRight && s_prevState.dpadRight; }

    // ========================================
    // スティック・トリガー値取得
    // ========================================

    // 左スティック（-1.0?1.0）
    static float GetLeftStickX() { return s_currentState.leftStickX; }
    static float GetLeftStickY() { return s_currentState.leftStickY; }

    // 右スティック（-1.0?1.0）
    static float GetRightStickX() { return s_currentState.rightStickX; }
    static float GetRightStickY() { return s_currentState.rightStickY; }

    // トリガー（0.0?1.0）
    static float GetTriggerL() { return s_currentState.triggerL; }
    static float GetTriggerR() { return s_currentState.triggerR; }

    // ========================================
    // 接続状態
    // ========================================

    // コントローラーが接続されているか
    static bool IsConnected() { return s_currentState.connected; }

    // 終了処理
    static void Finalize() {
        s_workingControllerId = -1;
        s_currentState = {};
        s_prevState = {};
        s_caps = {};
    }
};