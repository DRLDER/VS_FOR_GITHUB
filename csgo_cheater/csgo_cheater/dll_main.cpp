#include "do_cheats.hpp"
#include "process.h"


int _stdcall DllMain(void* _DllHandle, unsigned long _Reason, void* _Reserved)
{
	//������Ҫ�Ǽ���initial_d3d9,������һ���߳�,����dllִ��ʱ����
	//_beginthreadex��Ϊ�����߳�����
	if (_Reason == DLL_PROCESS_ATTACH) _beginthreadex(nullptr, 0,initial_d3d9,nullptr,0,nullptr);
	
	
	//��������ж��DLL����un_load������ʵ�ֵ�ַ��תԭ��ַ��ֹ�������
	if (_Reason == DLL_PROCESS_DETACH) un_load();


	return 1;

}