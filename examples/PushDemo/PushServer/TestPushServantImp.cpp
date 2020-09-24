#include "TestPushServantImp.h"
#include "servant/Application.h"
#include "TestPushThread.h"
#include <sstream>
#include "struct.h"
using namespace std;

//////////////////////////////////////////////////////
void TestPushServantImp::initialize()
{
    //initialize servant here:
    //...
}

//////////////////////////////////////////////////////
void TestPushServantImp::destroy()
{
    //destroy servant here:
    //...
}

int TestPushServantImp::doRequest(tars::TarsCurrentPtr current, vector<char>& response)
{
    //返回给客户端它自己请求的数据包，即原包返回(4字节长度+4字节requestid+buffer)
    const vector<char>& request = current->getRequestBuffer();
    //cout<<request.data()<<endl;
    response = request;
    string sBuf;
    sBuf.assign(request.data() + 8, request.size());
    //cout<<sBuf<<endl;
    TestApp::MsgStgInfo stginfo;
    stginfo.readFromJsonString(sBuf);
    //cout<<sBuf<<" "<<user.userId<<" "<<user.userName<<" "<<user.msg<<endl;
    //保存客户端的信息，以便对客户端进行push消息
    //stringstream keystr;
    //keystr<<current->getIp()<<":"<<current->getPort();
    (PushUser::mapMutex).lock();
    map<string, SerUser>::iterator it = PushUser::pushUser.find(stginfo.brokerType);
    if (it == PushUser::pushUser.end())
    {
        ServerUserInfo userInfo;
        userInfo.currPtr = current;
        userInfo.brokerType = stginfo.brokerType;
        PushUser::pushUser.insert(map<string, SerUser>::value_type(stginfo.brokerType, userInfo));
        LOG->debug() << "connect user: " << stginfo.brokerType << " uid:" << current->getUId() << " hostName:" << current->getHostName() << endl;
    }
    if (stginfo.msgType == "heart")
    {
        LOG->debug() << stginfo.brokerType <<"'s heart"<<endl;
    }
    else if(stginfo.msgType == "stgrsp")
    {
        LOG->debug() << stginfo.brokerType <<"'s rsp"<<endl;
    }
    
    (PushUser::mapMutex).unlock();
    //LOG->debug() << "request:"<<abc<<endl;
    return 0;
}
//客户端关闭到服务端的连接，或者服务端发现客户端长时间未发送包过来，然后超过60s就关闭连接
//调用的方法
int TestPushServantImp::doClose(TarsCurrentPtr current)
{
    stringstream keystr;
    keystr << current->getIp() << ":" << current->getPort();
    (PushUser::mapMutex).lock();
    for (auto& it : PushUser::pushUser)
    {
        stringstream ipport;
        ipport << it.second.currPtr->getIp() << ":" << it.second.currPtr->getPort();
        if (ipport.str() == keystr.str())
        {
            PushUser::pushUser.erase(it.first);
            LOG->debug() << "close ip: " << keystr.str() << endl;
        }
    }
    

    (PushUser::mapMutex).unlock();

    return 0;
}


