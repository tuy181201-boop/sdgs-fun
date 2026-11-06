#include <conio.h>
int main(void) {
    int k = getch();
    if (k == 0 || k == 0xE0) { k = getch(); return k; }
    return k;
}