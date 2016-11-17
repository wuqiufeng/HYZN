/**
 * @file ring_buffer.h
 * @brief
 * @author fu_hongzhu
 * @version
 * @date 2016-11-08
 */


#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

//判断x是否是2的次方
#define is_power_of_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))
//取a和b中最小值
#define min(a, b) (((a) < (b)) ? (a) : (b))

struct ring_buffer
{
    void        *buffer;        //缓冲区
    uint32_t    size;           //缓冲区大小
    uint32_t    in;             //写指针下标
    uint32_t    out;            //读指针下标
    pthread_mutex_t *f_lock;    //互斥锁
};

struct data
{
    struct ring_buffer *ring_buf_wr;
    struct ring_buffer *ring_buf_ps;
};

/* -------------------------------------------*/
/**
 * @brief  初始化缓冲区
 *
 * @param buffer
 * @param size
 * @param f_lock
 *
 * @returns   
 */
/* -------------------------------------------*/
struct ring_buffer* ring_buffer_init(void *buffer, uint32_t size, pthread_mutex_t *f_lock);



/* -------------------------------------------*/
/**
 * @brief  释放缓冲区
 *
 * @param ring_buf
 */
/* -------------------------------------------*/
void ring_buffer_free(struct ring_buffer *ring_buf);


/* -------------------------------------------*/
/**
 * @brief  缓冲区长度
 *
 * @param ring_buf
 *
 * @returns   
 */
/* -------------------------------------------*/
uint32_t __ring_buffer_len(const struct ring_buffer* ring_buf);


/* -------------------------------------------*/
/**
 * @brief  从缓冲区中取数据
 *
 * @param ring_buf
 * @param buffer
 * @param size
 *
 * @returns   
 */
/* -------------------------------------------*/
uint32_t __ring_buffer_get(struct ring_buffer* ring_buf, void *buffer, uint32_t size);


/* -------------------------------------------*/
/**
 * @brief  向缓冲区中存放数据
 *
 * @param ring_buf
 * @param buffer
 * @param size
 *
 * @returns   
 */
/* -------------------------------------------*/
uint32_t __ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size);


/* -------------------------------------------*/
/**
 * @brief  ring_buffer_len 
 *
 * @param ring_buf
 *
 * @returns   
 */
/* -------------------------------------------*/
uint32_t ring_buffer_len(const struct ring_buffer *ring_buf);


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
uint32_t ring_buffer_get(struct ring_buffer *ring_buf, void *buffer, uint32_t size);


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
uint32_t ring_buffer_put(struct ring_buffer *ring_buf, void *buffer, uint32_t size);

#endif
