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
static int threadcount = 1;

void* work_thread(void* argc)
{
  Thread* param = (Thread*) argc;
  printf("childthread_tid=%lu\n", param->tid);
  int contant = 0;
  //sleep(2);
  printf("childthread--read return %d\n",read(param->notifyReceiveFd, &contant, sizeof(int)));
  printf("childthread--read from pipe %d\n", contant);
  contant = 2;
  printf("childthread--write %d to pipe\n", contant);
  printf("childthread--write return %d\n",write(param->notifySendFd, &contant, sizeof(int)));
}

int main(int argc, char** argv)
{
     //在主线程和子线程之间建立管道
    m_Threads = malloc(sizeof(Thread) * threadcount);
    int fds[2];
    if( pipe(fds) )
    {
      perror("create pipe fds error");
    }

    int fds_1[2];
    if( pipe(fds_1) )
    {
      perror("create pipe fds_1 error");
    }

    m_Threads[0].notifyReceiveFd = fds[0];
    m_Threads[0].notifySendFd = fds_1[1];

	pthread_create(&m_Threads[0].tid, NULL,
                   work_thread, (void*)&m_Threads[0]);
    printf("mainthread_tid=%lu\n", m_Threads[0].tid);
    int contant = 1;
    // sleep(2);
    printf("mainthread--write %d to pipe\n", contant);
    printf("mainthread--write return %d\n",write(fds[1], &contant, sizeof(int)));
    printf("mainthread--read return %d\n",read(fds_1[0], &contant, sizeof(int)));
    printf("mainthread--read from pipe %d\n", contant);

    pthread_join(m_Threads[0].tid, NULL);

    close(fds[0]);
    close(fds[1]);
    close(fds_1[0]);
    close(fds_1[1]);
    return 0;
}
