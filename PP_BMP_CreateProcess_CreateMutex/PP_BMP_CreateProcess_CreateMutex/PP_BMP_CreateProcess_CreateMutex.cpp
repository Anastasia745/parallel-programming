// Гнибеда Анастасия Юрьевна, М22-514: X = 514 + 4 = 518
// 518 % 4 = 2 -> Способ C
// N = 4 + (518 % 5) = 7

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include <string>
#include "locale.h"
#include<Windows.h>
#include<cstdio>
#include <thread>
using namespace std;

struct RGB
{
    uint8_t r, g, b, a;
};

RGB* getPixel(uint32_t x, uint32_t y, BITMAPFILEHEADER bitMapFileHeader, BITMAPINFOHEADER bitMapInfoHeader, uint8_t* pixels)
{
    RGB* rgb = reinterpret_cast<RGB*>(pixels);
    return &rgb[(bitMapInfoHeader.biHeight - 1 - y) * bitMapInfoHeader.biWidth + x];
}

const int W = 640;
const int H = 462;
const int N = 7;

int global_res1_th = 0;
int global_res2_th = 0;

HANDLE proc[N];
HANDLE mutex;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    BITMAPFILEHEADER bitMapFileHeader;
    BITMAPINFOHEADER bitMapInfoHeader;

    int j = 0;
    FILE* file;
    ifstream in("../file.bmp", ifstream::ate | ifstream::binary);

    errno_t err;
    err = fopen_s(&file, "../file.bmp", "rb");
    if (err)
    {
        printf_s("Failed open file, error %d", err);
        return 0;
    }
    fread_s(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), sizeof(BITMAPFILEHEADER), 1, file);
    fread_s(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), sizeof(BITMAPINFOHEADER), 1, file);
    // printf_s("Размер файла %d x %d пикселей", bitMapInfoHeader.biWidth, bitMapInfoHeader.biHeight);
    in.seekg(bitMapFileHeader.bfOffBits, std::ios::beg);
    uint8_t* pixels = new uint8_t[bitMapFileHeader.bfSize - bitMapFileHeader.bfOffBits];
    in.read(reinterpret_cast<char*>(&pixels[0]), bitMapFileHeader.bfSize - bitMapFileHeader.bfOffBits);
    uint8_t* temp = new uint8_t[bitMapInfoHeader.biWidth * bitMapInfoHeader.biHeight * sizeof(RGB)];
    uint8_t* inp = pixels;
    RGB* rgb = reinterpret_cast<RGB*>(temp);
    int padding = (bitMapInfoHeader.biSizeImage - bitMapInfoHeader.biWidth * bitMapInfoHeader.biHeight * 3) / bitMapInfoHeader.biHeight;

    for (int i = 0; i < bitMapInfoHeader.biHeight; ++i, inp += padding)
    {
        for (int j = 0; j < bitMapInfoHeader.biWidth; ++j)
        {
            rgb->b = *(inp++);
            rgb->g = *(inp++);
            rgb->r = *(inp++);
            rgb->a = 0xFF;
            ++rgb;
        }
    }
    delete[] pixels;
    pixels = temp;

    uint8_t** mass = (uint8_t**)malloc(bitMapInfoHeader.biWidth * sizeof(uint8_t*));
    for (int i = 0; i < bitMapInfoHeader.biWidth; i++)
        mass[i] = (uint8_t*)malloc(bitMapInfoHeader.biHeight * sizeof(uint8_t));

    for (int i = 0; i < bitMapInfoHeader.biWidth; ++i)
    {
        for (int j = 0; j < bitMapInfoHeader.biHeight; j++)
        {
            mass[i][j] = getPixel(i, j, bitMapFileHeader, bitMapInfoHeader, pixels)->r;
        }
    }

    // mutex
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (!mutex) 
        printf("Error");

    if (argc == 1)
    {
        for (int i = 0;i < N;i++)
        {
            STARTUPINFOA si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            si.wShowWindow = SW_HIDE;
            ZeroMemory(&pi, sizeof(pi));
            
            int from = i * (W * H) / N;
            int to = (i + 1) * (W * H) / N;

            char cmd[4096];
            sprintf(cmd, "\"%s\" %d %d %d", argv[0], from, to, i);
            if (!CreateProcessA(NULL, cmd, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) 
                printf("Error 1");
            proc[i] = pi.hProcess;
        }
        WaitForMultipleObjects(N, proc, true, INFINITE);
        CloseHandle(mutex);

        int global_ans1 = 0;
        int global_ans2 = 0;
        for (int i = 0;i < N;i++)
        {
            char filename[4096];
            sprintf(filename, "%d.txt", i);
            int ans1, ans2;
            FILE* f = fopen(filename, "r");
            fscanf(f, "%d\n%d\n", &ans1, &ans2);
            global_ans1 += ans1;
            global_ans2 += ans2;
            fclose(f);
        }
    }
    else
    {
        int from = atoi(argv[1]);
        int to = atoi(argv[2]);
        int number = atoi(argv[3]);
        int ans1 = 0;
        int ans2 = 0;
        for (int i = from; i > to; i--)
        {
            for (int j = 0; j < H; j++)
            {
                if (mass[i][j] > 128)
                    ans1++;
                else
                    ans2++;
            }
        }

        global_res1_th += ans1;
        global_res2_th += ans2;

        char filename[4096];
        sprintf(filename, "%d.txt", number);
        FILE* f = fopen(filename, "w");
        DWORD result = WaitForSingleObject(mutex, INFINITE);
        if (result == WAIT_OBJECT_0)
        {
            fprintf(f, "%d\n%d\n", ans1, ans2);
            fclose(f);
            ReleaseMutex(mutex);
        }
        CloseHandle(proc[number]);
    }
}