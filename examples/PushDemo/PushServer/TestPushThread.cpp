#include "TestPushThread.h"
//#include <arpa/inet.h>

map<string, SerUser> PushUser::pushUser;
TC_ThreadMutex PushUser::mapMutex;


void PushInfoThread::terminate(void)
{
    _bTerminate = true;
    {
        tars::TC_ThreadLock::Lock sync(*this);
        notifyAll();
    }
}

void PushInfoThread::setPushInfo(const string &sInfo)
{
    unsigned int iBuffLength = htonl(sInfo.size() + 8);
    unsigned char * pBuff = (unsigned char*)(&iBuffLength);

    _sPushInfo = "";
    for (int i = 0; i < 4; ++i)
    {
        _sPushInfo += *pBuff++;
    }

    unsigned int iRequestId = htonl(_iId);
    unsigned char * pRequestId = (unsigned char*)(&iRequestId);

    for (int i = 0; i < 4; ++i)
    {
        _sPushInfo += *pRequestId++;
    }

    _sPushInfo += sInfo;
}

string PushInfoThread::setPushMsg(const string &sInfo)
{
    unsigned int iBuffLength = htonl(sInfo.size() + 8);
    unsigned char * pBuff = (unsigned char*)(&iBuffLength);

    string sPushInfo = "";
    for (int i = 0; i < 4; ++i)
    {
        sPushInfo += *pBuff++;
    }

    unsigned int iRequestId = htonl(_iId);
    unsigned char * pRequestId = (unsigned char*)(&iRequestId);

    for (int i = 0; i < 4; ++i)
    {
        sPushInfo += *pRequestId++;
    }

    sPushInfo += sInfo;
    return sPushInfo;
}

//定期向客户push消息
void PushInfoThread::run(void)
{
    while (!_bTerminate)
    {
        (PushUser::mapMutex).lock();
        for (auto& it : PushUser::pushUser)
        {
            if (it.first == "matic")
            {
                string pushinfo = it.second.brokerType + "'s  callback ";
                string abc = setPushMsg(pushinfo);
                it.second.currPtr->sendResponse(abc.c_str(), abc.size());
            }
            else if (it.first == "cats")
            {
                string pushinfo = it.second.brokerType + "'s  callback ";
                string abc = setPushMsg(pushinfo);
                it.second.currPtr->sendResponse(abc.c_str(), abc.size());
            }
        }
        (PushUser::mapMutex).unlock();
        if (!(PushUser::pushUser).empty())
        {
        	LOG->debug() <<" user count="<<PushUser::pushUser.size()<<endl;
        }
        {
            TC_ThreadLock::Lock sync(*this);
            timedWait(5000);
        }
    }
}
