#include "Client.h"
#include <cstring>
#include <iostream>

using namespace std;
using namespace yasio;
using namespace yasio::inet;

CHClient* hunter_client = nullptr;

CHClient::CHClient(const char* ip, unsigned short port)
{
	client = new io_service({ ip,port });//�����ŵ�
	client->set_option(YOPT_S_DEFERRED_EVENT, 0);//�������߳��е��������¼�
    client->start([&](event_ptr&& ev) { //��������̺߳���
        switch (ev->kind())
        {
        case YEK_PACKET: {//��Ϣ���¼�
            auto packet = std::move(ev->packet());
            char header[HEAD_LENGTH + 1] = { 0 };
            memcpy(header, packet.data(), HEAD_LENGTH);
            if (strstr(header, "SU"))
            {
                //cout << "DEBUG#:SU" << endl;
                memcpy(&uid, packet.data() + HEAD_LENGTH, 4);
                
            }
            else if (strstr(header, "RO"))
            {
                //cout << "DEBUG#:RO" << endl;
                memcpy(&room, packet.data() + HEAD_LENGTH, sizeof(RoomInformation));
            }
            else if (strstr(header, "MP"))
            {
                memcpy(&map, packet.data() + HEAD_LENGTH, sizeof(MapInformation));
            }
            else if (strstr(header, "GO"))//��Ϸ����
            {
                started = false;
            }
            else if (strstr(header, "ST"))//��Ϸ��ʼ
            {
                started = true;
            }
            fflush(stdout);
            break;
        }
        case YEK_CONNECT_RESPONSE://������Ӧ�¼�
            if (ev->status() == 0)//statusΪ0���� 1������
            {
                thandle = ev->transport();
                client->write(thandle, "GU\0", 4);
            }
            break;
        case YEK_CONNECTION_LOST://���Ӷ�ʧ�¼�
            break;
        }
        });
}

CHClient::~CHClient()
{
    if (client != nullptr)
        delete client;
}

void CHClient::link()
{
    client->open(0, YCK_TCP_CLIENT);
}

int CHClient::getuid()
{
    return uid;
}

void CHClient::setName(const char* name)
{
    char buf[128] = "SN\0";
    strcpy(buf + HEAD_LENGTH, name);
    while (uid == 0)
        ;
    client->write(thandle, buf, HEAD_LENGTH + 10);
}

bool CHClient::upload(PlayerAction action)
{
    char str[sizeof(PlayerAction) + 5] = "PA\0";
    memcpy(str + HEAD_LENGTH, &action, sizeof(PlayerAction));
    client->write(thandle, str, HEAD_LENGTH + sizeof(PlayerAction));
    return true;
}

bool CHClient::isStarted()
{
    return started;
}
