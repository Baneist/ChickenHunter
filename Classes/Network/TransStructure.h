#ifndef TRANSSTRUCTURE_H
#define TRANSSTRUCTURE_H

const int MAX_CONNECTIONS = 11;
const int MAX_NAME_LENGTH = 9;
//��ͷ����
const int HEAD_LENGTH = 4;
/*
* �ͻ��˶���
* GU = ��ѯuid
* SN = ��������
* PA = ��Ҳ�����Ϣ
* ST = ��ʼ��Ϸ
* GS = �յ���ʼ��Ϣ
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
	short speed[4] = { 0 };
	short hp_change = 0;
	bool is_shoot = false;
	float shoot_degree, shoot_speed;
	int shoot_damage;
	bool get_things = false;
};

struct PlayerInformation
{
	bool alive = 0;
	int uid = 0;
	int hp = 100;
	float position_x = 0, position_y = 0;
	bool speed[4] = { 0 };
	bool is_shoot = false;
	float shoot_degree, shoot_speed;
	short bullet;
};

struct RoomInformation
{
	int player_num;
	bool player_alive[MAX_CONNECTIONS];
	char player_name[MAX_CONNECTIONS][10];
};

struct MapInformation
{
	bool is_updated = false;
	int player_left_num;
	PlayerInformation player[MAX_CONNECTIONS];
};

#endif 