//********************************************************
//文件:Tool.cpp
//说明:各种辅助函数的实现
//作者:zdx
//创建时间:2011-07-28 10:00
//----------------------------------------------------------
//修改记录:
//修改者:张笃续
//修改日期:2011-08-18 10:05
//修改内容:增加除空格函数
//********************************************************

#include "stdafx.h"
#include "Tool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//************************************************************
//	函数:	ReadFile
//	功能:   读文件到字符串
//	访问权: public
//	参数:	Buffer FileName
//	返回值: bool
//***********************************************************
bool ReadFile(char* Buffer,char* FileName)
{
#ifdef linux 
	int fd = -1;
	ssize_t size = -1;
	fd = open(FileName,O_RDONLY);
	if(-1 == fd)
	{
	  printf("File open failed errno =%d",errno);
	   return false;
	} 
	else
	{
	  size = read(fd, Buffer,256);
	  if(size == -1)
	  {
         printf("File read failed errno =%d",errno);
	  }
	   close(fd);
	}
    return true;
#else
	int fd = -1,i;
	int size = -1;
	fd = open(FileName,O_BINARY);
	if(-1 == fd)
	{
	   printf("File open failed errno =%d",errno);
	   return false;
	} 
	else
	{
	  size = read(fd, Buffer,256);
	  if(size == -1)
	  {
         printf("File read failed errno =%d",errno);
	  }
	  close(fd);
	}
    return true;
#endif	
}
//************************************************************
//	函数:	SaveFile
//	功能:   生成 File
//	访问权: public
//	参数:	Buffer FileName
//	返回值: bool 
//***********************************************************
bool SaveFile(char* Buffer,char* FileName)
{

    FILE *fd;
	if((fd = fopen(FileName,"wb")) == NULL)
	{
	   printf("File open failed errno =%d",errno);
	   return false;
	} 
	else
	{
	  int size = fwrite(Buffer,1,strlen(Buffer),fd);
	  if(size ==-1)
	   {
        printf("File write failed errno =%d",errno);
	   }
	   fclose(fd);
	}
    return true;
}

//**************************************************************
//	函数:	isAllNum
//	功能:	检测所有字符均为数字
//	参数:
//	- szData:待检查字符串
//	- iSize:长度
//	返回值:
//	均为数字		返回 TRUE
//	存在非数字字符	返回 FALSE
//*************************************************************
bool isAllNum(const char* szData)
{
	int iSize = strlen(szData);
	for (int i=0;i<iSize;i++)
	{
		if(szData[i]<'0'||szData[i]>'9')
		{
			return false;
		}
	}
	return true;
}

//************************************************************
//	函数:	rmSpace
//	功能:	删除字符串左右空格和制表符
//	访问权:	public
//	参数:	char * str
//	返回值:	char *
//************************************************************
char *rmSpace(char *str)
{
	unsigned int len,i;
	//从字符串左边, 除去空格和制表符
	for(i=0,len=strlen(str);(i<len)&&(str[i]==0x20 || str[i]==0x09);str[len]='\0',i=0,len--)
		for(i=0;i<len;i++)
			str[i]=str[i+1];
	//从字符串右边, 除去空格和制表符
	for(;(len>0)&&(str[len-1]==0x20 || str[len-1]==0x09);--len)
		str[len-1]='\0';

	return(str);
}


