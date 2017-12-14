


#include "stdafx.h"
#include "tinyxml.h"
#include "Configure.h"
#include "zlog.h"					
extern zlog_category_t* g_server_cat;
const char g_sc_XOrKey[] = "NJITS.COM.CN";

CConfigure::CConfigure(void)
{
}
CConfigure::~CConfigure(void)
{
}
const char* CConfigure::GetLeafValue(TiXmlNode* pNode, const char* lpszName)
{
	TiXmlElement* pChild;

	pChild = pNode->FirstChildElement(lpszName);
	if (pChild != NULL)
	{
		const char* p = pChild->GetText();
		if (p != NULL)
			return p;
        else
        {
            printf("Parameter config error near %d line, \
                %s parameter is invalid\n", pChild->Row(),lpszName);
        }
	}
    else
    {
        printf("lack the config item:%s",lpszName);       
    }    
	return "";
}




	bool CConfigure::LoadBasicConfig(const char* lpszFileName )
	{
		bool result = false;
		bool res = false;
		TiXmlNode*		node;
		vector<string>	   configpara;
		vector<string>::iterator iter;
		configpara.clear();
		configpara.push_back("Port");
		configpara.push_back("IP");
		configpara.push_back("UserID");
		configpara.push_back("Password");
		configpara.push_back("ServiceName");
	
	 
		TiXmlElement*	child;
		const char* 	name;
		
		TiXmlDocument doc(lpszFileName);
		res = doc.LoadFile();
	
		if (res)
		{		 
			node = doc.FirstChild("DCOptions");
			if (node != NULL)
			{
				child = node->FirstChildElement();
				while (child)
				{
					name = child->Value();
					if (name != NULL)
					{
	
						for (unsigned int i = 0; i<configpara.size(); i++)
						{
							iter = configpara.begin();						  
							if(_stricmp(name, configpara[i].c_str()) == 0)
							{
								configpara.erase(iter+i);
								break;
							}
						}
						if (_stricmp(name, "Database") == 0)
						{
							res = ParseDBInfo(child);
							if(!res)
							{
								return result;
							}
						}
						
						else if(_stricmp(name, "Port") == 0)
						{
							const char* p = child->GetText();
							if (p != NULL)
							{					  
								m_port = atoi(p);
								if(m_port < 1024 || m_port > 65535)
								{
								   printf("m_port parameter error\n");
								   return result;
								}
							}
							else
							{
								printf("Parameter config error near %d line, RecogniserSvrPort parameter \
								is invalid\n", child->Row());
								return result;
							}
	
						}
						else if(_stricmp(name, "WebPort") == 0)
						{
							const char* p = child->GetText();
							if (p != NULL)
							{					  
								m_webPort = atoi(p);
								if(m_webPort < 1024 || m_webPort > 65535)
								{
								   printf("m_webPort parameter error\n");
								   return result;
								}
							}
							else
							{
								printf("Parameter config error near %d line, RecogniserSvrPort parameter \
								is invalid\n", child->Row());
								return result;
							}
	
						}
						else if (_stricmp(name, "RmqHostName") == 0) 
						{
							const char* p = child->GetText();
							if (p != NULL)
							{     
							m_strrmqhostname = p;
							}
							else 
							{
							  zlog_error(g_server_cat,"RmqHostName config error near %d line\n", 
								child->Row());
								return false;
							}
						}
						else if (_stricmp(name, "RmqPort") == 0)
						{
							const char* p = child->GetText();
							if (p != NULL)
							{   
								m_irmqport = atoi(p);
								if(m_irmqport < 1024 || m_irmqport> 65535)
								{
									  zlog_error(g_server_cat,"RmqPort Port parameter error");
									return false;
								}
							}
							else 
							{
							 zlog_error(g_server_cat,"RmqPort config error near %d line\n", 
								child->Row());
								return false;
							}
						}
						else if (_stricmp(name, "RmqUser") == 0) 
						{
							const char* p = child->GetText();
							if (p != NULL)
							{           
								m_strrmquser= p;
							}
							else 
							{
							   zlog_error(g_server_cat,"RmqUser config error near %d line\n", 
								child->Row());
								return false;
							}
						}
						else if (_stricmp(name, "RmqPasswd") == 0) 
						{
							const char* p = child->GetText();
							if (p != NULL)
							{           
								m_strrmqpasswd= p;
							}
							else 
							{
							   zlog_error(g_server_cat,"RmqPasswd config error near %d line\n", 
								child->Row());
								return false;
							}
						}
						else if (_stricmp(name, "RmqExchanage") == 0) 
						{
							const char* p = child->GetText();
							if (p != NULL)
							{           
							m_strrmqexchanage = p;
							}
							else 
							{
							   zlog_error(g_server_cat,"RmqExchanage config error near %d line\n", 
								child->Row());
								return false;
							}
						}
						else if (_stricmp(name, "RmqQueueName") == 0) 
						{
							const char* p = child->GetText();
							if (p != NULL)
							{        
							 m_strrmqqueuename = p;
							}
							else 
							{
							  zlog_error(g_server_cat,"RmqQueueName config error near %d line\n", 
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
				printf("Configure file error:format error!Maybe no config file!\n");
				return result;
			}
		}
	
		else
		{
			if (doc.ErrorId() == TiXmlBase::TIXML_ERROR_OPENING_FILE)
			{
				printf("Failure open config file,error: %s\n", doc.ErrorDesc());
			}
			else
			{
				printf("Failure open config file:error: %s, line: %d\n", doc.ErrorDesc(), doc.ErrorRow());
			}
			return result;
		}
		result = true;
		return result;
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
std::string CConfigure::getrmqhostname()
{
    return m_strrmqhostname;
}

int CConfigure::getrmqport()
{
    return m_irmqport;
}

std::string CConfigure::getrmqexchanage()
{
    return m_strrmqexchanage;
}


std::string CConfigure::getrmqqueuename()
{
    return m_strrmqqueuename;
}


LPCTSTR CConfigure::GetRmqUser()
{
    return m_strrmquser.c_str();
}

LPCTSTR CConfigure::GetRmqPasswd()
{
    return m_strrmqpasswd.c_str();
}





