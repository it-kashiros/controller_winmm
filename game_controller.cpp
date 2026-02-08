/*********************************************************************
 * \file   game_controller.cpp
 * \brief  ゲームコントローラー入力管理（WinMM版）
 *********************************************************************/
#include "game_controller.h"

// 静的メンバ変数の定義
int GameController::s_workingControllerId = -1;
GamepadState GameController::s_currentState = {};
GamepadState GameController::s_prevState = {};
GamepadCaps GameController::s_caps = {};

// デバイス情報を取得
void GameController::UpdateCaps(int id) {
    JOYCAPS jc;
    if (joyGetDevCaps(id, &jc, sizeof(JOYCAPS)) != JOYERR_NOERROR) {
        s_caps.valid = false;
        return;
    }

    s_caps.valid = true;
    s_caps.manufacturerId = jc.wMid;
    s_caps.productId = jc.wPid;

    // 製品名をコピー
    for (int i = 0; i < 31 && jc.szPname[i] != '\0'; i++) {
        s_caps.productName[i] = (char)jc.szPname[i];
    }
    s_caps.productName[31] = '\0';

    s_caps.numAxes = jc.wNumAxes;
    s_caps.numButtons = jc.wNumButtons;

    // 各軸の範囲
    s_caps.xMin = jc.wXmin; s_caps.xMax = jc.wXmax;
    s_caps.yMin = jc.wYmin; s_caps.yMax = jc.wYmax;
    s_caps.zMin = jc.wZmin; s_caps.zMax = jc.wZmax;
    s_caps.rMin = jc.wRmin; s_caps.rMax = jc.wRmax;
    s_caps.uMin = jc.wUmin; s_caps.uMax = jc.wUmax;
    s_caps.vMin = jc.wVmin; s_caps.vMax = jc.wVmax;

    // 十字キーの数
    s_caps.numPov = (jc.wCaps & JOYCAPS_HASPOV) ? 1 : 0;

    // 各軸の有無
    s_caps.hasZ = (jc.wCaps & JOYCAPS_HASZ) != 0;
    s_caps.hasR = (jc.wCaps & JOYCAPS_HASR) != 0;
    s_caps.hasU = (jc.wCaps & JOYCAPS_HASU) != 0;
    s_caps.hasV = (jc.wCaps & JOYCAPS_HASV) != 0;
    s_caps.hasPov = (jc.wCaps & JOYCAPS_HASPOV) != 0;
}

// 指定した軸の値を取得
int GameController::GetGamepadValue(int id, int func) {
    JOYINFOEX ji;
    ji.dwSize = sizeof(JOYINFOEX);
    ji.dwFlags = JOY_RETURNALL;

    if (joyGetPosEx(id, &ji) != JOYERR_NOERROR)
        return -1;

    switch (func) {
    case 0: return ji.dwXpos;    // 左スティックX
    case 1: return ji.dwYpos;    // 左スティックY
    case 2: return ji.dwZpos;    // L2トリガー（または合算トリガー）
    case 3: return ji.dwButtons; // ボタン
    case 4: return ji.dwRpos;    // 右スティックX
    case 5: return ji.dwUpos;    // 右スティックY
    case 6: return ji.dwVpos;    // R2トリガー
    case 8: return ji.dwPOV;     // 十字キー
    default: return -1;
    }
}

