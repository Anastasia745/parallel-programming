#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include "locale.h"
using namespace std;

// Структура заголовка файла .wav
struct WAVHEADER
{
    char chunkId[4];
    unsigned long chunkSize;
    char format[4];
    char subchunk1Id[4];
    unsigned long subchunk1Size;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned long sampleRate;
    unsigned long byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char subchunk2Id[4];
    unsigned long subchunk2Size;
};

const int n = 16000;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    WAVHEADER header;
    int k1 = 0;
    int k2 = 0;
    int j = 0;
    FILE* file;
    ifstream in("../file.wav", ifstream::ate | ifstream::binary);
    int size = in.tellg() / 2 - 22;
    short* mass = new short[size];

    errno_t err;
    err = fopen_s(&file, "../file.wav", "rb");
    if (err)
    {
        printf_s("Failed open file, error %d", err);
        return 0;
    }

    fread_s(&header, sizeof(WAVHEADER), sizeof(WAVHEADER), 1, file);

    // Проверка, что точность звучания 16 бит
    if (header.bitsPerSample == 16)
    {
        short a;
        while ((fread(&a, 1, 2, file)) > 0)
        {
            mass[j] = ((a & 0xff) << 8) | ((a & 0xff00) >> 8);
            j++;
        };
        for (int i = 0; i < size; i++)
        {
            if (abs(mass[i]) > n)
                k1++;
            if (abs(mass[i]) < n)
                k2++;
        }
        cout << "К-во чисел, модуль которых больше 16000: " << k1 << endl;;
        cout << "К-во чисел, модуль которых меньше 16000: " << k2;
    }
    else
    {
        cout << "Файл не подходит" << endl;
    }

    delete mass;
    return 0;
}