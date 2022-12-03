#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <iostream>
#include <string>
#include "locale.h"
#include <sys/types.h>
#include <stdio.h>
#include "mpi.h"
#include <stdio.h>

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

struct INTERVAL
{
    int from;
    int to;
};

struct RESULT
{
    int res1;
    int res2;
};

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    int ProcNum, ProcRank, RecvRank;
    INTERVAL interval, get_int;
    RESULT result, get_res;

    MPI_Status Status;
    MPI_Init(&argc, &argv); // Инициализация среды выполнения MPI-программы
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum); // Определение количества процессов в выполняемой параллельной программе
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank); // Определение ранга процесса

    WAVHEADER header;
    int j = 0;
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

        if (ProcRank == 0)
        {
            for (int i = 1; i < ProcNum; i++)
            {
                int from = M - (i * M / N);
                int to = M - ((i + 1) * M / N);
                interval.from = from;
                interval.to = to;
                MPI_Send(&interval, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
            for (int i = 1; i < ProcNum; i++)
            {
                MPI_Recv(&get_res, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
                global_res1 = global_res1 + get_res.res1;
                global_res2 = global_res2 + get_res.res2;
                printf("The count of numbers whose modulus is greater than 16000: %d\n", global_res1);
                printf("The count of numbers whose modulus is less  than 16000: %d\n", global_res2);
            }
        }
        else
        {
            MPI_Recv(&get_int, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
            result.res1 = 0;
            result.res2 = 0;
            for (int j = get_int.from; j > get_int.to; j--)
            {
                if (abs(mass[j]) > 16000)
                    result.res1++;
                if (abs(mass[j]) < 16000)
                    result.res2++;
            }
            MPI_Send(&result, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        printf("Invalid file\n");
    }
    delete mass;
    MPI_Finalize();
    return 0;
}