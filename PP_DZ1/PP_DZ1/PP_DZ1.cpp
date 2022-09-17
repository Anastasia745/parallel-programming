#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <string>
#include "locale.h"

using namespace std;

const int n = 16000;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    int k1 = 0;
    int k2 = 0;
    int j = 0;
    FILE* file;
    ifstream in("C:\\Users\\Анастасия\\source\\repos\\PPLab1\\file.wav", ifstream::ate | ifstream::binary);
    int size = in.tellg() / 2;
    short* mass = new short[size];

    errno_t err;
    err = fopen_s(&file, "C:\\Users\\Анастасия\\source\\repos\\PPLab1\\file.wav", "rb");
    if (err)
    {
        printf_s("Failed open file, error %d", err);
        return 0;
    }

    short a;
    while ((fread(&a, 1, 2, file)) > 0)
    {
        mass[j] = ((a & 0xff) << 8) | ((a & 0xff00) >> 8);
        j++;

    }

    for (int i = 0; i < size; i++)
    {
        if (abs(mass[i]) > n)
            k1++;
        if (abs(mass[i]) < n)
            k2++;
    }
    cout << "К-во чисел, модуль которых больше 16000: " << k1 << endl;;
    cout << "К-во чисел, модуль которых меньше 16000: " << k2;
    return 0;
}