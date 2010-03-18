/* hiwatermark.c - Source file: Output high water mark for virtual memory and resident memory in KBytes */
/*
  Copyright (c) 2010, by:      Anil Joshi
  All rights reserved
  
  This file may be used subject to the terms and conditions of the
  GNU Library General Public License Version 2, or any later version
  at your option, as published by the Free Software Foundation.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.
 
  This is modeled after top but works only for a one sibgle process and
  only in batch mode
*/
/* Memory information is read in from /proc/#/stat */

#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <values.h>
#include <limits.h>

#include "himemmark.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static struct timeval tv;
#define ZAP_TIMEOUT do{tv.tv_usec=0; tv.tv_sec=0;}while(0)


static double pageSize = 0.0;

static void setPageSize(void)
{
  pageSize = (double) sysconf(_SC_PAGESIZE);
}

static double getKBytes(double nPages)
{
  return((nPages*pageSize)/1024.0);
}

static int file2str(const char* filename,char* ret,int cap) 
{
  int fd;
  int num_read;
  
  fd = open(filename, O_RDONLY, 0);
  if (fd==-1) {
    return (-1);
  }
  num_read = read(fd,ret,(cap-1));
  close(fd);
  if (num_read<=0) {
    return(-1);
  }
  ret[num_read] = '\0';
  
  return(num_read);
}

void statm2proc(const char* s,MemStats ms)
{
  int num;
  long size;
  long resident;
  long share;
  long trs;
  long lrs;
  long drs;
  long dt;
  
  num = sscanf(s
	       ,"%ld %ld %ld %ld %ld %ld %ld"
	       ,&size
	       ,&resident
	       ,&share
	       ,&trs
	       ,&lrs
	       ,&drs
	       ,&dt
	       );
  /* fprintf(stderr, "statm2proc converted %d fields.\n",num); */
  
  msSize(ms) = (double) size;
  msResident(ms) = (double) resident;
}

int getMemStatsOfProc(pid_t pid,MemStats ms)
{
  int procExists = TRUE;
  char dir[PATH_MAX];
  char path[PATH_MAX];
  struct stat sb; /* stat() buffer */
  char sbuf[1024];
  
  sprintf(dir,"/proc/%d",pid);
  sprintf(path,"%s/%s",dir,"statm");
  if (stat(dir,&sb) == -1) {
    procExists = FALSE;
  }
  else if (stat(path,&sb) == -1) {
    procExists = FALSE;
  }
  
  if (procExists) {
    if (file2str(path,sbuf,sizeof(sbuf)) < 0) {
      procExists = FALSE;
    }
    else {
      statm2proc(sbuf,ms);
    }
  }
  
  return(procExists);
}

void parseArgs(int argc,char** argv,pid_t* pidP,int* historyP,double* delay_timeP)
{
  char c;
  char* argv0 = argv[0];
  
  /* Default values */
  *historyP = FALSE;
  *delay_timeP = DEF_INTERVAL;
  
  argc--;
  argv++;
  argv0 = argv[0];
  while ((argc > 0) && (*argv0 == '-')) {
    argv0++;
    c = *argv0;
    if (c) {
      switch (c) {
      case 'h':
	*historyP = TRUE;
	break;
      case 'd':
	if (*(argv0+1) != ' ') {
	  argv0++;
	}
	else {
	  argc--;
	  argv++;
	  argv0 = argv[0];
	}
	*delay_timeP = atof(argv0);
	break;
      default:
	break;
      }
    }
    argc--;
    argv++;
    argv0 = argv[0];
  }
  if (argc <= 0) {
    exit(255);
  }
  *pidP = atoi(argv[0]);
}

int main(int argc,char** argv)
{
  pid_t pid;
  int history = FALSE;
  double delay_time = DEF_INTERVAL;
  struct mem_stats_str ms_buff;
  MemStats ms = &ms_buff;
  struct mem_stats_str ms_hiwatermark_buff;
  MemStats msHiWaterMark = &ms_hiwatermark_buff;
  int procExists = TRUE;
  
  setPageSize();
  msSize(msHiWaterMark) = 0.0;
  msResident(msHiWaterMark) = 0.0;
  
  /* Parse arguments */
  parseArgs(argc,argv,&pid,&history,&delay_time);
  
  while (procExists) {
    /* Set the time delay and wait */
    tv.tv_sec = delay_time;
    tv.tv_usec = (delay_time - (int)delay_time) * 1000000;
    select(0, NULL, NULL, NULL, &tv); /* Delay */
    
    /* Get the proc mem and get the hiwatermarks */
    procExists = getMemStatsOfProc(pid,ms);
    if (procExists) {
      msSize(msHiWaterMark) = maxof(msSize(msHiWaterMark),msSize(ms));
      msResident(msHiWaterMark) = maxof(msResident(msHiWaterMark),msResident(ms));
    }
    if (history) {
      printf("%g %g\n",getKBytes(msSize(ms)),getKBytes(msResident(ms)));
    }
  }
  
  printf("\n");
  printf("High Memory Watermark: %g %g\n"
	 ,getKBytes(msSize(msHiWaterMark))
	 ,getKBytes(msResident(msHiWaterMark))
	 );
  
  return(0);
}
