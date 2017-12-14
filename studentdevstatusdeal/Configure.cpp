#include "Configure.h"
#include "tinyxml.h"
#include "zlog.h"
extern zlog_category_t* g_server_cat;


CConfigure::CConfigure()
{   
}

CConfigure::~CConfigure()
{
}

// 读取配置文件，解析得到各参数
bool CConfigure::LoadOptions(LPCTSTR lpszFileName)
{
	bool result = true;
	TiXmlNode*		node;
	TiXmlElement*	child;
	const char*		name;

	TiXmlDocument doc(lpszFileName);
	result = doc.LoadFile();
	if (result)
	{
		node = doc.FirstChild("AgentOptions");
		if (node != NULL)
		{
			//清除原有的配置数据，如果存在的话。
			child = node->FirstChildElement();
			while (child != NULL && result)
			{
				name = child->Value();
				if (name != NULL)
				{
					if (_stricmp(name, "Database") == 0)
					{
						result = ParseDBInfo(child);
						if(!result)
						{
							return result;
						}
					}          
                    else if (0 == _stricmp(name,"RmqServer"))
                    {
                         result = ParseRmqServerPara(child);
                    }   
					else if (_stricmp(name, "ChildInPoolCount") == 0)
					{
						const char* p = child->GetText();
						if (p != NULL)
                        {             
                            m_nChildThreadPoolCount = atoi(p);
                            if(m_nChildThreadPoolCount < 1 || m_nChildThreadPoolCount > 1024)
                            {
                               zlog_error(g_server_cat,"ChildInPoolCount parameter error");
                               return false;
                            }
                        }
                        else 
                        {
                             zlog_error(g_server_cat,"ChildInPoolCount config error near %d line\n", 
                            child->Row());
                            return false;
                        }
					}      
				}
				child = child->NextSiblingElement();
			}
          
		}
		else
		{
			result = false;
		    zlog_error(g_server_cat,"Configure file error:format error!Maybe no config file!");
		}
	}
	else
	{
		if (doc.ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE)
		{
			  zlog_error(g_server_cat,"Failure open config file,error: %s", doc.ErrorDesc());
		}
		else
		{
			  zlog_error(g_server_cat,"Failure open config file:error: %s, line: %d\n", doc.ErrorDesc(), doc.ErrorRow());
		}
        result = false;
	}

	return result;
}

LPCTSTR CConfigure::GetLeafValue(TiXmlNode* pNode, LPCTSTR lpszName)
{
	TiXmlElement* pChild;

	pChild = pNode->FirstChildElement(lpszName);
	if (pChild != NULL)
	{
		const char* p = pChild->GetText();
		if (p != NULL)
			return p;
	}
	return "";
}



bool CConfigure::ParseDBInfo(TiXmlNode* ptemp)
{
	bool result = false;
    m_dbconf.strIp = GetLeafValue(ptemp, "IP");
    if(m_dbconf.strIp == "")
    {
   
       printf("m_dbconf ip address parameter error\n");
       return result;
    }
    m_dbconf.strUserID = GetLeafValue(ptemp, "UserID");
    if(m_dbconf.strUserID == "")
    {
   
       printf("m_dbconf rUserID address parameter error\n");
       return result;
    }
    m_dbconf.strPassword = GetLeafValue(ptemp, "Password");
    if(m_dbconf.strPassword == "")
    {
   
       printf("m_dbconf Password address parameter error\n");
       return result;
	}

    m_dbconf.strServiceName = GetLeafValue(ptemp, "ServiceName");
    if(m_dbconf.strServiceName == "")
    {
   
       printf("m_dbconf ServiceName address parameter error\n");
       return result;
	}

	const char* p = GetLeafValue(ptemp, "Port");
	if (p != NULL)
	{					  
		m_dbconf.port = atoi(p);
		if(m_dbconf.port < 1024 || m_dbconf.port > 65535)
		{
		   printf("m_dbconf.port parameter error\n");
		   return result;
		}
	}
	else
	{
		printf("m_dbconf.port parameter error");
		return result;
	}

    const char* p1 = GetLeafValue(ptemp, "ConnNum");
	if (p1 != NULL)
	{					  
		m_dbconf.dbconnNum = atoi(p1);
		if(m_dbconf.dbconnNum <=0)
		{
		   printf("m_dbconf.dbconnNum parameter error\n");
		   return result;
		}
	}
	else
	{
		printf("m_dbconf.dbconnNum parameter error");
		return result;
	}
	
    
    return true;
}

bool CConfigure::ParseRmqServerPara(TiXmlNode* ptemp)
{
    bool bResult = true;
    m_strRmqServerIP = GetLeafValue(ptemp, "IP");
    m_iRmqServerPort = (atoi)(GetLeafValue(ptemp, "Port"));
    m_strrmquser = GetLeafValue(ptemp, "User");
    m_strrmqpasswd = GetLeafValue(ptemp, "Passwd");
    m_strConSumeExchange = GetLeafValue(ptemp, "ConSumeExchange");
    //m_strRespExchange = GetLeafValue(ptemp, "RespExchange");
    m_strQueueName = GetLeafValue(ptemp,"QueueName");
    m_iConsumeIntervalTime = atoi(GetLeafValue(ptemp,"ConsumeIntervalTime"));

    if(("" == m_strRmqServerIP) || (0 == m_iRmqServerPort)
        ||("" == m_strConSumeExchange) || (0 == m_iConsumeIntervalTime)
        ||("" == m_strrmquser) || ("" == m_strrmqpasswd))
    {  
        return false;
    }

    if(m_iRmqServerPort < 1024 || m_iRmqServerPort> 65535)
    {
        zlog_error(g_server_cat,"RmqServer Port parameter error");
        return false;
    }
    if(m_iConsumeIntervalTime <= 0)
    {
        zlog_error(g_server_cat,"RmqServer m_iConsumeIntervalTime parameter error");
        return false;
    }
    return bResult;
}



LPCTSTR CConfigure::GetRmqServerIP()
{
    return m_strRmqServerIP.c_str();
}

int  CConfigure::GetRmqServerPort()
{
    return m_iRmqServerPort;
}

LPCTSTR CConfigure::GetConSumerExchange()
{
    return m_strConSumeExchange.c_str();
}


LPCTSTR CConfigure::GetQueueName()
{
    return m_strQueueName.c_str();
}
int CConfigure::GetConsumeIntervalTime()
{
    return m_iConsumeIntervalTime;
}
LPCTSTR CConfigure::GetRmqUser()
{
    return m_strrmquser.c_str();
}

LPCTSTR CConfigure::GetRmqPasswd()
{
    return m_strrmqpasswd.c_str();
}


