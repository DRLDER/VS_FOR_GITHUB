#pragma once

#include "D3DDrawEngine.h"


///////////////////////////////////////////////////////////////////////////////////////////////////

//�������ȫ�ֱ���
DWORD g_ProcessId;
HANDLE g_ProcessHandle;

int g_MatrixAddr;
int g_AngleAddr;
int g_SelfAddr;
int g_PlayersAddr;

HWND g_game_hwnd;
HWND g_transparent_hwnd;

const int g_players_count = 32;
///////////////////////////////////////////////////////////////////////////////////////////////////

//�����м�ȫ�ֱ���
char ProcessName[] = "hl2.exe";


///////////////////////////////////////////////////////////////////////////////////////////////////

struct PlayerList
{
	bool effective;//�Ƿ���Ч
	int aimbot_len;//���鳤��
	bool self;//���Լ�
	float location[3];//����λ��
	float head_bone[3];//ͷ��λ��
	int camp;//��Ӫ
	int blood;//Ѫ��
};

///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����ص�ַ����ַ��ȡ��������Ҫ��̬������
void InitializeAddress(const char* process_name)
{
	//���Ⱦ��ǻ�ȡ����id
	DWORD ProcessID= GetprocessID(process_name);

	//��ȡ���̾��
	HANDLE ProcessHandle = GetprocessHandle(ProcessID);
	
	//ȫ�ֱ�����ֵ
	g_ProcessId = ProcessID;
	g_ProcessHandle = ProcessHandle;

	//����ģ����Ϣ��ȡ�ṹ�洢��
	struct module_information engine_module;
	struct module_information server_module;
	struct module_information client_module;
	//��ȡģ����Ϣ
	GetModuleInfo(ProcessHandle, ProcessID, "engine.dll", engine_module);
	GetModuleInfo(ProcessHandle, ProcessID, "client.dll", client_module);

	//��ȡ��ַ
	int matrix_address = engine_module.module_address + 0x5A78EC;//�Լ������ַ

	int angle_address = engine_module.module_address + 0x4791B4;//�Լ��ǶȻ�ַ

	int self_address=client_module.module_address+0x4D3904;//4D3914
	//read_memory(ProcessHandle, client_module.module_address + 0x4D3904,&self_address,sizeof(int));

	int players_address= client_module.module_address + 0x4D3904; //�����Լ�����ҵ�ַ
	

#ifdef DEBUG_STRING
	printf("SelfMatrix :%8x \n", matrix_address);
	printf("SelfAngle :%8x \n", angle_address);
	printf("SelfAddress :%8x \n", self_address);
	printf("PlayerAddress :%8x \n", players_address);
#endif // DEBUG_STRING

	//ȫ�ֱ�����ֵ
	g_MatrixAddr = matrix_address;
	g_AngleAddr = angle_address;
	g_SelfAddr = self_address;
	g_PlayersAddr = players_address;

}
///////////////////////////////////////////////////////////////////////////////////////////////////

