#pragma once
#include "Windows.h"

//��������
void check_error(bool state, const char* str = nullptr)
{
	if (state)return;

	//�趨һ���ռ䣬�Ŵ�����ʾ
	char buffer[1024*2];
	wsprintf(buffer,"ERROR:%s",str );

	//����һ������
	MessageBox(nullptr, buffer, nullptr, MB_OK | MB_ICONHAND);
	exit(-1);
}