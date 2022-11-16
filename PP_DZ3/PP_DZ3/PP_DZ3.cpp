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

const int M = 7;
const int N = 563756 / 2 - 22;

HANDLE th[M];

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


int main(int argc, char** argv)
{
	WAVHEADER header;
	int j = 0;
	FILE* file;
	ifstream in("../file.wav", ifstream::ate | ifstream::binary);
	short* mass = new short[N];

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

	if (argc == 1)
	{
		for (int i = 0;i < M;i++)
		{
			STARTUPINFOA si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			si.wShowWindow = SW_HIDE;
			ZeroMemory(&pi, sizeof(pi));

			int from = i * N / M;
			int to = (i + 1) * N / M;

			char cmd[4096];
			sprintf(cmd, "\"%s\" %d %d", argv[0], from, to, i);

			if (!(CreateProcessA(NULL, cmd, NULL, NULL, false, 0, NULL, NULL, &si, &pi)))
				printf("Error 1");
			th[i] = pi.hProcess;
		}

		WaitForMultipleObjects(M, th, false, INFINITE);

		int global_ans1 = 0;
		int global_ans2 = 0;
		for (int i = 0;i < M;i++)
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
		for (int i = from;i < to;i++)
		{
			if (mass[i] > 16000)
				ans1++;
			else
				ans2++;
		}
		char filename[4096];
		sprintf(filename, "%d.txt", number);
		FILE* f = fopen(filename, "w");
		fprintf(f, "%d\n%d\n", ans1, ans2);
		fclose(f);

	}
}
