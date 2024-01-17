#include <Windows.h>  
#include <iostream>

int main() {

    setlocale(LC_ALL, "Russian");

    CURSORINFO ci{};
    POINT cp;

    ci.cbSize = sizeof(ci);

    bool detected = false;

    while (!detected)
    {
        if (!GetCursorInfo(&ci)) {
            std::cout << "Ошибка GetCursorInfo" << std::endl;
            return 1;
        }


        if (!GetCursorPos(&cp)) {
            std::cout << "Ошибка GetCursorPos" << std::endl;
            return 1;
        }

        if (ci.ptScreenPos.x != cp.x || ci.ptScreenPos.y != cp.y) {
            std::cout << "Обнаружено несоответствие координат курсора!";
            std::cout << " Возможна эмуляция ввода." << std::endl;
            detected = true;
        }
    }

    return 0;
}