//**************************************************************
//	函数:	GetConfigString
//	功能:	获取*.ini配置文件字符型配置选项
//	参数:
//	- psFile:文件名字串
//	- psItem:指定项名字串
//	- psName:指定名称
//	- szDataBuf:接收字串内容缓冲区
//	- iBufLen:缓冲区szDataBuf的长度
//	- szDefaultValue:默认值
//	返回值:
//	- 成功	返回字串内容指针
//	- 失败	返回NULL, 将缓冲赋值为默认值
//	************************************************************
char* GetConfigString( const char* psFile, const char* psItem, const char* psName,char* 
szDataBuf,unsigned int iBufLen,const char* szDefaultValue)
{
FILE* pFileStream;
unsigned char	szBuf[256],*p,*q;
int		i;
char	*pBuf,bItemFlag='\0';
unsigned int		iLen=0;


	if ( !psFile || !psItem || !psName || !szDataBuf )
	{
		return	(NULL);
	}
	

	if ( ! (pFileStream=fopen(psFile,"rt") ) )
	{
		return	(NULL);
	}

	for(;;)	{
		/* 读取一行数据 */
		if ( !fgets((char*)szBuf,255,pFileStream) )
			break;
		p = szBuf;
		while ( *p==32 || *p==9 ) /* 滤除空格和制表符 */
			p++;

		/* 注释或回车符 */
		if ( !*p || *p=='#' || ( *p=='/' && *(p+1)=='/') || *p==13 || *p==10 )
			continue;

		if ( bItemFlag ) { /* 已经匹配Item */
			if ( *p == '[' ) /* 结束未找到指定Name */
				break;
			
			q=(unsigned char*)psName;

			while ( (*p==*q) && (*p!='=') && *q ) {
				p++;
				q++;
			}
			while ( *p==32 || *p==9 ) /* 滤除空格和制表符 */
				p++;

			if ( ( *p!='=' ) || *q  ) /* 不匹配Name */
				continue;

			p++;
			while ( *p==32 || *p==9 ) /* 滤除空格和制表符 */
				p++;
			
			pBuf = szDataBuf;
			i = 0;
			while ( !( !*p || *p=='#' || *p==9 || ( *p=='/' && *(p+1)=='/') || *p==13 || *p==10 )) {
				if ( *p==32 ) /* 计算字符间空格符 */
					i++;
				else {
					while ( i ) {
						i--;
						iLen++;
						if(iLen>iBufLen)
						{
							fclose( pFileStream );
							return (NULL);
						}
						*pBuf++ = 32;
					}
					iLen++;
					if(iLen>iBufLen)
					{
						fclose( pFileStream );
						return (NULL);
					}
					*pBuf++ = *p;
				}
				p++;
			}
			iLen++;
			if(iLen>iBufLen)
			{
				fclose( pFileStream );
				return (NULL);
			}
			*pBuf = '\0';

			fclose( pFileStream );
			return	rmSpace(szDataBuf);
		}

		/* 分析是否是指定Item项 */
		else {
			if ( *p != '[' )
				continue;
			p++;
			while ( *p==32 || *p==9 ) /* 滤除空格和制表符 */
				p++;

			q=(unsigned char*)psItem;

			while ( (*p==*q) && (*p!=']') && *q ) {
				p++;
				q++;
			}
			while ( *p==32 || *p==9 ) /* 滤除空格和制表符 */
				p++;

			if ( ( *p==']' ) && !*q  ) /* 匹配Item */
				bItemFlag='9';
		} /* End of else */
	}

	fclose( pFileStream );

	if (szDefaultValue != NULL && iBufLen>strlen(szDefaultValue))	/* 未在文件中找到对应匹配项则使用默认值*/
	{
		strncpy(szDataBuf,szDefaultValue,strlen(szDefaultValue));
	}
	return	NULL;
}

//**************************************************************
//	函数:	GetConfigInt
//	功能:	获取*.ini配置文件数字型配置选项
//	参数:
//	- psFile:文件名字串
//	- psItem:指定项名字串
//	- psName:指定名称
//	- iDefaultValue:默认值
//	返回值:
//	- 成功	返回参数值
//	- 失败	返回默认值
//9**********************************************************
int GetConfigInt( const char* psFile, const char* psItem, const char* psName,const int iDefaultValue)
{
	char szDataBuf[128];
	if( GetConfigString(psFile,psItem,psName,szDataBuf,64,NULL) != NULL && isAllNum(szDataBuf))
	{
		return atoi(szDataBuf);
	}
	else
	{
		return iDefaultValue;
	}
	
}


// 解密HEX格式的密文并转换成明文
bool XorDecodeFromHexString( const char* szSrc,char* szDes,const int iDesLeng,
const char* szKey,const int iKeyLeng )
{
	char* buff = NULL;
	int iOrgLen = strlen(szSrc);
	int TextLeng = 0;
	if (szSrc == NULL || szDes == NULL || szKey == NULL ||
		iDesLeng < (iOrgLen/2) || iOrgLen < 1 )
	{
		return false;
	}

	buff = new char[iOrgLen/2+1];

	TextLeng = HexstrToAscii(szSrc,iOrgLen,buff,(iOrgLen/2+1));
	if (TextLeng < 1)		// 转换
	{
		fprintf(stderr,"tool AsciiToHexstr error!\n");
		delete [] buff;
		buff = NULL;
		return false;
	}

	if(!XorEncode(buff,szDes,TextLeng,szKey,iKeyLeng))		// 直接解密
	{
		fprintf(stderr,"tool Encode error!\n");
		delete [] buff;
		buff = NULL;
		return false;
	}

	delete [] buff;
	buff = NULL;
	return true;
}

int HexstrToAscii( const char* szSrc,int iSrcLeng,char* szDest,int iDesLeng )
{
	if(szSrc == NULL || szDest == NULL || (iSrcLeng%2) != 0 || iDesLeng < (iSrcLeng/2+1) )
		return 0;
	char buff[16];		//中间buff
	int i;
	unsigned int ival;
	for(i=0;i<(iSrcLeng/2);i++)
	{
		buff[0]=toupper(szSrc[i*2]);
		buff[1]=toupper(szSrc[i*2+1]);
		buff[2]=0x00;
		sscanf(buff,"%02x",&ival);
		szDest[i] = ival;
	}
	return iSrcLeng/2;
}

bool XorEncode( const char* szSrc,char* szDes,const int iStrLeng,const char* 
szKey,const int iKeyLeng )
{
	int i,k;
	if(szSrc == NULL || szDes == NULL || szKey == NULL)
		return false;
	for(i=0,k=0;i<iStrLeng;i++)
	{
		szDes[i] = szSrc[i]^szKey[k];
		k++;
		if(k >= iKeyLeng)
			k =0;
	}
	return true;
}


