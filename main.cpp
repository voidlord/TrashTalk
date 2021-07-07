#include <iostream>
#include <Windows.h>

void pressKey(INPUT ip, WORD vk) {
    UINT uSent;
    // Press Shift key
    ip.ki.wVk = vk;
    ip.ki.dwFlags = 0;
    
    uSent = SendInput(1, &ip, sizeof(INPUT));
    if (uSent != 1) {
        wprintf(L"SendInput failed: 0x%x\n", HRESULT_FROM_WIN32(GetLastError()));
    }
}
void releaseKey(INPUT ip, WORD vk) {
    UINT uSent;
    // Press Shift key
    ip.ki.wVk = vk;
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    
    uSent = SendInput(1, &ip, sizeof(INPUT));
    if (uSent != 1) {
        wprintf(L"SendInput failed: 0x%x\n", HRESULT_FROM_WIN32(GetLastError()));
    }
}
void redirectButton(PKBDLLHOOKSTRUCT p) {
    INPUT ip;
    // Generic keyboard event
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0;
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;

    pressKey(ip, VK_LSHIFT);
    pressKey(ip, p->vkCode);    // Press Intended key
    releaseKey(ip, p->vkCode);  // Release Intended key
    releaseKey(ip, VK_LSHIFT);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static BOOL fWork = TRUE;    // Enable/Disable program
    static BOOL fRedirect = FALSE;
    BOOL fEatKeystroke = FALSE;     // Flag
    static BOOL fLock = FALSE;

    // Keyboard combination flags
    static bool ctrlDown = false;
    static bool altDown = false;

    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        switch (wParam) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:

            // Command key press
            switch (p->vkCode) {
            case VK_LCONTROL:
                if (!ctrlDown) {
                    ctrlDown = true;
                }
                break;
            case VK_LMENU:
                if (!altDown) {
                    altDown = true;
                }
                break;
            case 'S':
                if (ctrlDown && altDown) {
                    // CTRL+ALT+S
                    fWork = !fWork;
                    if (fWork) {
                        printf("Enabled\n");
                    } else {
                        printf("Disabled\n");
                    }
                }
                break;
            }

            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:

            // Command key release
            switch (p->vkCode) {
            case VK_LCONTROL:
                ctrlDown = false;
                break;
            case VK_LMENU:
                altDown = false;
                break;
            }

            // Normal key release
            if (fWork && (p->vkCode >= 0x41 && p->vkCode <= 0x5A)) {

                // Redirect input only if not locked
                if (fRedirect && (fLock == FALSE)) {
                    fLock = TRUE;
                    redirectButton(p);
                    fLock = FALSE;

                    fEatKeystroke = TRUE;
                }
                
                fRedirect = !fRedirect;
                break;
            }

            break;
        }
    }

    // If redirected key, eat hook
    return fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);
    printf("Press CTRL+ALT+S to disable/enable\n");

    MSG msg;
    // Message loop to keep hook alive
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hhkLowLevelKybd);

    return 0;
}