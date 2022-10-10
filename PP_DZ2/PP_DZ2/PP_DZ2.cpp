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
using namespace std;

CRITICAL_SECTION cs;

const int M = 563756 / 2 - 22;
const int N = 7;

struct task
{
    int from;
    int to;
    int number;
    short wav_numbers[M];
};

int global_res1 = 0;
int global_res2 = 0;

DWORD __stdcall f(void* arg)
{
    task* t = (task*)arg;
    int res1 = 0;
    int res2 = 0;
    for (int i = t->from; i > t->to; i--)
    {
        if (t->wav_numbers[i] > 16000)
            res1++;
        if (t->wav_numbers[i] < 16000)
            res2++;
    }
    EnterCriticalSection(&cs);
    global_res1 += res1;
    global_res2 += res2;
    printf("Thread %d:\n   >16000: %d   <16000: %d\n", t->number, res1, res2);
    LeaveCriticalSection(&cs);
    return 0;
}

HANDLE th[N];
DWORD tid[N];
task t[N];

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

int main()
{
    setlocale(LC_ALL, "Russian");
    WAVHEADER header;
    int j = 0;
    FILE* file;
    ifstream in("../file.wav", ifstream::ate | ifstream::binary);
    short* mass = new short[M];

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
    }
    else
    {
        printf_s("Файл не подходит\n");
    }

    // Потоки
    InitializeCriticalSection(&cs);
    for (int i = 0;i < N;i++)
    {
        short* wav_numbers = new short[M];
        t[i].from = M - (i * M / N);
        t[i].to = M - ((i + 1) * M / N);
        t[i].number = i;

        for (int j = 0; j < M; j++)
        {
            t[i].wav_numbers[j] = mass[j];
        }

        th[i] = CreateThread(NULL, M, f, (void*)&t[i], 0, &tid[i]);
    }

    DWORD code;
    for (int i = 0;i < N;i++)
        WaitForSingleObject(th[i], INFINITE);
    WaitForMultipleObjects(N, th, true, INFINITE);
    printf("\nК-во чисел, >16000: %d\n", global_res1);
    printf("К-во чисел, <16000: %d\n", global_res2);
    DeleteCriticalSection(&cs);
}
