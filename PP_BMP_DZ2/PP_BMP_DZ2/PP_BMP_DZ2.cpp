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
#include <mutex>
using namespace std;

std::mutex mtx;

const int M = 819200 / 2 - 27;
const int N = 7;

struct task
{
    int from;
    int to;
    int number;
    short bmp_numbers[M];
};

int global_res1_th = 0;
int global_res2_th = 0;

void threadFunc(void* arg)
{
    task* t = (task*)arg;
    mtx.lock();
    int res1 = 0;
    int res2 = 0;
    int from = t->from;
    int to = t->to;

    for (int j = from; j > to; j--)
    {
        if (abs(t->bmp_numbers[j]) >= 16000)
            res1++;
        else 
            res2++;
    }

    global_res1_th += res1;
    global_res2_th += res2;
    printf("Thread %d:\n   >16000: %d   <16000: %d\n", t->number, res1, res2);
    mtx.unlock();
}

task t[N];

int main()
{
    setlocale(LC_ALL, "Russian");
    BITMAPFILEHEADER bitMapFileHeader;
    BITMAPINFOHEADER bitMapInfoHeader;
    int j = 0;
    FILE* file;
    ifstream in("../file.bmp", ifstream::ate | ifstream::binary);
    short* mass = new short[M];

    errno_t err;
    err = fopen_s(&file, "../file.bmp", "rb");
    if (err)
    {
        printf_s("Failed open file, error %d", err);
        return 0;
    }

    fread_s(&bitMapFileHeader, sizeof(BITMAPFILEHEADER), sizeof(BITMAPFILEHEADER), 1, file);
    fread_s(&bitMapInfoHeader, sizeof(BITMAPINFOHEADER), sizeof(BITMAPINFOHEADER), 1, file);
    
    short a;
    while ((fread(&a, 1, 2, file)) > 0)
    {
        mass[j] = ((a & 0xff) << 8) | ((a & 0xff00) >> 8);
        j++;
    };
    
    printf("\n-- std::thread --\n\n");

    // std::thread
    for (int i = 0; i < N; i++)
    {        
        short* bmp_numbers = new short[M];
        t[i].from = M - (i * M / N);
        t[i].to = M - ((i + 1) * M / N);
        t[i].number = i;
        for (int j = 0; j < M; j++)
        {
            t[i].bmp_numbers[j] = mass[j];
        }
        std::thread th(threadFunc, (void*)&t[i]);
        th.join();
    }

    printf("\nК-во чисел, >16000: %d\n", global_res1_th);
    printf("К-во чисел, <16000: %d\n", global_res2_th);
}