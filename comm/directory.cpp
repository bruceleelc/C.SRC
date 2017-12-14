/*
 * (C) 2014 NJITS Inc.
 *
 * Version: 1.0
 *
 * Authors:
 *   wangjinwen <wangjw@njits.com.cn>
 *     - initial release
 *
 */
#include "stdafx.h"
#include "directory.h"
#include <string.h>
#include <iostream>

#ifdef linux 
#include <libgen.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#else
#include <windows.h>
#include <process.h>
#include <stdlib.h>  
#include <direct.h>  
#include <io.h>

#define ACCESS _access  
#define MKDIR(a) _mkdir((a))  
typedef int int32_t;
#endif

namespace estuary {
  namespace common {
    static char* strip_tail_dir_slashes(char* fname, const int len) {
      if (!fname) return NULL;

      int i = len - 1;
      if (len > 1) {
        #ifdef linux
            while (fname[i] == '/')
        #else
            while (fname[i] == '\\')
        #endif
                   fname[i--] = '\0';
      }
      return fname;
    }

    bool Directory::Exists(const char* filename) {
      if (filename == NULL) return false;

      int len = strlen(filename);
      if (len > kMaxPathLength || len < 0) return false;

      #ifdef linux
        struct stat file_info;
      #endif
      char path[kMaxPathLength+1];
      strncpy(path, filename, len);
      path[len] = '\0';
      char* copy = strip_tail_dir_slashes(path, len);

      #ifdef linux
        int32_t result = stat(copy, &file_info);
        return result == 0;
      #else
        
        WIN32_FIND_DATA fd;
        HANDLE hFind = FindFirstFile(copy, &fd); 
        bool RetValue = (hFind != INVALID_HANDLE_VALUE) ? TRUE : FALSE;  
        FindClose(hFind);
        return RetValue;  
      #endif
      
    }

    bool Directory::MyDeleteFile(const char* filename) {
      if (!filename) return false;

      bool ret = true;
      bool exist = true;

      if (!Exists(filename)) exist = false;

      if (exist) {
        if (IsDirectory(filename))
          ret = false;
      }

      if (ret && exist){
        #ifdef linux
            if (unlink(filename) == -1) {
               std::cout << "unlink file: " << filename
                    << " errno: " << errno << std::endl;
                ret = false;
            }
        #else
            int result = MyDeleteFile(filename);
            if(result == 0)
	        {
		        if(GetLastError() == ERROR_FILE_NOT_FOUND)
		        {
					std::cout<<filename<<"File is not exist!"<<std::endl;
		        }
		        else if(GetLastError() == ERROR_ACCESS_DENIED)
		        {
					std::cout<<filename<<"File is only read!"<<std::endl;
		        }
                ret = false;
	        }
        #endif
      }
      return !exist ? true : ret;
    }

#ifdef linux
    bool Directory::DeleteDirectoryRecursively(const char* directory,
                                               const bool delete_flag) {
      if (!directory) return false;

      struct dirent dirent;
      struct dirent* result = NULL;
      DIR* dir = NULL;
      bool ret = true;
      dir = opendir(directory);
      if (!dir) ret = false;

      while (ret && !readdir_r(dir, &dirent, &result) && result) {
        char* name = result->d_name;
        if (strcmp(name, ".") == 0 ||
            strcmp(name, "..") == 0 ||
            strcmp(name, "lost_found") == 0) continue;

        char path[kMaxPathLength];
        snprintf(path, kMaxPathLength, "%s%c%s", directory, '/', name);
        if (IsDirectory(path)) {
          if (!DeleteDirectoryRecursively(path, true)) {
            ret = false;
            break;
          }
        } else {
          if (!MyDeleteFile(path)) {
            ret = false;
            break;
          }
        }
      } // end while

      if (dir != NULL)
        closedir(dir);

      if (!delete_flag) {
        return ret;
      } else {
        return ret ? DeleteDirectory(directory) : ret;
      }
    }

    bool Directory::Rename(const char* srcfilename,
                           const char* destfilename) {
      if (!srcfilename || !destfilename)
        return false;

      bool ret = true;
      if (rename(srcfilename, destfilename) != 0)
        ret = false;
      return ret;
    }

