#ifndef TRANSSTRUCTURE_H
#define TRANSSTRUCTURE_H

const int MAX_CONNECTIONS = 11;

//��ͷ����
const int HEAD_LENGTH = 4;
/*
* �ͻ��˶���
* GU = ��ѯuid
* SN = ��������
* PA = ��Ҳ�����Ϣ
* ST = ��ʼ��Ϸ
* 
* ����˶���
* SU = ����uid
* RO = ������Ϣ
* MP = ��ͼ��Ϣ
* GO = ��Ϸ����
* ST = ֪ͨ������ҿ�ʼ��Ϸ
*/



#define PA_NONE 0
#define PA_LEFT 1
#define PA_RIGHT 2
#define PA_UP 3
#define PA_DOWN 4
#define PA_PICK 5
#define PA_DISCARD 6

struct PlayerAction
{
	short keyboard_action = PA_NONE;
	short hp_change = 0;
	bool is_shoot = false;
	float shoot_degree, shoot_speed;
	int shoot_damage;
};

struct PlayerInformation
{
	bool alive = 0;
	int uid = 0;
	int hp = 100;
	float position_x = 0, position_y = 0;
	bool is_shoot = false;
	float shoot_degree, shoot_speed;
};

struct RoomInformation
{
	int player_num;
	char player_name[MAX_CONNECTIONS][10];
};

struct MapInformation
{
	int player_num;
	PlayerInformation player[MAX_CONNECTIONS];
};

#endif 