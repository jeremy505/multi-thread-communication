#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


typedef struct __Thread
{
  pthread_t tid;				//线程的ID
  int notifyReceiveFd;		//管道的接收端
  int notifySendFd;			//管道的发送端

}Thread;

Thread *m_Threads;
static int threadcount = 2;

void* work_thread(void* argc)
{
  Thread* param = (Thread*) argc;
  while(1)
  {
  printf("work_thread_tid=%lu\n", param->tid);
  int contant = 0;
  printf("read return %d\n",read(param->notifyReceiveFd, &contant, sizeof(int)));

  printf("read from pipe %d\n", contant);
  }

}

int main(int argc, char** argv)
{
     //在主线程和子线程之间建立管道

    m_Threads = malloc(sizeof(Thread) * threadcount);
    int fds[2];
    if( pipe(fds) )
    {
      perror("create pipe error");
    }


    for(int i = 0; i < threadcount; i++)
    {
      m_Threads[i].notifyReceiveFd = fds[0];
      m_Threads[i].notifySendFd = fds[1];
      pthread_create(&m_Threads[i].tid, NULL,
                     work_thread, (void*)&m_Threads[i]);
      printf("thread_tid=%lu\n", m_Threads[i].tid);
    }

    int contant = 1;
    // sleep(2);
    printf("write %d to pipe\n", contant);
    printf("write return %d\n",write(m_Threads[0].notifySendFd, &contant, sizeof(int)));
    contant = 2;
    printf("write %d to pipe\n", contant);
    printf("write return %d\n",write(m_Threads[0].notifySendFd, &contant, sizeof(int)));
    for(int i = 0; i < threadcount; i++)
    {
      pthread_join(m_Threads[i].tid, NULL);
    }

    return 0;
}
