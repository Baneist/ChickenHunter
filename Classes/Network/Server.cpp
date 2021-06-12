#include "Server.h"

int CHServer::get_unused_uid()
{
    for (int i = 1; i < MAX_CONNECTIONS; i++)
        if (uid_usage[i] == false)
        {
            uid_usage[i] = true;
            return i;
        }
    return -1;
}

bool CHServer::delete_uid(int id)
{
    if(uid_usage[id] == false)
        return false;
    uid_usage[id] = false;
}

CHServer::CHServer(const char* ip, unsigned short port)
{
    server = new io_service({ ip,port });//�����ŵ�
    server->set_option(YOPT_S_DEFERRED_EVENT, 0);//�������߳��е��������¼�
    server->start([&](event_ptr&& ev) { //��������̺߳���
        switch (ev->kind())
        {
        case YEK_PACKET: {//��Ϣ���¼�
            auto packet = std::move(ev->packet());
            char header[HEAD_LENGTH + 1] = { 0 };
            memcpy(header, packet.data(), HEAD_LENGTH);
            auto thandle = ev->transport();
            if (strstr(header, "GU"))
            {
                //cout << "DEBUG#:GU" << endl;
                int id = uid[thandle];
                char buf[HEAD_LENGTH + 8] = "SU\0";
                memcpy(buf + HEAD_LENGTH, &id, 4);
                server->write(thandle, buf, HEAD_LENGTH + 4);
            }
            else if (strstr(header, "SN"))
            {
                //cout << "DEBUG#:SN" << endl;
                char buf[20] = { 0 };
                memcpy(buf, packet.data() + HEAD_LENGTH, packet.size() - HEAD_LENGTH);
                string s = buf;
                player_name[uid[thandle]] = s;
            }
            else if (strstr(header, "PA"))
            {
                //cout << "DEBUG#:PA" << endl;
                memcpy(&paction[uid[thandle]], packet.data() + HEAD_LENGTH, sizeof(MapInformation));
            }
            fflush(stdout);
            break;
        }
        case YEK_CONNECT_RESPONSE://������Ӧ�¼�
            if (ev->status() == 0)//statusΪ0���� 1������
            {
                auto thandle = ev->transport();
                uid[thandle] = get_unused_uid();
            }
            break;
        case YEK_CONNECTION_LOST://���Ӷ�ʧ�¼�
            auto thandle = ev->transport();
            delete_uid(uid[thandle]);
            uid.erase(thandle);
            break;
        }
        });
}

CHServer::~CHServer()
{
    if (server != nullptr)
        delete server;
}

void CHServer::listen()
{
    if (server != nullptr)
        server->open(0, YCK_TCP_SERVER);
}

void CHServer::update()
{
    for (int i = 1; i < MAX_CONNECTIONS; i++)
    {
        //if(paction[i])
    }
}
