#pragma once
#include "Windows.h"

//�˴�λ5���ֽڵ���˼������������һ�ֽںͲ������ĸ��ֽ�
constexpr int byte_length = 5;


class inline_hook {

private:

	//�������ض��壬Ϊ���Ҽ�д
	using uchar = unsigned char;
	using dword = DWORD;


	//����洢ԭʼ��ת��������޷����ַ�����
	uchar mine_original_byte[byte_length];

	//����洢�Զ�����ת������޷����ַ�����
	uchar mine_self_byte[byte_length];

	//����洢��ת��ַ��ԭ��ת��ַ
	int mine_original_address;

	//����洢�Զ�����ת��ַ����ת��ַ
	int mine_self_address;

	//�����ڴ�����޸ı����������޸ķ��ʵ��ڴ�Ƭ��������
	//��仰�Լ�д��ʱ����������⣡����ע��attributes������Ϊdword��
	dword modify_memery_attributes(int address,dword attributes=PAGE_EXECUTE_READWRITE)
	{
		//���ԭ�ڴ�����
		dword old_attributes;

		//�ڴ汣������
		VirtualProtect(reinterpret_cast<void*>(address), byte_length, attributes, &old_attributes);

		//����ԭ�ڴ�����
		return old_attributes;
	}



public:
	//���캯�������þ��ǽ����ǵ�original_byte��ȡ����������self_byte�������
	inline_hook(int original_address, int self_address) :mine_original_address(original_address), mine_self_address(self_address)
	{
		//�˴�e9��ʾ�Ĳ����볤��ת����˼��
		mine_self_byte[0] = '\xe9';

		//Ȼ�����ƫ��
		int offset = self_address - (original_address + byte_length);

		//�������Լ�����ת��ַѹ�����в�������Զ�����ת����У����չ���һ����ת���
		memcpy(&mine_self_byte[1], &offset, byte_length - 1);

		//�������Դ�����ڴ�ռ䣬����ʹ�������ڴ�Ƭ����
		dword attributes = modify_memery_attributes(original_address);




		//��ԭ��ַ�д洢�Ĳ�����䣬ѹ�����Ǵ洢ԭʼ������������У������Ժ�ִ�������ǵĴ���󣬿�����ת��ԭ����ִ���Բ���������ı���
		//��仰�Լ�д��ʱ����������⣡�����ǵ�ǿ��ת����
		memcpy(mine_original_byte, reinterpret_cast<void*>(original_address), byte_length);


		//�������ԭʼ������������ǿ��Իָ��ڴ�ռ������
		modify_memery_attributes(original_address, attributes);


	}
	//�˺��������þ���Ϊ����ԭ��ת����ǰ����ת����ת�������Լ��Ĵ���
	void modify_address()
	{

		dword attributes = modify_memery_attributes(mine_original_address);

		//������ǽ������Լ�����ת����ע��ԭ��ת�����ַ������ִ�����ǵ���Ӧ����
		memcpy(reinterpret_cast<void*>(mine_original_address), mine_self_byte, byte_length);

		modify_memery_attributes(mine_original_address, attributes);



	}

	void restore_address()
	{
		dword attributes = modify_memery_attributes(mine_original_address);

		//������ǽ��洢��ԭ��ת����ע��ԭ��ת�����ַ�����Իָ���Ϸ����������
		memcpy(reinterpret_cast<void*>(mine_original_address),mine_original_byte, byte_length);

		modify_memery_attributes(mine_original_address, attributes);
	}
	
};