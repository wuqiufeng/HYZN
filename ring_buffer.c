/**
 * @file ring_buffer.c
 * @brief  ring_bufferAPI封装包裹
 * @author fu_hongzhu
 * @version 
 * @date 2016-11-07
 */

#include "ring_buffer.h"


/* -------------------------------------------*/
/**
 * @brief  初始化缓冲区
 *
 * @param buffer    缓冲区首地址
 * @param size      缓冲区长度
 * @param f_lock     
 *
 * @returns   
 */
/* -------------------------------------------*/
struct ring_buffer* ring_buffer_init(void *buffer, uint32_t size, pthread_mutex_t *f_lock)
{  
    assert(buffer);  
    struct ring_buffer *ring_buf = NULL;  

    if (!is_power_of_2(size))
    {
        fprintf(stderr,"size must be power of 2.\n");
        return ring_buf;
    }

    ring_buf = (struct ring_buffer *)malloc(sizeof(struct ring_buffer));  

    if (!ring_buf)
    {
        fprintf(stderr,"Failed to malloc memory,errno:%u,reason:%s",
                errno, strerror(errno));
        return ring_buf;
    }

    memset(ring_buf, 0, sizeof(struct ring_buffer));  
    ring_buf->buffer = buffer;  
    ring_buf->size = size;  
    ring_buf->in = 0;  
    ring_buf->out = 0;  
    ring_buf->f_lock = f_lock;  
    return ring_buf;  
}  



/* -------------------------------------------*/
/*
 * @brief  释放缓冲区
 *
 * @param ring_buf
 */
/* -------------------------------------------*/
void ring_buffer_free(struct ring_buffer *ring_buf)
{
    if (ring_buf)
    {
        if (ring_buf->buffer)
        {
            free(ring_buf->buffer);
            ring_buf->buffer = NULL;

        }
        free(ring_buf);
        ring_buf = NULL;
    }
}

/* -------------------------------------------*/
/**
 * @brief  缓冲区长度
 *
 * @param ring_buf
 *
 * @returns  
 */
/* -------------------------------------------*/
uint32_t __ring_buffer_len(const struct ring_buffer* ring_buf)
{
    return (ring_buf->in - ring_buf->out);
}

/* -------------------------------------------*/
/**
 * @brief  从缓冲区中取数据
 *
 * @param ring_buf
 * @param buffer    
 * @param size
 *
 * @returns   
 *  size 取数据长度     0取数据fail 
 */
/* -------------------------------------------*/
uint32_t __ring_buffer_get(struct ring_buffer* ring_buf, void *buffer, uint32_t size)
{
    assert(ring_buf || buffer);
    uint32_t len = 0;
    //size  = min(size, ring_buf->in - ring_buf->out);
    if (size >(ring_buf->in - ring_buf->out))
    {
        fprintf(stderr, "Failed to  ring_buffer_get.\n");
        return 0;
    }
    /* first get the data from fifo->out until the end of the buffer */
    len = min(size, ring_buf->size - (ring_buf->out & (ring_buf->size - 1)));
    memcpy(buffer, ring_buf->buffer + (ring_buf->out & (ring_buf->size - 1)), len);
    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + len, ring_buf->buffer, size - len);
    ring_buf->out += size;
    return size;
}

/* -------------------------------------------*/
/**
 * @brief  向缓冲区中存数据
 *
 * @param ring_buf
 * @param buffer
 * @param size
 *
 * @returns   
 *  size 向缓冲区存数据长度     0代表存数据fail
 */
/* -------------------------------------------*/
uint32_t __ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    assert(ring_buf || buffer);
    uint32_t len = 0;
    //size = min(size, ring_buf->size - ring_buf->in + ring_buf->out);

    if (size > ring_buf->size - ring_buf->in + ring_buf->out)
    {
        fprintf(stderr, "Fail ring_buffer_put.\n");
        return 0;
    }
    /* first put the data starting from fifo->in to buffer end */
    len  = min(size, ring_buf->size - (ring_buf->in & (ring_buf->size - 1)));
    memcpy(ring_buf->buffer + (ring_buf->in & (ring_buf->size - 1)), buffer, len);
    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(ring_buf->buffer, buffer + len, size - len);
    ring_buf->in += size;
    return size;
}

/* -------------------------------------------*/
/**
 * @brief  ring_buffer_len 
 *
 * @param ring_buf
 *
 * @returns   
 *  len 已存数据的长度
 */
/* -------------------------------------------*/
uint32_t ring_buffer_len(const struct ring_buffer *ring_buf)
{
    uint32_t len = 0;
    pthread_mutex_lock(ring_buf->f_lock);
    len = __ring_buffer_len(ring_buf);
    pthread_mutex_unlock(ring_buf->f_lock);
    return len;
}

/* -------------------------------------------*/
/**
 * @brief  ring_buffer_get 
 *
 * @param ring_buf
 * @param buffer
 * @param size
 *
 * @returns   
 */
/* -------------------------------------------*/
uint32_t ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    pthread_mutex_lock(ring_buf->f_lock);
    ret = __ring_buffer_get(ring_buf, buffer, size);

    //buffer中没有数据
    if (ring_buf->in == ring_buf->out) 
    {
        ring_buf->in = ring_buf->out = 0;
    }
    pthread_mutex_unlock(ring_buf->f_lock);
    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  ring_buffer_put 
 *
 * @param ring_buf
 * @param buffer
 * @param size
 *
 * @returns   
 */
/* -------------------------------------------*/
uint32_t ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size)
{
    uint32_t ret;
    pthread_mutex_lock(ring_buf->f_lock);
    ret = __ring_buffer_put(ring_buf, buffer, size);
    pthread_mutex_unlock(ring_buf->f_lock);
    return ret;
}
