/*********************************************************************
 * \file   main.cpp
 * \brief  コントローラー入力デバッグ用
 *********************************************************************/
#include <cstdio>
#include <cstring>
#include <conio.h>
#include <windows.h>
#include "game_controller.h"

 // カーソルを左上に戻す
void ClearScreen() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 固定幅で1行出力（79文字+改行）
void PrintLine(const char* pStr) {
    std::printf("%-79s\n", pStr);
}

// スティック用バー文字列生成
void GetStickBar(char* pBuf, float value) {
    int pos = static_cast<int>((value + 1.0f) * 6.0f);
    if (pos < 0) pos = 0;
    if (pos > 12) pos = 12;
    pBuf[0] = '[';
    for (int i = 0; i < 13; i++) {
        if (i == 6) pBuf[i + 1] = '|';
        else if (i == pos) pBuf[i + 1] = '*';
        else pBuf[i + 1] = '-';
    }
    pBuf[14] = ']';
    pBuf[15] = '\0';
}

// トリガー用バー文字列生成
void GetTriggerBar(char* pBuf, float value) {
    int filled = static_cast<int>(value * 8);
    pBuf[0] = '[';
    for (int i = 0; i < 8; i++) {
        pBuf[i + 1] = (i < filled) ? '=' : ' ';
    }
    pBuf[9] = ']';
    pBuf[10] = '\0';
}

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // ウィンドウサイズ設定
    SMALL_RECT rect = { 0, 0, 80, 25 };
    SetConsoleWindowInfo(hConsole, TRUE, &rect);
    COORD bufSize = { 80, 25 };
    SetConsoleScreenBufferSize(hConsole, bufSize);

    // カーソル非表示
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    GameController::Initialize();

    char line[128];
    char bar1[16], bar2[16], bar3[16], bar4[16];
    char tbar1[16], tbar2[16];
    bool isRunning = true;

    while (isRunning) {
        // ESCキーで終了
        if (_kbhit()) {
            int key = _getch();
            if (key == 27) {
                isRunning = false;
                break;
            }
        }

        GameController::Update();
        ClearScreen();

        if (!GameController::IsConnected()) {
            PrintLine("===============================================================================");
            PrintLine("                         CONTROLLER DEBUG MONITOR                              ");
            PrintLine("===============================================================================");
            PrintLine("");
            PrintLine(" Controller not connected...");
            PrintLine("");
            for (int i = 0; i < 15; i++) {
                PrintLine("");
            }
            PrintLine("-------------------------------------------------------------------------------");
            PrintLine(" ESC to exit");
            Sleep(100);
            continue;
        }

        const GamepadState& state = GameController::GetCurrentState();
        const GamepadCaps& caps = GameController::GetCaps();

        GetStickBar(bar1, state.leftStickX);
        GetStickBar(bar2, state.leftStickY);
        GetStickBar(bar3, state.rightStickX);
        GetStickBar(bar4, state.rightStickY);
        GetTriggerBar(tbar1, state.triggerL);
        GetTriggerBar(tbar2, state.triggerR);

        // ボタン表示用
        const char* pDpadUp = state.dpadUp ? "[U]" : " U ";
        const char* pDpadDown = state.dpadDown ? "[D]" : " D ";
        const char* pDpadLeft = state.dpadLeft ? "[L]" : " L ";
        const char* pDpadRight = state.dpadRight ? "[R]" : " R ";

        // MAINボタン（位置ベース：上=X、左=Y、右=A、下=B）Switch配列
        const char* pMainUp = state.buttonUp ? "[X]" : " X ";
        const char* pMainDown = state.buttonDown ? "[B]" : " B ";
        const char* pMainLeft = state.buttonLeft ? "[Y]" : " Y ";
        const char* pMainRight = state.buttonRight ? "[A]" : " A ";

        const char* pBtnL1 = state.buttonL1 ? "[L1]" : " L1 ";
        const char* pBtnR1 = state.buttonR1 ? "[R1]" : " R1 ";
        const char* pBtnL2 = state.triggerL > 0.5f ? "[L2]" : " L2 ";
        const char* pBtnR2 = state.triggerR > 0.5f ? "[R2]" : " R2 ";
        const char* pBtnL3 = state.buttonL3 ? "[L3]" : " L3 ";
        const char* pBtnR3 = state.buttonR3 ? "[R3]" : " R3 ";

        const char* pBtnSelect = state.buttonSelect ? "[SELECT]" : " SELECT ";
        const char* pBtnStart = state.buttonStart ? "[START]" : " START  ";

        const char* pBtnExtra1 = state.buttonExtra1 ? "[EX1]" : " EX1 ";
        const char* pBtnExtra2 = state.buttonExtra2 ? "[EX2]" : " EX2 ";

        PrintLine("===============================================================================");
        PrintLine("                         CONTROLLER DEBUG MONITOR                              ");
        PrintLine("===============================================================================");

        std::sprintf(line, " Device: %-26s ID:%d  Axes:%d  Buttons:%d",
            caps.productName, GameController::GetControllerId(), caps.numAxes, caps.numButtons);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        std::sprintf(line, " Raw | LX:%5d LY:%5d RX:%5d RY:%5d POV:%5d Btn:0x%04X",
            state.axisLeftX, state.axisLeftY, state.axisRightX, state.axisRightY,
            state.povValue, state.buttonsRaw);
        PrintLine(line);

        std::sprintf(line, "     | L2:%5d R2:%5d", state.axisTriggerL, state.axisTriggerR);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        std::sprintf(line, " L Stick | X:%6.2f %s   Y:%6.2f %s",
            state.leftStickX, bar1, state.leftStickY, bar2);
        PrintLine(line);

        std::sprintf(line, " R Stick | X:%6.2f %s   Y:%6.2f %s",
            state.rightStickX, bar3, state.rightStickY, bar4);
        PrintLine(line);

        std::sprintf(line, " Trigger | L2:%5.2f %s       R2:%5.2f %s",
            state.triggerL, tbar1, state.triggerR, tbar2);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        std::sprintf(line, "  D-PAD        %s                MAIN            %s", pDpadUp, pMainUp);
        PrintLine(line);

        std::sprintf(line, "            %s   %s                          %s   %s",
            pDpadLeft, pDpadRight, pMainLeft, pMainRight);
        PrintLine(line);

        std::sprintf(line, "               %s                                %s", pDpadDown, pMainDown);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        std::sprintf(line, " Shoulder: %s %s                                       %s %s",
            pBtnL1, pBtnL2, pBtnR2, pBtnR1);
        PrintLine(line);

        std::sprintf(line, " Stick   : %s                                             %s",
            pBtnL3, pBtnR3);
        PrintLine(line);

        std::sprintf(line, " System  : %s                                     %s",
            pBtnSelect, pBtnStart);
        PrintLine(line);

        std::sprintf(line, " Extra   : %s %s", pBtnExtra1, pBtnExtra2);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        char event[80] = " Event:";
        if (GameController::IsTrigger_ButtonDown())  std::strcat(event, " B+");
        if (GameController::IsTrigger_ButtonRight()) std::strcat(event, " A+");
        if (GameController::IsTrigger_ButtonLeft())  std::strcat(event, " Y+");
        if (GameController::IsTrigger_ButtonUp())    std::strcat(event, " X+");
        if (GameController::IsTrigger_L1())          std::strcat(event, " L1+");
        if (GameController::IsTrigger_R1())          std::strcat(event, " R1+");
        if (GameController::IsTrigger_L2())          std::strcat(event, " L2+");
        if (GameController::IsTrigger_R2())          std::strcat(event, " R2+");
        if (GameController::IsTrigger_L3())          std::strcat(event, " L3+");
        if (GameController::IsTrigger_R3())          std::strcat(event, " R3+");
        if (GameController::IsTrigger_Start())       std::strcat(event, " STA+");
        if (GameController::IsTrigger_Select())      std::strcat(event, " SEL+");
        if (GameController::IsTrigger_DpadUp())      std::strcat(event, " U+");
        if (GameController::IsTrigger_DpadDown())    std::strcat(event, " D+");
        if (GameController::IsTrigger_DpadLeft())    std::strcat(event, " L+");
        if (GameController::IsTrigger_DpadRight())   std::strcat(event, " R+");
        if (GameController::IsRelease_ButtonDown())  std::strcat(event, " B-");
        if (GameController::IsRelease_ButtonRight()) std::strcat(event, " A-");
        if (GameController::IsRelease_ButtonLeft())  std::strcat(event, " Y-");
        if (GameController::IsRelease_ButtonUp())    std::strcat(event, " X-");
        if (GameController::IsRelease_L1())          std::strcat(event, " L1-");
        if (GameController::IsRelease_R1())          std::strcat(event, " R1-");
        if (GameController::IsRelease_L2())          std::strcat(event, " L2-");
        if (GameController::IsRelease_R2())          std::strcat(event, " R2-");
        if (GameController::IsRelease_L3())          std::strcat(event, " L3-");
        if (GameController::IsRelease_R3())          std::strcat(event, " R3-");
        if (GameController::IsRelease_Start())       std::strcat(event, " STA-");
        if (GameController::IsRelease_Select())      std::strcat(event, " SEL-");
        if (GameController::IsRelease_DpadUp())      std::strcat(event, " U-");
        if (GameController::IsRelease_DpadDown())    std::strcat(event, " D-");
        if (GameController::IsRelease_DpadLeft())    std::strcat(event, " L-");
        if (GameController::IsRelease_DpadRight())   std::strcat(event, " R-");
        PrintLine(event);

        PrintLine("===============================================================================");
        PrintLine(" ESC to exit");

        Sleep(16);
    }

    // 終了処理
    GameController::Finalize();

    // カーソル再表示
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    return 0;
}