void GetPlayerList(PlayerList* Players)
{
	//��ȡ���̾��
	HANDLE process = g_ProcessHandle;

	for (int i = 0; i < g_players_count; i++)
	{
		int PlayerBaseAddr;

		//��ȡ���ƫ�ƺ���Ϣ�ṹ
		read_memory(process, (g_PlayersAddr + i * 0x10), &PlayerBaseAddr, sizeof(int));

		//�����ַΪ�գ���������һ��ѭ��
		if (PlayerBaseAddr == 0)continue;

		//���ǻ�ȡѪ��
		read_memory(process, PlayerBaseAddr + 0x94, &Players[i].blood, sizeof(int));

		//���Ѫ��Ϊ�գ�����Ҳ����һ��ѭ��
		if (Players[i].blood <= 1)continue;

		//���������Ч���
		Players[i].effective = true;

		//��������ֱ�߾���
		Players[i].aimbot_len = 9999;
		
		//���ù������׵�ַ�洢��
		int bone_base_address;

		if (read_memory(process, (PlayerBaseAddr + 0x578), &bone_base_address, sizeof(int)))
		{
			//��ȡ������
			read_memory(process, (bone_base_address + 171 * sizeof(float)), &Players[i].head_bone[0], sizeof(float));
			read_memory(process, (bone_base_address + 175 * sizeof(float)), &Players[i].head_bone[1], sizeof(float));
			read_memory(process, (bone_base_address + 179 * sizeof(float)), &Players[i].head_bone[2], sizeof(float));		
		}
		
		//��ȡλ��
		read_memory(process, PlayerBaseAddr + 0x338, Players[i].location, sizeof(Players[i].location));
		
		//��ȡ��Ӫ
		read_memory(process, PlayerBaseAddr + 0x9C, &Players[i].camp, sizeof(int));
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ�Լ���λ��
void get_self_location(float* location)
{
	int location_base_address;
	read_memory(g_ProcessHandle, g_SelfAddr, &location_base_address, sizeof(int));
	if (location_base_address)
		read_memory(g_ProcessHandle, location_base_address + 0x338, location, sizeof(float) * 3);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

//ת��Ϊ������Ϣ
bool to_rect_info(float matrix[][4], float* location, int window_width, int window_heigt, int& x, int& y, int& w, int& h)
{
	float to_target = matrix[2][0] * location[0]
		+ matrix[2][1] * location[1]
		+ matrix[2][2] * location[2]
		+ matrix[2][3];
	if (to_target < 0.01f)
	{
		x = y = w = h = 0;
		return false;
	}
	to_target = 1.0f / to_target;

	float to_width = window_width + (matrix[0][0] * location[0]
		+ matrix[0][1] * location[1]
		+ matrix[0][2] * location[2]
		+ matrix[0][3]) * to_target * window_width;

	float to_height_h = window_heigt - (matrix[1][0] * location[0]
		+ matrix[1][1] * location[1]
		+ matrix[1][2] * (location[2] + 75.0f)
		+ matrix[1][3]) * to_target * window_heigt;

	float to_height_w = window_heigt - (matrix[1][0] * location[0]
		+ matrix[1][1] * location[1]
		+ matrix[1][2] * (location[2] - 5.0f)
		+ matrix[1][3]) * to_target * window_heigt;

	x = (int)(to_width - (to_height_w - to_height_h) / 4.0f);
	y = (int)(to_height_h);
	w = (int)((to_height_w - to_height_h) / 2.0f);
	h = (int)(to_height_w - to_height_h);
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////


//�����������
int get_aimbot_len(int window_w, int window_h, int x, int y)
{
	int temp_x = abs(window_w - x);
	int temp_y = abs(window_h - y);
	return (int)sqrt((temp_x * temp_x) + (temp_y * temp_y));
}

//����Ѫ��
void render_player_blood(float blood, int x, int y, int h)
{
	float value = blood / 100.0f * h;
	render_line(D3DCOLOR_XRGB(250, 0, 255), x, y, x, y + value);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

int GetSelfCamp(PlayerList* Players);

//�������﷽��
void render_player_box(PlayerList* Players)
{

	//�Զ���xy����ͷ��򳤿�
	int window_x, window_y, window_w, window_h;
	get_window_size(g_game_hwnd, window_x, window_y, window_w, window_h);
	window_w /= 2;
	window_h /= 2;

	float matrix[4][4];
	read_memory(g_ProcessHandle, g_MatrixAddr, matrix, sizeof(float) * 4 * 4);

	float self_location[3];
	get_self_location(self_location);

	int self_camp = GetSelfCamp(Players);

	for (int i = 0; i < g_players_count; i++)
	{
		int x, y, w, h;
		if (Players[i].effective && Players[i].blood > 1 && Players[i].self == false && to_rect_info(matrix, Players[i].location, window_w, window_h, x, y, w, h))
		{
			D3DCOLOR color = NULL;


			if (self_camp != Players[i].camp)
			{
				color = D3DCOLOR_XRGB(255, 137, 0);

				Players[i].aimbot_len = get_aimbot_len(window_w, window_h, x + (w / 2), y + (h / 2));

				render_rect(color, x, y, w, h);

				render_player_blood(Players[i].blood, x - 5, y, h);
			}

		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//�����Լ��ı���ֵ
void SetSelfCamp(PlayerList* Players)
{
	Players[0].self = true;
}

//��ȡ�Լ�����Ӫ
int GetSelfCamp(PlayerList* Players)
{
	return Players[0].camp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ���������²���
bool get_mouse_left_down()
{
	return GetAsyncKeyState(VK_LBUTTON) & 0x8000;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ����ĵ���
int GetAimIndexHeadLocation(PlayerList* Players)
{
	//��ʼ������ֵ
	int index = -1;
	
	//��ʼ����Ӫ����ȡ�Լ�����Ӫ
	int camp = GetSelfCamp(Players);

	//�����������
	for (int i = 0; i < g_players_count; i++) 
	{
		//��������Ч������ӪΪ������Ӫ
		if (Players[i].effective && camp != Players[i].camp) 
		{
			//��ȡ����ֵ
			if (index == -1)index = i;

			//���������׼�߳��ȱ��ֻ�ȡĿ�껹Ҫ����ô��������ֵ
			else if (Players[index].aimbot_len > Players[i].aimbot_len)index = i;

		}

	}

	return index;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//��ȡ��ǰ�Ƕ�
void get_current_angle(float* angle)
{
	//��ȡ��ǰ�ĽǶ�
	read_memory(g_ProcessHandle, g_AngleAddr, angle, sizeof(float) * 2);
	
}

//��ȡ����Ƕ�
void get_aimbot_angle(float* self_location, float* player_location, float* aim_angle)
{
	float x = self_location[0] - player_location[0];
	float y = self_location[1] - player_location[1];
	float z = self_location[2] - player_location[2] + 65.0f;

	const float pi = 3.1415f;
	aim_angle[0] = (float)atan(z / sqrt(x * x + y * y)) / pi * 180.f;
	aim_angle[1] = (float)atan(y / x);

	if (x >= 0.0f && y >= 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.0f - 180.0f;
	else if (x < 0.0f && y >= 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.0f;
	else if (x < 0.0f && y < 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.0f;
	else if (x >= 0.0f && y < 0.0f) aim_angle[1] = aim_angle[1] / pi * 180.f + 180.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////


//�������
void AimbotPlayers(PlayerList* Players, float max_fov = 50)
{
	//���Ȼ�ȡ�Լ���λ��
	float self_location[3];
	get_self_location(self_location);

	//��ȡ�������,������ӽ�����
	int AimIndex = GetAimIndexHeadLocation(Players);

	//û�ҵ������
	if (AimIndex == -1) return;

	//��ȡ��ǰ�ĽǶ�
	float current_angle[2];
	get_current_angle(current_angle);

	//printf("current_angle:%f,%f\n", current_angle[0], current_angle[1]);

	//��������Ƕ�
	float aim_angle[2];
	
	//printf("Players[AimIndex].head_bone:%f,%f,%f\n", Players[AimIndex].head_bone[0], Players[AimIndex].head_bone[1], Players[AimIndex].head_bone[2]);

	get_aimbot_angle(self_location, Players[AimIndex].head_bone, aim_angle);

	//printf("aim_angle:%f,%f\n",aim_angle[0], aim_angle[1]);

	if (abs((int)aim_angle[0] - (int)current_angle[0]) > max_fov
		|| abs((int)aim_angle[1] - (int)current_angle[1]) > max_fov)
		return;

	//д������Ƕ�
	write_memory(g_ProcessHandle, g_AngleAddr, aim_angle, sizeof(float) * 2);


}



///////////////////////////////////////////////////////////////////////////////////////////////////


//��������
void CheatDoing()
{
	//����������飬�����洢
	PlayerList Players[g_players_count]{ 0 };

	//��ȡ�����Ϣ
	GetPlayerList(Players);

	SetSelfCamp(Players);

	render_player_box(Players);

	if (get_mouse_left_down())AimbotPlayers(Players,90);
}


///////////////////////////////////////////////////////////////////////////////////////////////////

//��ʼ����
void StartCheater() 
{
	//��ȡ����������ַ����Ϣ
	InitializeAddress(ProcessName);
	
	//��ִ�л���ʱ
	g_cheating = CheatDoing;

	//Ѱ����Ҫ�������Ǹ�������
	HWND game_hwnd = FindWindowA("Valve001", "Counter-Strike Source");
	HWND transparent_hwnd = create_transparent_window(game_hwnd);
	
	//��ֵ��ȫ�ֱ���
	g_game_hwnd = game_hwnd;
	g_transparent_hwnd = transparent_hwnd;

	//��ʼ���豸
	initialize_direct3d9(transparent_hwnd);

	//�����¼�
	message_handle(game_hwnd, transparent_hwnd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
