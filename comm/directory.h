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
#ifndef ESTUARY_COMMON_DIRECTORY_H_
#define ESTUARY_COMMON_DIRECTORY_H_

#ifdef linux
#include <unistd.h>
#include <stdint.h>
#else
typedef unsigned int mode_t;
typedef long long int int64_t;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

namespace estuary {
  namespace common {
#ifndef S_IRWXUGO
#define S_IRWXUGO (S_IRWXU | S_IRWXG | S_IRWXO)
#endif
    
    class Directory {
     public:
      const static int kMaxPathLength = 256;
      
      static bool Exists(const char* filename);
      static bool MyDeleteFile(const char* filename);
      static bool IsDirectory(const char* dirname);
	#ifdef linux
		static bool DeleteDirectory(const char* dirname);
      		static bool DeleteDirectoryRecursively(const char* directory,
                                             const bool delete_flag = false);
		static bool Rename(const char* srcfilename, const char* destfilename);
      		static int64_t Size(const char* filename);
		static bool CreateFullPath(const char* fullpath,
                                 const bool with_file = false,
                                 const mode_t dir_mode = 0);
	#else
		static bool CreateFullPath(const char* fullpath);
	#endif
      static bool CreateDirectory(const char* dirname, const mode_t dir_mode = 0);


    };
  }
}

#endif
