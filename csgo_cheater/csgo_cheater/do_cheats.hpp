#pragma once
#include "help_func.hpp"
#include "inline_hook.hpp"
#include "d3d9.h"
#pragma comment(lib,"d3d9.lib")
#include "stdio.h"

//�������ƴ����õ�ͷ�ļ�
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include"imgui/imgui_impl_win32.h"

//�趨D3D��Ŀָ��
IDirect3D9* g_direct3d9 = nullptr;

//D3D�豸ָ��
IDirect3DDevice9* g_direct3ddevice9 = nullptr;

// �趨D3D��ǰ�������ڴ�����
D3DPRESENT_PARAMETERS g_present;

//�趨������������������inline_hookָ��
inline_hook* g_Reset_hook = nullptr;
inline_hook* g_EndScene_hook = nullptr;
inline_hook* g_DrawIndexedPrimitive_hook = nullptr;

//��������ԭʼ���ڹ���ָ��
WNDPROC g_original_proc = nullptr;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//�����ⲿ���ڵĺ������壬extern���߱���������ĺ��������ڱ𴦶��壬�ڴ�����
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT CALLBACK self_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM IParam)
{

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, IParam))
		return true;

	//�Ӵ��ڻ�ת������ʹ��CallWindowProcW
	return CallWindowProcW(g_original_proc, hWnd, uMsg, wParam, IParam);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//��ʼ������
void initialize_imgui(IDirect3DDevice9* direct3ddevice9)
{

	//��ʼ������
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//��������
	ImGui::StyleColorsDark();

	//��ʼ������
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//���ÿ�ָ�룬��ֹ��Ŀ¼�������ļ����������ļ��ȣ�
	io.LogFilename = nullptr;
	io.IniFilename = nullptr;

	//������ʲô���������ô˻��ƴ���
	ImGui_ImplWin32_Init(FindWindowW(L"Direct3DWindowClass", nullptr));
	ImGui_ImplDX9_Init(direct3ddevice9);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//������ǽٳֲ����ļ�������

//�����Լ������ú�����Դ��IDirect3DDevice9�е�reset����,����Ϸ���ڷ����仯ʱ���ǽ�������
HRESULT _stdcall self_Reset(IDirect3DDevice9* direct3ddevice9,D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	//printf("DEBUG:DLL in self_Reset!\n");

	//������ת�����ǵĴ�������Ҫ�Ƚ�ԭ��ת����ָ����˺�������ִ�У���Ϊ�����߳��У�����Ϸ�̲߳���ͻ
	g_Reset_hook->restore_address();
	
	//ʹimgui��Ч
	ImGui_ImplDX9_InvalidateDeviceObjects();

	//����Ϸ��Ļ�����ı�ʱ���˺��������õĺ���
	HRESULT result = direct3ddevice9->Reset(pPresentationParameters);

	//ʹimgui��Ч
	ImGui_ImplDX9_CreateDeviceObjects();

	//��������ָ�����ԭ��ת��ַ�������Ҫ���޸�Ϊ�Լ�����ת��ַ��������һִ��ԭresetʱ�������ǵ�reset
	g_Reset_hook->modify_address();

	//����ִ��״̬
	return result;

}



//��������������ķ���
HRESULT _stdcall self_EndScene(IDirect3DDevice9* direct3ddevice9)
{
	//�趨��̬ȫ�ֱ������������ļ�ʹ�ã������ڱ���ļ���ȡ��ͬ����������ͻ��
	static bool first_call = true;

	if (first_call)
	{
		first_call = false;
		//�����ǵ�d3d�豸�ϳ�ʼ��imgui
		initialize_imgui(direct3ddevice9);
		//����ԭʼ���ڹ��̣�ʹ��SetWindowLongA
		g_original_proc= (WNDPROC)SetWindowLongA(FindWindowW(L"Direct3DWindowClass", nullptr),GWL_WNDPROC,(LONG)self_proc);
	}

	//printf("DEBUG:DLL in self_EndScene!\n");

	//��ַ�ָ�
	g_EndScene_hook->restore_address();

	//���ÿ��
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//imgui���濪ʼ
	ImGui::Begin("new window");

	//��������
	ImGui::Text("this is test window");

	//imgui�������
	ImGui::End();

	//������ܹ���
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	//����Ϸ���ڻ���ʱ���˺��������õĺ���
	HRESULT result = direct3ddevice9->EndScene();
	
	//��ַת�����ǵĵ�ַ
	g_EndScene_hook->modify_address();

	//����ִ��״̬
	return result;
}



//����ģ��͸��
HRESULT _stdcall  self_DrawIndexedPrimitive(IDirect3DDevice9* direct3ddevice9, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{

	return 1;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




//��ʼ��d3d9
unsigned int _stdcall initial_d3d9(void* data)
{
//#ifdef _DEBUG
//	AllocConsole();
//	SetConsoleTitleA("test");
//	freopen("CON", "w", stdout);
//#endif 


	//��ʼ��D3D�豸�����ݸ�g_direct3d9
	g_direct3d9 =  Direct3DCreate9(D3D_SDK_VERSION);

	//������Ƿ��ʼ��ʧ��
	check_error(g_direct3d9,"Direct3DCreate9ʧ��");

	//��ʼ���ɹ�����ճ�ʼ���豸����
	memset(&g_present, 0, sizeof(g_present));

	//�ڳ�ʼ���������趨��Ӧ����
	g_present.Windowed = TRUE;
	g_present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_present.BackBufferFormat = D3DFMT_UNKNOWN;
	g_present.EnableAutoDepthStencil = TRUE;
	g_present.AutoDepthStencilFormat = D3DFMT_D16;

	//ǰ������ĳ�ʼ����ɣ��˴����������豸
	HRESULT result= g_direct3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, FindWindowW(L"Direct3DWindowClass", nullptr), 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_present,&g_direct3ddevice9);

	//��ⴴ���豸�Ƿ����
	check_error(result == 0, "CreateDevice����ʧ�ܣ�");

	//��ԭ�ȵ���Ŀָ��ת�ɱ�
	int* direct3d9_table = (int*)*(int*)g_direct3d9;

	//��ԭ�ȵ��豸ָ��ת�ɱ�
	int* direct3ddevice9_table = (int*)*(int*)g_direct3ddevice9;

	//�����������ӡ������ù��ӣ���Ϊ����Ϸ���ڱ䶯ʱִ�в����Ĺ���
	g_Reset_hook = new inline_hook(direct3ddevice9_table[16], (int)self_Reset);

	//���ƹ��ӣ�����Ϸ�ڻ��ƽ�ɫʱ���������Ĺ���
	g_EndScene_hook = new inline_hook(direct3ddevice9_table[42], (int)self_EndScene);

	//��ת�����ǵ�reset����
	g_Reset_hook->modify_address();

	//��ת�����ǵ�EndScene����
	g_EndScene_hook->modify_address();

	return 0;

}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






//����ж�غ���
void un_load()
{
	//�ָ�������Դ��ת��ַ
	g_Reset_hook->restore_address();
	g_EndScene_hook->restore_address();
}