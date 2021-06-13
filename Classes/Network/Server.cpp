#include "Server.h"
#include <cstdlib>
#include <iostream>
using namespace std;

int CHServer::get_unused_uid()
{
    for (int i = 1; i < MAX_CONNECTIONS; i++)
        if (uid_usage[i] == false)
        {
            uid_usage[i] = true;
            connection_num++;
            return i;
        }
    return -1;
}

bool CHServer::delete_uid(int id)
{
    if (uid_usage[id] == false)
        return false;
    uid_usage[id] = false;
    connection_num--;
    return true;
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
                if(debug_mode) cout << "uid:" << uid[thandle] << " DEBUG#:GU" << endl;
                int id = uid[thandle];
                char buf[HEAD_LENGTH + 8] = "SU\0";
                memcpy(buf + HEAD_LENGTH, &id, 4);
                server->write(thandle, buf, HEAD_LENGTH + 4);
            }
            else if (strstr(header, "SN"))
            {
                if (debug_mode) cout << "uid:" <<uid[thandle] << " DEBUG#:SN" << endl;
                char buf[20] = { 0 };
                memcpy(buf, packet.data() + HEAD_LENGTH, packet.size() - HEAD_LENGTH);
                string s = buf;
                player_name[uid[thandle]] = s;
            }
            else if (strstr(header, "PA"))
            {
                if (debug_mode) cout << "uid:" << uid[thandle] << " DEBUG#:PA" << endl;
                memcpy(&paction[uid[thandle]], packet.data() + HEAD_LENGTH, sizeof(MapInformation));
            }
            fflush(stdout);
            break;
        }
        case YEK_CONNECT_RESPONSE://������Ӧ�¼�
            if (ev->status() == 0)//statusΪ0���� 1������
            {
                auto thandle = ev->transport();
                int id = get_unused_uid();
                uid[thandle] = id;
                uid_to_handle[id] = thandle;
                if (debug_mode) cout << "Client#" << thandle << "#comes in uid:" << id << endl;
                if (started)
                {
                    smap.player[id].alive = true;
                    smap.player[id].position_x = 100, smap.player[id].position_y = 100, smap.player[id].hp = 100;
                }
            }
            break;
        case YEK_CONNECTION_LOST://���Ӷ�ʧ�¼�
            auto thandle = ev->transport();
            int id = uid[thandle];
            if (debug_mode) cout << "Client#" << thandle << "#lost connection! uid:" << id << endl;
            delete_uid(uid[thandle]);
            uid.erase(thandle);
            uid_to_handle.erase(id);
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

#define CHRAND() ((float)rand()/RAND_MAX)
void CHServer::map_init(int seed)
{
    srand(seed);
    started = true;
    for(int i = 1; i < MAX_CONNECTIONS;i++)
        if (uid_usage[i] == true)
        {
            smap.player[i].alive = true;
            smap.player[i].position_x = CHRAND() * 200;
            smap.player[i].position_y = CHRAND() * 200;
            smap.player[i].hp = 100;
        }
    
}

void CHServer::map_update()
{
    //�ͻ����ϴ������ݶ�ȡ
    if (connection_num == 0)
        return;
    //if (debug_mode)cout << "DEBUG#MAP_UPDATE #START#" << endl;
    for (int i = 1; i < MAX_CONNECTIONS; i++)
    {
        if (uid_usage[i] && smap.player[i].alive)
        {
            switch (paction[i].keyboard_action)
            {
            case PA_LEFT: smap.player[i].position_x += -4; break;
            case PA_RIGHT: smap.player[i].position_x += 4; break;
            case PA_UP: smap.player[i].position_y += 4; break;
            case PA_DOWN: smap.player[i].position_y += -4; break;
            }
            if (paction[i].is_shoot)
            {
                ;
            }
            memset(&paction[i], 0, sizeof(PlayerAction));
        }
    }
    //���µ�ͼ
    memset(&map_trans, 0, sizeof(MapInformation));
    map_trans.player_num = connection_num;
    for (int i = 1; i < MAX_CONNECTIONS; i++)
        if (uid_usage[i] && smap.player[i].alive)
        {
            map_trans.player[i].position_x = smap.player[i].position_x;
            map_trans.player[i].position_y = smap.player[i].position_y;
            map_trans.player[i].hp = map_trans.player[i].hp;
        }
    for (int i = 1; i < MAX_CONNECTIONS; i++)
        if (uid_usage[i])
        {
            char buf[sizeof(MapInformation) + HEAD_LENGTH + 2] = "MP\0";
            memcpy(buf + HEAD_LENGTH, &map_trans, sizeof(MapInformation));
            if (debug_mode)cout << "Send Map to #" << uid_to_handle[i] << "#" << endl;
            server->write(uid_to_handle[i], buf, sizeof(PlayerInformation) + HEAD_LENGTH);
        }
    //if (debug_mode)cout << "DEBUG#MAP_UPDATE #OVER#" << endl;
}

int CHServer::getConnectionNum()
{
    return connection_num;
}

void CHServer::open_debug_mode()
{
    debug_mode = !debug_mode;
    cout << "DEBUG MODE IS " << (debug_mode ? "OPEN" : "CLOSED") << endl;
}
