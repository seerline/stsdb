﻿#ifndef _SIS_LIST_H
#define _SIS_LIST_H

#include "sis_core.h"
#include "sis_str.h"
#include "sis_sds.h"

#include "sis_malloc.h"

// 定义一个结构体链表
// 定义一个指针读写链表
// 定义一个字符串读写链表

///////////////////////////////////////////////////////////////////////////
//------------------------s_sis_struct_list ---------------------------------//
//  固定长度的列表
//////////////////////////////////////////////////////////////////////////


typedef struct s_sis_struct_list {
	int		     maxcount; // 总数
	int		     count;    // 当前个数
	int		     start;    // 开始位置 优化limit引起的频繁内存移动
	int          len;      // 每条记录的长度
	void        *buffer;   // 必须是mallco申请的char*类型
} s_sis_struct_list;

#define STRUCT_LIST_STEP_ROW 256  //默认增加的记录数

s_sis_struct_list *sis_struct_list_create(int len_, void *in_, int inlen_); //len_为每条记录长度
void sis_struct_list_destroy(s_sis_struct_list *list_);
void sis_struct_list_clear(s_sis_struct_list *list_);

int sis_struct_list_push(s_sis_struct_list *, void *in_);
int sis_struct_list_insert(s_sis_struct_list *, int index_, void *in_);
int sis_struct_list_update(s_sis_struct_list *, int index_, void *in_);
void *sis_struct_list_first(s_sis_struct_list *);
void *sis_struct_list_last(s_sis_struct_list *);
void *sis_struct_list_get(s_sis_struct_list *, int index_);
void *sis_struct_list_next(s_sis_struct_list *list_, void *);

void struct_list_set_size(s_sis_struct_list *list_, int len_);
int sis_struct_list_set(s_sis_struct_list *, void *in_, int inlen_);

void sis_struct_list_limit(s_sis_struct_list *, int limit_);
int sis_struct_list_clone(s_sis_struct_list *src_, s_sis_struct_list *dst_);
int sis_struct_list_delete(s_sis_struct_list *src_, int start_, int count_);
int sis_struct_list_pack(s_sis_struct_list *list_);

// 获取指针的位置编号
// int sis_struct_list_pto_recno(s_sis_struct_list *list_,void *);

///////////////////////////////////////////////////////////////////////////
//------------------------s_pointer_list --------------------------------//
//  存储指针的列表,记录长度为sizeof(char *)
///////////////////////////////////////////////////////////////////////////
#define POINTER_LIST_STEP_ROW 1024  //默认增加的记录数

typedef struct s_sis_pointer_list {
	int		     maxcount; // 总数
	int		     count;    // 当前个数
	int          len;      // 每条记录的长度
	void        *buffer;   // 必须是mallco申请的char*类型
	void(*free)(void *);   // ==NULL 不释放对应内存
} s_sis_pointer_list;

s_sis_pointer_list *sis_pointer_list_create(); 

void sis_pointer_list_destroy(s_sis_pointer_list *list_);
void sis_pointer_list_clear(s_sis_pointer_list *list_);

int sis_pointer_list_push(s_sis_pointer_list *, void *in_);
int sis_pointer_list_update(s_sis_pointer_list *, int index_, void *in_);
int sis_pointer_list_insert(s_sis_pointer_list *, int index_, void *in_);

void *sis_pointer_list_get(s_sis_pointer_list *, int index_);
void *sis_pointer_list_first(s_sis_pointer_list *);

int sis_pointer_list_delete(s_sis_pointer_list *src_, int start_, int count_);

int sis_pointer_list_indexof(s_sis_pointer_list *list_, void *in_);
int sis_pointer_list_find_and_update(s_sis_pointer_list *, void *finder_, void *in_);
int sis_pointer_list_find_and_delete(s_sis_pointer_list *list_, void *finder_);

///////////////////////////////////////////////////////////////////////////
//------------------------s_sis_string_list --------------------------------//
//  存储不定长字符串的列表，
///////////////////////////////////////////////////////////////////////////
#define STRING_LIST_RD  1
#define STRING_LIST_WR  2
void sis_free_call(void *p);

typedef struct s_sis_string_list {
	int    permissions;     //权限
	char*  m_ptr_r;         // 保存的只读字符串
	s_sis_pointer_list *strlist; //存储指针列表 --free为空只读 不为空可写
} s_sis_string_list;

s_sis_string_list *sis_string_list_create_r(); //只读
s_sis_string_list *sis_string_list_create_w(); //读写
void sis_string_list_destroy(void *list_);
void sis_string_list_clear(s_sis_string_list *list_);

// read & write
int sis_string_list_load(s_sis_string_list *list_, const char *in_, size_t inlen_, const char *sign);
const char *sis_string_list_get(s_sis_string_list *list_, int index_);
int sis_string_list_getsize(s_sis_string_list *list_);

s_sis_sds sis_string_list_sds(s_sis_string_list *list_);

int sis_string_list_clone(
	s_sis_string_list *src_, 
	s_sis_string_list *des_);
int sis_string_list_merge(
	s_sis_string_list *list_, 
	s_sis_string_list *other_); 
int sis_string_list_across(
	s_sis_string_list *list_, 
	s_sis_string_list *other_);

int sis_string_list_indexof(s_sis_string_list *list_, const char *in_);
int sis_string_list_indexofcase(s_sis_string_list *list_, const char *in_);

int sis_string_list_update(s_sis_string_list *list_, int index_, const char *in_, size_t inlen);
//比较字符串地址并修改，字符串比较应该使用string_list_indexof&sis_string_sis__update
int sis_string_list_find_and_update(s_sis_string_list *list_, char *finder_, const char *in_, size_t inlen);
int sis_string_list_insert(s_sis_string_list *list_, int index_, const char *in_, size_t inlen);
int sis_string_list_push(s_sis_string_list *list_, const char *in_, size_t inlen);
int sis_string_list_push_only(s_sis_string_list *list_, const char *in_, size_t inlen);

void sis_string_list_limit(s_sis_string_list *list_, int limit_);

int sis_string_list_delete(s_sis_string_list *list_, int index_);
//比较字符串地址并删除，字符串比较应该使用string_list_indexof&sis_string_lsis_delete
int sis_string_list_find_and_delete(s_sis_string_list *list_, const char *finder_);



#endif //_SIS_LIST_H