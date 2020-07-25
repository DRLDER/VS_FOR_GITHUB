#pragma warning(disable:4996)
//��ֹһЩ�����Ϊ���ش���
#define DEBUG_STRING

//���ȵ���Windows.h,����TlHelp32�����
#include "Windows.h"
#include "TlHelp32.h"
#include "Psapi.h"

//����ʱʹ��
#ifdef DEBUG_STRING
#include "stdio.h"
#endif // DEBUG_STRING



///////////////////////////////////////////////////////////////////////////////////////////////////

//������ʾ
void IfError(bool state, const char* text = nullptr)
{
	//�������ֱ�ӷ���
	if (state)return;

	//״̬�쳣ִ�����´���
	char buffer[1024];
	//ʹ��0�����һƬ�ڴ�����
	ZeroMemory(buffer, 1024);
	//���������ʾ�ı���䵽buffer�в��������Ļ
	wsprintf(buffer, "ERROR:%s", text);

}

//������ʾ��������
void warning(const char* text, bool state = false)
{
	if (state) return;

	char buffer[5000];
	ZeroMemory(buffer, 5000);
	wsprintf(buffer, "���� : %s", text);

#ifdef DEBUG_STRING
	printf("%s \n", buffer);
#endif // DEBUG_STRING

	MessageBox(nullptr, buffer, nullptr, MB_OK);
}

///////////////////////////////////////////////////////////////////////////////////////////////////


//����ģ����Ϣ�ṹ��
struct module_information
{
	//����ģ�������Ϣ����
	HANDLE module_handle;
	char module_name[1024];
	char* module_data;
	int module_address;
	int module_size;

	//�����ڴ溯��
	void alloc(int size)
	{
		//��ʼģ���С
		module_size = size;

		//�����ڴ棬����ɹ����׵�ַ����
		module_data = (char*)VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		//�������ֵ��������ʾ����ʧ��
		IfError(module_data, "Allocated failed!\n");
	}

