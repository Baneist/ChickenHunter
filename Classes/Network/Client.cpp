#include "Client.h"
#include <cstring>

using namespace std;
using namespace yasio;
using namespace yasio::inet;

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
                memcpy(&uid, packet.data() + HEAD_LENGTH, 4);
                
            }
            else if (strstr(header, "RO"))
            {
                memcpy(&room, packet.data() + HEAD_LENGTH, sizeof(RoomInformation));
            }
            else if (strstr(header, "MP"))
            {
                memcpy(&map, packet.data() + HEAD_LENGTH, sizeof(MapInformation));
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

bool CHClient::link()
{
    client->open(0, YCK_TCP_CLIENT);
    return client->is_open(thandle);
}

int CHClient::getuid()
{
    return uid;
}

void CHClient::setName(const char* name)
{
    char buf[30] = "SN\0";
    strcpy(buf + HEAD_LENGTH, name);
    client->write(thandle, buf, HEAD_LENGTH + 10);
}

bool CHClient::upload(PlayerAction action)
{
    char str[128]="PA\0";
    memcpy(str + HEAD_LENGTH, &action, sizeof(PlayerAction));
    client->write(thandle, str, HEAD_LENGTH + sizeof(PlayerAction));
    return true;
}