    int64_t Directory::Size(const char* filename) {
      if (!filename) return 0;

      int64_t ret = 0;
      bool is_exists = true;
      if (!Exists(filename))
        is_exists = false;

      if (is_exists) {
        if (IsDirectory(filename))
          is_exists = false;
      }

      if (is_exists) {
        struct stat file_info;
        if (stat(filename, &file_info) == 0)
          ret = file_info.st_size;
      }
      return ret;
    }

    bool Directory::DeleteDirectory(const char* dirname) {
      if (!dirname) return false;

      bool ret = true;
      bool isexist = true;
      if (!Exists(dirname))
        isexist = false;

      if (isexist) {
        if (!IsDirectory(dirname))
          ret = false;
      }

      if (ret && isexist) {
        if (rmdir(dirname) != 0)
          ret = false;
      }
      return !isexist ? true : ret;
    }

#endif

    bool Directory::IsDirectory(const char* dirname) {
      if (!dirname) return false;

      int dir_len = strlen(dirname);
      if (dir_len > kMaxPathLength || dir_len <= 0)
        return false;

      #ifdef linux
        struct stat file_info;
      #endif
      char path[kMaxPathLength];
      strncpy(path, dirname, dir_len);
      path[dir_len] = '\0';
      char* copy = strip_tail_dir_slashes(path, dir_len);

      #ifdef linux
        int result = stat(copy, &file_info);
        return result < 0 ? false : S_ISDIR(file_info.st_mode);
        
      #else
        DWORD ftyp = GetFileAttributesA(copy);
        if (ftyp == INVALID_FILE_ATTRIBUTES)
		    return false;  //something is wrong with your path!

	    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		    return true;   

        return false;       //this is a directory!
      #endif
    }
#ifdef linux
    bool Directory::CreateDirectory(const char* dirname,
                                    const mode_t dir_mode) {
      if (!dirname) return false;

      mode_t umake_value = umask(0);
      umask(umake_value);
      mode_t mode = (S_IRWXUGO & (~umake_value)) | S_IWUSR | S_IXUSR;
      bool ret = false;

      if (mkdir(dirname, mode) == 0 && // create directory success
          (dir_mode == 0 ||            // not chmod
           chmod(dirname, dir_mode) == 0)) {
        ret = true;
      }

      if (!ret && EEXIST == errno)
        ret = true;
      return ret;
    }

    // creats the full path of fullpath, return true on success
    bool Directory::CreateFullPath(const char* fullpath,
                                   const bool with_file,
                                   const mode_t dir_mode) {
      if (!fullpath) return false;

      int32_t len = strlen(fullpath);
      if (len > kMaxPathLength || len <= 0)
        return false;

      char dirpath[kMaxPathLength];
      strncpy(dirpath, fullpath, len);
      dirpath[len] = '\0';
      char* path = dirpath;

      if (with_file)
        path = dirname(dirpath);

      bool ret = true;
      struct stat stats;
      if ((lstat(path, &stats) != 0) || !S_ISDIR(stats.st_mode)) {
        while (*path == '/') path++;

        while (ret) {
          path = strchr(path, '/');
          if (!path) break;

          *path = '\0';

          if (!IsDirectory(dirpath)) {
            if (!CreateDirectory(dirpath, dir_mode)) {
              ret = false;
              break;
            }
          }
          *path++ = '/';

          while (*path == '/')
            path++;
        }

        if (ret) {
          if (!IsDirectory(dirpath)) {
            if (!CreateDirectory(dirpath, dir_mode)) 
              ret = true;
          }
        }
      }
      return ret;
    }

#else
bool Directory::CreateFullPath(const char *pDir)  
{  
    int i = 0;  
    int iRet;  
    int iLen;  
    char* pszDir;  
    char temp[256]={0};
  
    if(NULL == pDir)  
    {  
        return 0;  
    }  
      
    pszDir = strdup(pDir);  
    iLen = strlen(pszDir);  
  
    // 创建全目录  
    for (i = 0; i < iLen; i++)  
    {  
        if (pszDir[i] == '\\')  
        {   
            memset(temp,0,sizeof(temp));
			memcpy(temp,pszDir,i);
            //如果不存在,创建  
            iRet = ACCESS(temp,0);  
            if (iRet != 0)  
            {  
                iRet = MKDIR(temp);  
                if (iRet != 0)  
                {  
                    return false;  
                }   
            }   
        }   
    }  
  
    //iRet = MKDIR(pszDir); 
    free(pszDir);  
    return (iRet == 0) ? true : false;  
}  
#endif
  }
}