	//�ͷ��ڴ溯��
	void release()
	{
		//������ڷ�����ڴ��׵�ַ�����ͷŸ�������ΪMEM_RELEASE�ͷ�Ƭ�����ͷſռ�Ϊ0
		if (module_data) VirtualFree(module_data, 0, MEM_RELEASE);
		//ָ���ÿ�
		module_data = nullptr;
	}

};
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ����ID
DWORD GetprocessID(const char* process_name)
{
	//���ȣ���ȡ�Ľ���ID��ҪCreateToolhelp32Snapshot���Խ���ʹ�õ�ģ�飬�ѣ��߳�ִ�п���
	HANDLE SNAP = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

	//�ж��Ƿ��ĵ����գ���ȷ������������ʾ
	IfError(SNAP != INVALID_HANDLE_VALUE, "CreateToolhelp32Snapshot crashed!\n");

	//����PROCESSENTRY32���ͣ�������������Ϣ�ṹ��
	PROCESSENTRY32 ProcessInfo;

	//�ÿ��ڴ�����
	ZeroMemory(&ProcessInfo,sizeof(ProcessInfo));

	//��ʼ������ProcessInfo�Ľṹ���С������ᱨ��
	ProcessInfo.dwSize = sizeof(ProcessInfo);

	//�������Ŀ��������Ĵ洢��
	char target[1024];

	//��0
	ZeroMemory(target, 1024);

	//��Ŀ�����������洢��
	strncpy(target, process_name,strlen(process_name));

	//����������д
	_strupr(target);

	//��ȡ��һ��������Ϣ,������ProcessInfo
	BOOL state = Process32First(SNAP,&ProcessInfo);

	//ƥ�������
	while (state)
	{
		//�б�������Ƿ���ͬ
		if (strncmp(_strupr(ProcessInfo.szExeFile), target, strlen(target)) == 0)
		{
#ifdef DEBUG_STRING
			printf("ProcessName : %s \n", ProcessInfo.szExeFile);
			printf("ProcessID : %d \n", ProcessInfo.th32ProcessID);
			printf("\n");
#endif // DEBUG_STRING

			//��ȡ����Ӧ�Ľ�����Ϣ������ID..�����Ǿͽ������
			CloseHandle(SNAP);

			//���ؽ���ID
			return ProcessInfo.th32ProcessID;

		}
		//��ת����һ�����̣�����ƥ��
		state = Process32Next(SNAP, &ProcessInfo);
	}
	//�رս��̾��
	CloseHandle(SNAP);
	warning("!!!Unable to find processID!!!\n");
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ���̾��
HANDLE GetprocessHandle(DWORD ProcessID, DWORD Access = PROCESS_ALL_ACCESS)
{
	//��ȡ���
	HANDLE ProcessHandle = OpenProcess(Access,FALSE,ProcessID);
	IfError(ProcessHandle,"Open process handle failed!\n");

#ifdef DEBUG_STRING
	printf("ProcessID : %8x \n", (unsigned int)ProcessID);
	printf("ProcessHandle : %8x \n", (unsigned int)ProcessHandle);
	printf("\n");
#endif // DEBUG_STRING

	return ProcessHandle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ�ڴ�
DWORD read_memory(HANDLE process, int address, void* recv, int size)
{
	DWORD read_size;
	ReadProcessMemory(process, (LPCVOID)address, recv, size, &read_size);
	return read_size;
}

//д���ڴ�
DWORD write_memory(HANDLE process, int address, void* data, int size)
{
	DWORD write_size;
	WriteProcessMemory(process, (LPVOID)address, data, size, &write_size);
	return write_size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////


//��ȡģ����Ϣ
void GetModuleInfo(HANDLE ProcessHandle, DWORD ProcessID, const char* ModuleName, struct module_information& Info)
{
	//����ģ����գ�����̿�������
	HANDLE SNAP = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
	
	//���
	IfError(SNAP!= INVALID_HANDLE_VALUE,"CreateToolhelp32Snapshot crashed!\n");

	//����ģ����Ϣ�洢��
	MODULEENTRY32 ModuleInfo;

	//���һ���ڴ�����
	ZeroMemory(&ModuleInfo, sizeof(ModuleInfo));

	//����ģ���С��ʼֵ�����򱨴�
	ModuleInfo.dwSize = sizeof(ModuleInfo);

	//����Ŀ��ģ��洢���ռ�
	//����Ĳ�������ȡ����ID
	char target[1024];

	ZeroMemory(target,1024);

	strncpy(target, ModuleName, strlen(ModuleName));

	_strupr(target);

	BOOL state = Module32First(SNAP, &ModuleInfo);

	while(state)
	{
		if (strncmp(_strupr(ModuleInfo.szModule), target, strlen(target) )== 0)
		{
			//����ģ���ַ��ֵַ
			Info.module_address = (int)ModuleInfo.modBaseAddr;

			//����ģ����
			Info.module_handle = ModuleInfo.hModule;

			//�����ڴ�ռ�
			Info.alloc(ModuleInfo.modBaseSize);

			//�ÿ�ģ��������
			ZeroMemory(Info.module_name, sizeof(Info.module_name));

			//����ģ����
			strncpy(Info.module_name,ModuleInfo.szModule,strlen(ModuleInfo.szModule));

			DWORD Size = read_memory(ProcessHandle,Info.module_address,Info.module_data,Info.module_size);

			IfError(Size, "read memory error! @N1\n");

#ifdef DEBUG_STRING
			printf("ModuleName : %s \n", ModuleInfo.szModule);
			printf("ModuleAddress : %8x \n", (unsigned int)ModuleInfo.modBaseAddr);
			printf("ModuleSize : %8x \n", ModuleInfo.modBaseSize);
			printf("RealReadSize : %8x \n", Size);
			printf("\n");
#endif // DEBUG_STRING

			CloseHandle(SNAP);
			return;
		}
		state = Module32Next(SNAP,&ModuleInfo);
	}
	CloseHandle(SNAP);
	warning("!!!Unable to find Module!!!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
