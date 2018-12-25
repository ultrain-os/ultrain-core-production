#pragma once

#if defined( __linux__ )
#include <unistd.h>
#include <sys/statfs.h>
//#include <sys/time.h>
#endif

#include <stdlib.h>
#include <string.h>

namespace ultrainio {

  const uint16_t PROCESS_ITEM = 14;

class PerformanceMonitor {

  struct Total_Cpu_Occupy_t {
      uint64_t user;
      uint64_t nice;
      uint64_t system;
      uint64_t idle;
      uint64_t iowait;
      uint64_t irq;
      uint64_t softirq;
  };

  struct Proc_Cpu_Occupy_t {
      uint32_t pid;
      uint64_t utime;  //user time
      uint64_t stime;  //kernel time
      uint64_t cutime; //all user time
      uint64_t cstime; //all dead time
  };

public:

  PerformanceMonitor() {
#if defined( __linux__ )
      pid = getpid();
#else
      pid = 0;
#endif
  }

  //get the required start position.
  const char* get_items(const char* buffer, uint32_t item) {
      const char *p =buffer;
      int len = strlen(buffer);
      int count = 0;

      for (int i=0; i<len;i++) {
          if (' ' == *p) {
              count++;
              if(count == item - 1) {
                  p++;
                  break;
              }
          }
          p++;
      }

      return p;
  }

  //get total CPU time
  uint64_t get_cpu_total_occupy() {
      uint64_t totalTime = 0;
#if defined( __linux__ )
      FILE *fd;
      char buff[1024] = {0};
      Total_Cpu_Occupy_t t;

      fd =fopen("/proc/stat","r");
      if (nullptr == fd) {
          return 0;
      }

      fgets(buff, sizeof(buff), fd); //get the first line
      char name[64] = {0};
      sscanf(buff, "%s %ld %ld %ld %ld %ld %ld %ld", name, &t.user, &t.nice, &t.system, &t.idle, &t.iowait, &t.irq, &t.softirq);
      fclose(fd);

      totalTime = t.user + t.nice + t.system + t.idle + t.iowait + t.irq + t.softirq;
#endif
      return totalTime;
  }

  //get CPU time for a process
  uint64_t get_cpu_proc_occupy() {
      uint64_t procTime = 0;
#if defined( __linux__ )
      char file_name[64] = {0};
      Proc_Cpu_Occupy_t t;
      FILE *fd;
      char line_buff[1024] = {0};
      sprintf(file_name, "/proc/%d/stat", pid);

      fd = fopen(file_name, "r");
      if(nullptr == fd) {
          return 0;
      }

      fgets(line_buff, sizeof(line_buff), fd);

      sscanf(line_buff, "%u", &t.pid);
      const char *q = get_items(line_buff, PROCESS_ITEM);
      sscanf(q, "%ld %ld %ld %ld", &t.utime, &t.stime, &t.cutime, &t.cstime);
      fclose(fd);

      procTime = t.utime + t.stime + t.cutime + t.cstime;
#endif

      return procTime;
  }

  //get memory usage for a process
  uint32_t get_proc_mem() {
      uint32_t vmrss = 0;
#if defined( __linux__ )
      char file_name[64] = {0};
      FILE *fd;
      char line_buff[512] = {0};
      sprintf(file_name, "/proc/%d/status", pid);

      fd =fopen(file_name, "r");
      if(nullptr == fd) {
          return 0;
      }

      char name[64];
      while ( fgets(line_buff, sizeof(line_buff), fd) ) {
          memset(name, 64, 0);
          sscanf(line_buff, "%s %d", name, &vmrss); //read first 2 items
          if(strcmp(name, "VmRSS:") == 0) {
              break;
          }
      }
 
      fclose(fd);
#endif

      return vmrss;
  }

  uint32_t get_proc_virtualmem() {
      uint32_t vmsize = 0;
#if defined( __linux__ )
      char file_name[64] = {0};
      FILE *fd;
      char line_buff[512] = {0};
      sprintf(file_name, "/proc/%d/status", pid);

      fd =fopen(file_name, "r");
      if(nullptr == fd) {
          return 0;
      }

      char name[64];
      while ( fgets(line_buff, sizeof(line_buff), fd) ) {
          memset(name, 64, 0);
          sscanf(line_buff, "%s %d", name, &vmsize); //read first 2 items
          if(strcmp(name, "VmSize:") == 0) {
              break;
          }
      }

      fclose(fd);
#endif

      return vmsize;
  }

  uint64_t get_storage_total_size() const {
      uint64_t totalSize = 0;
#if defined( __linux__ )
      struct statfs diskInfo;
      statfs("/root/workspace/log/", &diskInfo);
      auto all = diskInfo.f_blocks - diskInfo.f_bfree + diskInfo.f_bavail;
      totalSize = ((diskInfo.f_bsize >> 10) * all) >> 10; //unit: MB
#endif
      return totalSize;
  }

  uint64_t get_storage_usage_size() const {
      uint64_t usedSize = 0;
#if defined( __linux__ )
      struct statfs diskInfo;
      statfs("/root/workspace/log/", &diskInfo);
      auto usedBlocks = diskInfo.f_blocks - diskInfo.f_bfree;
      usedSize = ((diskInfo.f_bsize >> 10) * usedBlocks) >> 10; //unit: MB
#endif
     return usedSize;
  }

private:
  pid_t pid;
};
}
//#endif