// 状態を更新
bool GameController::UpdateState() {
    // 前フレームの状態を保存
    s_prevState = s_currentState;

    // 接続中のコントローラーがなければ探す
    if (s_workingControllerId == -1) {
        for (int id = 0; id < 16; id++) {
            int testValue = GetGamepadValue(id, 3);
            if (testValue != -1) {
                s_workingControllerId = id;
                UpdateCaps(id);
                break;
            }
        }

        // 見つからなかった
        if (s_workingControllerId == -1) {
            s_currentState.connected = false;
            return false;
        }
    }

    // 各軸の値を取得
    int leftX = GetGamepadValue(s_workingControllerId, 0);
    int leftY = GetGamepadValue(s_workingControllerId, 1);
    int rightX = GetGamepadValue(s_workingControllerId, 4);
    int rightY = GetGamepadValue(s_workingControllerId, 5);
    int triggerZ = GetGamepadValue(s_workingControllerId, 2);
    int triggerV = GetGamepadValue(s_workingControllerId, 6);
    int buttons = GetGamepadValue(s_workingControllerId, 3);
    int pov = GetGamepadValue(s_workingControllerId, 8);

    // 切断チェック
    if (leftX == -1) {
        s_workingControllerId = -1;
        s_currentState.connected = false;
        s_caps.valid = false;
        return false;
    }

    s_currentState.connected = true;

    // デバッグ用に値を保存
    s_currentState.axisLeftX = leftX;
    s_currentState.axisLeftY = leftY;
    s_currentState.axisRightX = rightX;
    s_currentState.axisRightY = rightY;
    s_currentState.axisTriggerL = triggerZ;
    s_currentState.axisTriggerR = triggerV;
    s_currentState.buttonsRaw = buttons;
    s_currentState.povValue = pov;

    // スティック値を正規化（-1.0 1.0）
    s_currentState.leftStickX = (float)(leftX - 32767) / 32767.0f;
    s_currentState.leftStickY = (float)(leftY - 32767) / 32767.0f;
    s_currentState.rightStickX = (float)(rightX - 32767) / 32767.0f;
    s_currentState.rightStickY = (float)(rightY - 32767) / 32767.0f;

    // デッドゾーン適用
    s_currentState.leftStickX = GamepadState::ApplyDeadzone(s_currentState.leftStickX);
    s_currentState.leftStickY = GamepadState::ApplyDeadzone(s_currentState.leftStickY);
    s_currentState.rightStickX = GamepadState::ApplyDeadzone(s_currentState.rightStickX);
    s_currentState.rightStickY = GamepadState::ApplyDeadzone(s_currentState.rightStickY);

    // トリガー値を正規化
    // 一部コントローラーはL2/R2が1つの軸（Z軸）に合算されている
    if (s_caps.hasV) {
        // Z軸とV軸が別々にある場合（XInputコントローラーなど）
        s_currentState.triggerL = (float)triggerZ / 65535.0f;
        s_currentState.triggerR = (float)triggerV / 65535.0f;
    } else {
        // Z軸のみの場合（DirectInputコントローラーなど）
        // 32767が中央（未入力）、65535方向がL2、0方向がR2
        const int CENTER = 32767;
        const int DEADZONE = 1000;

        if (triggerZ > CENTER + DEADZONE) {
            // L2が押されている
            s_currentState.triggerL = (float)(triggerZ - CENTER) / (float)(65535 - CENTER);
            s_currentState.triggerR = 0.0f;
        } else if (triggerZ < CENTER - DEADZONE) {
            // R2が押されている
            s_currentState.triggerL = 0.0f;
            s_currentState.triggerR = (float)(CENTER - triggerZ) / (float)CENTER;
        } else {
            // 未入力
            s_currentState.triggerL = 0.0f;
            s_currentState.triggerR = 0.0f;
        }
    }

    // 十字キーの処理
    if (pov == 65535 || pov == -1) {
        // 未入力
        s_currentState.dpadUp = false;
        s_currentState.dpadDown = false;
        s_currentState.dpadLeft = false;
        s_currentState.dpadRight = false;
    } else {
        // 角度を度に変換（0.01度単位なので100で割る）
        int angle = pov / 100;
        s_currentState.dpadUp = (angle >= 315 || angle <= 45);
        s_currentState.dpadRight = (angle >= 45 && angle <= 135);
        s_currentState.dpadDown = (angle >= 135 && angle <= 225);
        s_currentState.dpadLeft = (angle >= 225 && angle <= 315);
    }

    // ボタンの処理
    s_currentState.buttonDown = (buttons & (1 << 0)) != 0;  // ボタン0
    s_currentState.buttonRight = (buttons & (1 << 1)) != 0;  // ボタン1
    s_currentState.buttonLeft = (buttons & (1 << 2)) != 0;  // ボタン2
    s_currentState.buttonUp = (buttons & (1 << 3)) != 0;  // ボタン3
    s_currentState.buttonL1 = (buttons & (1 << 4)) != 0;  // ボタン4
    s_currentState.buttonR1 = (buttons & (1 << 5)) != 0;  // ボタン5
    s_currentState.buttonSelect = (buttons & (1 << 6)) != 0;  // ボタン6
    s_currentState.buttonStart = (buttons & (1 << 7)) != 0;  // ボタン7
    s_currentState.buttonL3 = (buttons & (1 << 8)) != 0;  // ボタン8
    s_currentState.buttonR3 = (buttons & (1 << 9)) != 0;  // ボタン9
    s_currentState.buttonExtra1 = (buttons & (1 << 10)) != 0; // ボタン10
    s_currentState.buttonExtra2 = (buttons & (1 << 11)) != 0; // ボタン11

    // トリガーをボタンとしても判定（50%以上でオン）
    s_currentState.buttonL2 = (s_currentState.triggerL > 0.5f);
    s_currentState.buttonR2 = (s_currentState.triggerR > 0.5f);

    return true;
}
