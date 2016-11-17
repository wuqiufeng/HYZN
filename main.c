#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <pthread.h>
#include "ring_buffer.h"

#define BUFFER_SIZE         1024*1024*8
#define BUFFER_SIZE_WR      1024*1024*32
#define BUFFER_SIZE_PS      1024*1024*64

pthread_mutex_t f_lock[2];
char *buf[2];
int  if_runthread=1;

void *read_proc(void *arg)
{
    int filefd = 0;
    uint32_t length = 0;
    char *buf = NULL;
    struct stat st;
    struct data *wr_ps = NULL;

    buf = (char *)malloc(BUFFER_SIZE);
    if (!buf)
    {
        fprintf(stderr, "Failed to buf malloc memory.\n");
        return NULL;
    }
    wr_ps = (struct data *)malloc(sizeof(struct data));
    if (!wr_ps)
    {
        fprintf(stderr, "Failed to wr_ps malloc memory.\n");
        return NULL;
    }

    wr_ps = (struct data *)arg;
    struct ring_buffer *ring_buf_wr  = NULL;
    struct ring_buffer *ring_buf_ps  = NULL;

    ring_buf_wr = wr_ps->ring_buf_wr;
    ring_buf_ps = wr_ps->ring_buf_ps;

    memset(buf, 0, BUFFER_SIZE);
    filefd = open("./first.jpg", O_RDONLY);
    if (filefd ==-1)
    {
        perror("Failed to open file.\n");
        return NULL;
    }
    if (fstat(filefd, &st) < 0) {
            perror("Failed to stat file");
            close(filefd);
            return NULL;
    }
    length = st.st_size;
    memcpy(buf, &length, sizeof(uint32_t));

    if (read(filefd, buf+sizeof(uint32_t), BUFFER_SIZE) == -1)
    {
        perror("Failed to read file");
        close(filefd);
        return NULL;
    }
    close(filefd);

    while (if_runthread)
    {

        printf("**********************\n");
        printf("ring buffer length: %u\n", ring_buffer_len(ring_buf_wr));
        ring_buffer_put(ring_buf_wr, (void*)buf, length + sizeof(uint32_t));
        ring_buffer_put(ring_buf_ps, (void*)buf, length + sizeof(uint32_t));
        printf("ring buffer length: %u\n", ring_buffer_len(ring_buf_wr));
        printf("**********************\n\n");
        usleep(100000);
    }
    return NULL;
}

void *write_proc(void *arg)
{
    int filefd,i = 100;
    uint32_t length = 0;
    char path[128] = {0};
    struct ring_buffer *ring_buf = (struct ring_buffer *)arg;

    while(if_runthread)
    {
        memset(buf[0], 0, BUFFER_SIZE);
        printf("---------------\n");
        printf("get a buf info from ring buffer.\n");
        if(ring_buffer_get(ring_buf, (void*)&length, sizeof(uint32_t))>0)
        {
            printf("length[%u]\n", length);
            ring_buffer_get(ring_buf, (void*)buf[0], length);
            //sprintf(path, "/media/fhz/9e576200-94ab-4c16-b768-b08cfff8fa5f/test/000%d.jpg", i++);
            sprintf(path, "./000%d.jpg", i++);
            filefd = open(path, O_CREAT|O_WRONLY, 0644);
            write(filefd, buf[0], length);
            close(filefd);
            printf("---------------\n\n");
        }
        usleep(80000);
    }
    return NULL;
}

void *parser_proc(void *arg)
{
    uint32_t length = 0;
    struct ring_buffer *ring_buf = (struct ring_buffer *)arg;

    while(if_runthread)
    {
        memset(buf[1], 0, BUFFER_SIZE);
        if(ring_buffer_get(ring_buf, (void*)&length, sizeof(uint32_t))>0)
        {
            ring_buffer_get(ring_buf, (void*)buf[1], length);
            printf("parser_proc work\n");
        }
        usleep(100000);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int i = 0;
    void *buffer_wr = NULL;
    void *buffer_ps = NULL;
    uint32_t size_wr = 0;
    uint32_t size_ps = 0;
    struct data wr_ps;
    struct ring_buffer *ring_buf_wr = NULL;
    struct ring_buffer *ring_buf_ps = NULL;
    pthread_t write_pid, read_pid, parser_pid;

    buffer_wr = (void *)malloc(BUFFER_SIZE_WR);
    if (!buffer_wr)
    {
        fprintf(stderr, "Failed to buffer_wr malloc memory.\n");
        return -1;
    }
    buffer_ps = (void *)malloc(BUFFER_SIZE_PS);
    if (!buffer_ps)
    {
        fprintf(stderr, "Failed to buffer_ps malloc memory.\n");
        return -1;
    }

    size_wr = BUFFER_SIZE_WR;
    size_ps = BUFFER_SIZE_PS;

    for (i =0;i <2;i++)
    {
        if (pthread_mutex_init(&f_lock[i], NULL) != 0)
        {
            fprintf(stderr, "Failed init mutex,errno:%u,reason:%s\n",
                    errno, strerror(errno));
            return -1;
        }
        buf[i] = (char *)malloc(BUFFER_SIZE);
        if (!buf[i])
        {
            fprintf(stderr, "Failed to buf malloc memory.\n");
            return -1;
        }
    }

    ring_buf_wr = ring_buffer_init(buffer_wr, size_wr, &f_lock[0]);
    if (!ring_buf_wr)
    {
        fprintf(stderr, "Failed to init ring buffer .\n");
        return -1;
    }
    ring_buf_ps = ring_buffer_init(buffer_ps, size_ps, &f_lock[1]);
    if (!ring_buf_ps)
    {
        fprintf(stderr, "Failed to init ring buffer .\n");
        return -1;
    }

    wr_ps.ring_buf_wr = ring_buf_wr;
    wr_ps.ring_buf_ps = ring_buf_ps;

    printf("multi thread test ....\n");
    //采集数据
    pthread_create(&read_pid, NULL, read_proc, (void *)&wr_ps);
    //写入磁盘
    pthread_create(&write_pid, NULL, write_proc, (void *)ring_buf_wr);
    //Data analysis
    pthread_create(&parser_pid, NULL, parser_proc, (void *)ring_buf_ps);

    while(if_runthread)
    {
        char str[20]={0};
        fgets(str, 20, stdin);
        if(strncmp(str,"exit", 4)==0)
         {
            if_runthread=0;
            usleep(1000000);
            break;
        }
        else
        {
            continue;
        }
    }
    //回收资源
    pthread_join(read_pid, NULL);
    pthread_join(write_pid, NULL);
    pthread_join(parser_pid, NULL);
    ring_buffer_free(ring_buf_wr);
    ring_buffer_free(ring_buf_ps);

    for (i =0;i <2;i++)
    {
        pthread_mutex_destroy(&f_lock[i]);
        if(buf[i] != NULL)
        {
            free(buf[i]);
            buf[i] = NULL;
        }
    }
    printf("free success\n");
    return 0;
}

