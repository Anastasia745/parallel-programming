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
    int r, g, b, a;
};

RGB* getPixelInfo(int i, int j, BITMAPFILEHEADER bitMapFileHeader, BITMAPINFOHEADER bitMapInfoHeader, int* pixels)
{
    RGB* rgb = reinterpret_cast<RGB*>(pixels);
    return &rgb[(bitMapInfoHeader.biHeight - 1 - j) * bitMapInfoHeader.biWidth + i];
}

const int W = 640;
const int H = 462;
const int N = 7;

HANDLE proc[N];
HANDLE semaphore;
int global_ans1 = 0;
int global_ans2 = 0;

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    BITMAPFILEHEADER bitMapFileHeader;
    BITMAPINFOHEADER bitMapInfoHeader;

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
    int widthBMP = bitMapInfoHeader.biWidth;
    int heightBMP = bitMapInfoHeader.biHeight;
    // printf_s("Размер файла %d x %d пикселей", widthBMP, heightBMP);
    in.seekg(bitMapFileHeader.bfOffBits, std::ios::beg);
    int* pixels = new int[bitMapFileHeader.bfSize - bitMapFileHeader.bfOffBits];
    in.read(reinterpret_cast<char*>(&pixels[0]), bitMapFileHeader.bfSize - bitMapFileHeader.bfOffBits);
    int* temp = new int[widthBMP * heightBMP * sizeof(RGB)];
    int* inp = pixels;
    RGB* rgb = reinterpret_cast<RGB*>(temp);
    int padding = (bitMapInfoHeader.biSizeImage - widthBMP * heightBMP * 3) / heightBMP;

    for (int i = 0; i < heightBMP; i++, inp += padding)
    {
        for (int j = 0; j < widthBMP; ++j)
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

    int** mass = (int**)malloc(widthBMP * sizeof(int*));
    for (int i = 0; i < widthBMP; i++)
        mass[i] = (int*)malloc(heightBMP * sizeof(int));

    for (int i = 0; i < widthBMP; ++i)
    {
        for (int j = 0; j < heightBMP; j++)
        {
            mass[i][j] = getPixelInfo(i, j, bitMapFileHeader, bitMapInfoHeader, pixels)->r;
        }
    }

    semaphore = CreateSemaphore(NULL, N, N, NULL);
    if (!semaphore)
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

            int from = W - (i * W / N);
            int to = W - ((i + 1) * W / N);

            char cmd[4096];
            sprintf(cmd, "\"%s\" %d %d %d", argv[0], from, to, i);
            if (!CreateProcessA(NULL, cmd, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
                printf("Error 1");
            proc[i] = pi.hProcess;
        }
        WaitForMultipleObjects(N, proc, true, INFINITE);
        CloseHandle(semaphore);

        for (int i = 1;i < N;i++)
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
        printf("\nК-во пикселей, >128: %d\n", global_ans1);
        printf("К-во пикселей, <128: %d\n", global_ans2);
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

        char filename[4096];
        sprintf(filename, "%d.txt", number);
        FILE* f = fopen(filename, "w");
        DWORD result = WaitForSingleObject(semaphore, 0L);
        if (result == WAIT_OBJECT_0)
        {
            fprintf(f, "%d\n%d\n", ans1, ans2);
            fclose(f);
            ReleaseSemaphore(semaphore, 1, NULL);
        }
        CloseHandle(proc[number]);
    }
    return 0;
}