#ifdef linux
/**
 * 检查文件是否存在
 * @file 文件路径
 * @return 如果存在返回TRUE，否则FALSE
 */
int file_exist(const char *file) {

  struct stat buf;

  ASSERT(file);

  return (stat(file, &buf) == 0);

}

/**
 * 检查是否文件是规则的
 * @param 路径
 * @return 规则文件TRUE，否则FALSE
 *
 */
int file_isFile(const char *file) {

  struct stat buf;

  ASSERT(file);

  return (stat(file, &buf) == 0 && S_ISREG(buf.st_mode));

}
/*
 *  获取已存在的实例的PID
 *  @pidfile 存放pid的文件的路径
 *  @prog	   程序名称
 */
pid_t tool_getPid(const char *pidfile,const char* prog) {
        FILE *file= NULL;
        int pid= -1;

        ASSERT(pidfile);

        if(! file_exist(pidfile)) {
                printf("%s: pidfile '%s' does not exist\n",prog, pidfile);
                return FALSE;
        }
        if(! file_isFile(pidfile)) {
        		printf("%s: pidfile '%s' is not a regular file\n",prog, pidfile);
                return FALSE;
        }
        if((file = fopen(pidfile,"r")) == (FILE *)NULL) {
        		printf("%s: Error opening the pidfile '%s' -- %s\n", prog, pidfile, STRERROR);
                return FALSE;
        }
        if(fscanf(file, "%d", &pid) != 1) {
        		printf("%s: Error reading pid from file '%s'\n", prog, pidfile);
                if (fclose(file))
                	printf("%s: Error closing file '%s' -- %s\n", prog, pidfile, STRERROR);
                return FALSE;
        }
        if (fclose(file))
        		printf("%s: Error closing file '%s' -- %s\n", prog, pidfile, STRERROR);

        if(pid < 0)
                return(FALSE);

        return(pid_t)pid;

}

/**
 * 是否已有实例存在
 * @pidfile pid文件路径
 * @prog 程序名称
 * @若存在，则返回原进程的PID 否则返回 FALSE
 */
int exist_instance(const char *pidfile,const char* prog)
{
	pid_t pid;
	errno= 0;
	if ((pid = tool_getPid(pidfile, prog)))
		if ((getpgid(pid)) > -1 || (errno == EPERM))
			return ((int) pid);
	return (FALSE);

}
/**
 * 是否已有实例存在
 * @szAppName 程序名称
 * @若存在，则返回原进程的PID 否则返回 FALSE
 */
int exist_instance(const char *szAppName)
{
	char szPath[64];
	sprintf(szPath,"%s%s.pid",PIDROOT,szAppName);
	return exist_instance(szPath,szAppName);
}


/*
 * 无阻塞读按键
 * @返回值 按键值
 */
int getkey(void) {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

    return character;
}

/*
 * 记录进程PID到 /var/run/[appname].pid
 * @成功，TRUE，其他FALSE
 */
int writePid(const char *szAppName)
{
	char szPath[64];
	sprintf(szPath,"%s%s.pid",PIDROOT,szAppName);
	pid_t pid;
	FILE *fp = fopen(szPath,"w");
	if(fp!= NULL)
	{
		pid = getpid();
		fprintf(fp,"%d",pid);
		fclose(fp);
		return TRUE;
	}
	else
	{
		printf("Can't open file %s \n",szPath);
		return FALSE;
	}
}

/*
 * 发送终止信号到对应程序进程
 * @pid 进程pid
 */
int sendExitSignal(pid_t pid)
{
	return kill(pid,SIGTERM);
}

/*
 * 发送终止信号到对应程序进程，根据读取pid文件获取
 * @szAppName 进程名称
 */
int sendExitSignal(const char* szAppName)
{
	pid_t pid = exist_instance(szAppName);
	if(pid)
	{
		return kill(pid,SIGTERM);
	}
	else
	{
		printf("%s is not runing!\n",szAppName);
		return FALSE;
	}
}

vector<string> split(char *src,char delimiters)
{
	vector<string> vec;
	char *p ,*ptmp;
	p = src; 
	ptmp = src;
	while((*p)!='\0') 
	{
		if (*p == delimiters)
		{

			
			string tmp(ptmp,p);
			vec.push_back(tmp);
			
			ptmp = p+1;
			while(*ptmp == delimiters)
			{
				vec.push_back(",");
				ptmp++;
				p++;
			}
			p++;
		}
		else
		{
			p++;
		}
		
	} 
	if ((*ptmp)!='\0')
	{
		vec.push_back(ptmp);
	}
	return vec;
}

int GetCurPath(char *szBuff, int iLeng)
{
	int  iRet = -1;
#ifdef linux
	iRet = readlink("/proc/self/exe" , szBuff , iLeng);
#endif
	return iRet;
}
#endif	//function for linux


