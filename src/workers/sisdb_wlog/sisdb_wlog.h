#ifndef _SISDB_WLOG_H
#define _SISDB_WLOG_H

#include "sis_method.h"
	
// 四个时间尺度的数据表 同名字段会传递只能由大到小
// 'T'  TICK数据 4-8 不覆盖 有就追加数据
// 'M'  分钟数据 4 同值最新覆盖
// 'D'  日数据 4 同值最新覆盖

// 根据SDB的时间定义来确定是什么周期
// # 一个是KEY 一个是属性 FIELD
// # KEY 有自己的属性 保留小数点 dot 默认为 2 除非特别设置
// # FIELD 为一个map表 name 数据类型（I:整数,F:浮点数,P:转换价格,J:JSON,C:定长字符串,S:不定长字符串,M:结构体)
// set SH600600 {"name":"aaaa","dot":2,"market":"SH"} 数据直接写入SH600600键值中
// set SH600600.stk_snapshot {"open":1200,"newp":1300,"volume":10000} 写入
// getkey SH* 获取所有SH开头的 code
// sub SH600600,SH600601.stk_snapshot,stk_trancat 订阅两只股票的两类数据
// sub *.* 订阅所有股票的所有数据
// 只有sub和getkey有模糊匹配功能
// update SH600600.money {search:{sno :3, name:sss}, val:{...}} 大括号表示并且的关系
// update SH600600.money {search:[{sno :3}, {name:sss}], val:{...}} 方括号表示或的关系
// del SH600600.money {search:{sno :[1,3]}} 大括号表示并且的关系 取sno为1..3的数据 (1,3) 不包含 1,3
// del SH600600.money {search:[{sno :[1,3]}, {sno:[5,8]}]} 方括号表示或的关系 取1..3 和 5..8的数据
// 取边界外的数据不支持
// 最多的方法是取一段时间的数据

typedef struct s_sisdb_wlog_cxt
{
	// int  status;
	// int  cursor;   // 游标

	// s_sis_double_list *inputs;   // 传入的数据列表

	// // s_sis_struct_list *outputs;  // s_factor_output 输出列表

	// int  field;    // 字段
	// int  output;   // 输出类型
	// int  period;   // 12	
	// int  merges;   // 多少个数据合并。默认为1
	// int  min_count; // 最小数据量

	// int ago_level;
	// double ago_drift; 
	// double ago_speed; 
	
	// s_factor_init   init_info; // 初始化的信息作用是 必须大于均量才会生效
	
}s_sisdb_wlog_cxt;

// bget 二进制获取 同种结构的单一品种数据 key+db
// bset 二进制写入 同种结构的单一品种数据 key+db
// bsub 二进制订阅 一次一条或多条数据多种数据 key+dbs
// 
// aget 数组获取 同种结构的单一品种数据 key+db
// aset 数组写入 同种结构的单一品种数据 key+db
// asub 数组订阅 一次一条或多条数据多种数据 key+dbs 

// get  字符串获取 
// set  字符串写入 

// jget JSON类型获取 
// jset JSON类型写入 

// lget 列表类型获取 
// lset 列表类型写入 

// iget 整数获取 
// iset 整数写入 

// fget 浮点数获取 
// fset 浮点数写入 

bool  sisdb_wlog_init(void *, void *);
void  sisdb_wlog_work_init(void *);
void  sisdb_wlog_working(void *);
void  sisdb_wlog_work_uninit(void *);
void  sisdb_wlog_uninit(void *);
void  sisdb_wlog_method_init(void *);
void  sisdb_wlog_method_uninit(void *);

// 从log中获取数据并回写
int cmd_sisdb_wlog_read(void *worker_, void *argv_);
// 接收数据并顺序写入log
int cmd_sisdb_wlog_write(void *worker_, void *argv_);
// 数据全部处理完后清理内存
int cmd_sisdb_wlog_clear(void *worker_, void *argv_);


#endif