#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <string>
#include "locale.h"
#include <sys/types.h>
#include <unistd.h>
using namespace std;

const int M = 7;
const int N = 563756 / 2 - 22;

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

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    WAVHEADER header;
    int j = 0;
    int pid = 0;
    int global_res1 = 0;
    int global_res2 = 0;
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

        for (int i = 1; i <= M; i++)
        {
            if (fork())
            {
                pid = i;
                break;
            }
        }

        if (pid != 0)
        {
            int res1 = 0;
            int res2 = 0;
            int from = M - (pid * M / N);
            int to = M - ((pid + 1) * M / N);
            for (int i = from; i > to; i--)
            {
                if (abs(mass[i]) > 16000)
                    res1++;
                if (abs(mass[i]) < 16000)
                    res2++;
            }
            char fileName[4096];
            sprintf(fileName, "%d.txt", pid);
            FILE* new_file = fopen(fileName, "w");
            fprintf(new_file, "%d\n%d\n", res1, res2);
            fclose(new_file);
            return 0;
        }
        else {
            for (int i = 1; i <= M; i++)
            {
                char fileName[4096];
                int res1;
                int res2;
                sprintf(fileName, "%d.txt", i);
                FILE* new_file = fopen(fileName, "r");
                fscanf(new_file, "%d\n%d\n", &res1, &res2);
                fclose(new_file);
                global_res1 = global_res1 + res1;
                global_res2 = global_res2 + res2;
            }
            cout << "К-во чисел, модуль которых больше 16000: " << global_res1 << endl;
            cout << "К-во чисел, модуль которых меньше 16000: " << global_res2 << endl;
            return 0;
        }
    }
    else
    {
        cout << "Файл не подходит" << endl;
    }
    delete mass;
    return 0;
}