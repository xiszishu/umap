#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    // optind
#include <errno.h>
#include <time.h>

#define NUMPAGES 10000000
#define NUMTHREADS 2

#ifdef _OPENMP
#include <omp.h>
#endif

#include "uffd_handler.h"

volatile int stop_uffd_handler;

static inline uint64_t getns(void)
{
  struct timespec ts;
  int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
  assert(ret == 0);
  return (((uint64_t)ts.tv_sec) * 1000000000ULL) + ts.tv_nsec;
}

typedef struct {
  int numpages;
  int numthreads;
  char *fn;
} optstruct_t;

optstruct_t options;

void getoptions(optstruct_t *options, int argc, char *argv[]) {

  int c;

  while ((c = getopt(argc, argv, "p:t:f")) != -1) {

    switch(c) {
    case 'p':
      options->numpages = atoi(optarg);
      if (options->numpages > 0)
        break;
      else goto R0;
    case 't':
      options->numthreads = atoi(optarg);
      if (options->numthreads > 0)
        break;
      else goto R0;
    case 'f':
      options->fn = optarg;
      break;
    R0:
    default:
      fprintf(stdout, "Usage: %s ",  argv[0]);
      fprintf(stdout, " -p [number of pages], default: %d ", NUMPAGES);	
      fprintf(stdout, " -t [number of threads], default: %d ",  NUMTHREADS);
      fprintf(stdout, " -f [file name], name of existing file to read pages from, default no -f\n");
      exit(1);
    }
  }
}

int main(int argc, char **argv)
{
  int uffd;
  long page_size;
  long num_pages;
  void *region;
  pthread_t uffd_thread;

  page_size = get_page_size();

  options.numpages = NUMPAGES;
  options.numthreads = NUMTHREADS;
  options.fn = NULL;

  getoptions(&options, argc, argv);
  
  num_pages = options.numpages;
  omp_set_num_threads(options.numthreads);
  
  // allocate a memory region to be managed by userfaultfd
  region = mmap(NULL, page_size * num_pages, PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE, -1, 0);
  //MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  if (region == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }
    
  // start the thread that will handle userfaultfd events

  stop_uffd_handler = 0;

  params_t *p = malloc(sizeof(params_t));
  p->uffd = uffd_init(region, page_size, num_pages);
  p->page_size = page_size;
  p->faultnum = 0;
  fprintf(stdout, "%d pages, %d threads\n", num_pages, options.numthreads);
#ifdef USEFILE
  if (!options.fn)
    options.fn = "/tmp/abc.0";
  fprintf(stdout, "USEFILE enabled %s\n", options.fn);
  p->fd = open(options.fn, O_RDONLY);// | O_DIRECT);
  if (p.fd == -1) {
    perror("file open");
    exit(1);
  }
#endif
  
  pthread_create(&uffd_thread, NULL, uffd_handler, p);
  //printf("total number of fault:%d\n",faultnum);
  sleep(1);

  // storage for the latencies for each page
  int num_batches = 10;
  uint64_t *latencies = malloc(sizeof(uint64_t) * num_batches);
  assert(latencies);
  memset(latencies, 0, sizeof(uint64_t) * num_batches);

  // measure latency in batches
  long batch_size=num_pages/num_batches;  

#ifdef TESTBUFFER
  fprintf(stdout, "TESTBUFFER enabled\n");
#endif

  // touch each page in the region
  int value;
  int *cur = region;

#pragma omp parallel for reduction(+:value) //private (value)
  for (long i = 0; i < num_pages; i+=batch_size) {
    uint64_t start = getns();
    for (long j=0;j<batch_size;j++)
      {
	//fprintf(stdout, "mode %llu\n", (unsigned long long)uffdio_register.mode);
	//fprintf(stdout, "cur %x, adddress at i:%d, j:%d, %x\n", cur, i, j, &cur[i*1024 + j*1024]);
	int v = cur[i*1024 + j*1024 + 5];
	//int v = *cur;
	//fprintf(stdout, "%llu\n", (unsigned long long)latencies[i]);
	//fprintf(stdout, "mode %llu\n", (unsigned long long)uffdio_register.mode);
	//fprintf(stdout,"%d\n",faultnum);
	value += v;
	  
#ifndef TESTBUFFER
	int ret = madvise(cur+(i*1024 + j*1024), page_size, MADV_DONTNEED);
	if(ret == -1) { perror("madvise"); assert(0); } 
#endif

      }
    uint64_t dur = getns() - start;
    latencies[i/batch_size] = dur/batch_size;
    //fprintf(stdout, "%llu\n", (unsigned long long)latencies[i]);
    fprintf(stdout, "."); fflush(stdout);
  }
  fprintf(stdout,"\n");
  //------------------check when page is loaded----------------------

  /* #pragma omp parallel for private (value) */
  /*     for (long i = 0; i < num_pages; i+=1) { */
  /*         uint64_t start = getns(); */
  /*         //fprintf(stdout, "mode %llu\n", (unsigned long long)uffdio_register.mode); */
  /* 	//        int v = *((int*)cur); */
  /*         int v = cur[i*1024]; */
  /*         uint64_t dur = getns() - start; */
  /*         latencies[i] = dur; */
  /*         //fprintf(stdout, "%llu\n", (unsigned long long)latencies[i]); */
  /*         //fprintf(stdout, "mode %llu\n", (unsigned long long)uffdio_register.mode); */
  /*         //fprintf(stdout,"%d\n",faultnum); */
  /*         value += v; */
  /* 	int ret = madvise(&cur[i*1024], page_size, MADV_DONTNEED); */
  /* 	if(ret == -1) { perror("madvise"); assert(0); } */
  /*         //cur += page_size; */
  /*     } */
  //-------------------------------------------------------------------
  stop_uffd_handler = 1;
  pthread_join(uffd_thread, NULL);
  //fprintf(stdout, "mode %llu\n", (unsigned long long)uffdio_register.mode);
  fprintf(stdout,"total number of fault:%d, value is %d\n",p->faultnum,value);
		
  uffd_finalize(region, p->uffd, page_size, num_pages);

  for (long i = 0; i < num_batches; i++) {
    fprintf(stdout, "%llu\n", (unsigned long long)latencies[i]);
  }

  free(latencies);
  munmap(region, page_size * num_pages);

  return 0;
}