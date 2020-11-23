﻿#include "sis_net.ws.h"

static char __ws_define_hash[] = "                        258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static char __ws_define_ans[] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
static char __ws_define_ask[] = "GET / HTTP/1.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: "; // 24位随机字符
static char __ws_define_eof[] = "\r\n\r\n";
static char __ws_define_key[] = "Sec-WebSocket-Key";

size_t sis_net_ws_get_ask(s_sis_memory *memory_)
{
    sis_memory_clear(memory_);
    sis_memory_cat(memory_, __ws_define_ask, sis_strlen(__ws_define_ask));
    char key[32];
    sis_str_get_id(key, 25);
    sis_memory_cat(memory_, key, 24);
    sis_memory_cat(memory_,  __ws_define_eof, sis_strlen(__ws_define_eof));
    return sis_memory_get_size(memory_);
}

// 得到应答信息
size_t sis_net_ws_get_ans(char *key_, s_sis_memory *memory_)
{
    for (int i = 0; i < 24; i++)
    {
        __ws_define_hash[i] = key_[i];
    }
    char anskey[32];
    shacalc(__ws_define_hash, anskey);
    anskey[28] = 0;
    sis_memory_clear(memory_);
    sis_memory_cat(memory_, __ws_define_ans, sis_strlen(__ws_define_ans));
    sis_memory_cat(memory_, anskey, 28); // 这里不确定几个字符
    sis_memory_cat(memory_,  __ws_define_eof, sis_strlen(__ws_define_eof));
    return sis_memory_get_size(memory_);
}
// 判断是否头完整
int sis_net_ws_head_complete(s_sis_memory *in_)
{
    int size = sis_strsub(sis_memory(in_), __ws_define_eof);
    if (size < 0)
    {
        return -1;
    }
    return size + 4;
}

int sis_net_ws_get_key(s_sis_memory *in_, char *key_)
{
    int set = sis_strsub(sis_memory(in_), __ws_define_key);
    if (set < 0)
    {
        return -1;
    }
    char *ptr = sis_memory(in_) + set + sis_strlen(__ws_define_key);
    while(*ptr==' '||*ptr==':') 
    {
        ptr++;
    }
    char key[32];
    int  count = 0;
    while(count < 24)
    {
        if (*ptr=='\r'||*ptr=='\n')
        {
            break;
        }
        key[count] = *ptr;
        ptr++;
        count++;
    }
    if (count < 24)
    {
        return -2;
    }
    sis_strcpy(key_, 25, key);
    return 0;
}

int sis_net_ws_chk_ans(s_sis_memory *in_)
{
    int set = sis_strsub(sis_memory(in_), "101");
    if (set < 0)
    {
        return -1;
    }   
    return 0;
}

#define MAX_SEND_BUFF (16 * 1024)
// #define MAX_SEND_BUFF (16)
// 浏览器的ws协议不能超过64K数据大小，因此必须拆包，才能发送给浏览器，否则发送失败
// 打包
int sis_net_pack_ws_message(int isstr_, s_sis_memory *in_, s_sis_memory *out_, s_sis_memory_info *info)
{
    size_t insize = sis_memory_get_size(in_);
    if (info) 
    {
        insize += sizeof(s_sis_memory_info);
    }
    int count = insize / MAX_SEND_BUFF + 1;
    sis_memory_clear(out_);
    sis_memory_set_maxsize(out_, insize + count * 10);

    char *inptr = sis_memory(in_);
    for (int i = 0; i < count; i++)
    {
        size_t size = insize > MAX_SEND_BUFF ? MAX_SEND_BUFF : insize;
        s_sis_bits_stream *stream = sis_bits_stream_create((uint8 *)sis_memory(out_), size + 4);
        if (i == count - 1)
        {
            sis_bits_stream_put(stream, 1, 1); // fin
        }
        else
        {
            sis_bits_stream_put(stream, 0, 1); // fin
        }
        sis_bits_stream_put(stream, 0, 3); // rsv
        if (count > 0 && i > 0)
        {
            sis_bits_stream_put(stream, 0, 4); // opcode
        }
        else
        {
            if (!isstr_)
            {
                sis_bits_stream_put(stream, 2, 4); // opcode  1 - text 2 - binary
            }
            else
            {
                sis_bits_stream_put(stream, 1, 4); // opcode  1 - text 2 - binary
            }           
        }
        sis_bits_stream_put(stream, 0, 1); // mask
        // printf("len= %d\n", sis_bits_stream_getbytes(stream));
        if (size < 126)
        {
            sis_bits_stream_put(stream, size, 7);
        }
        else if (size <= 0xFFFF)
        {
            sis_bits_stream_put(stream, 126, 7);
            sis_bits_stream_put(stream, size, 16);
        }
        else
        {
            sis_bits_stream_put(stream, 127, 7);
            sis_bits_stream_put(stream, size, 64);            
        }     
        if (i == count - 1 && info)
        {
            sis_bits_stream_put_buffer(stream, inptr, size - sizeof(s_sis_memory_info));
            sis_bits_stream_put_buffer(stream, (char *)info, sizeof(s_sis_memory_info));
        }
        else
        {
            sis_bits_stream_put_buffer(stream, inptr, size);
        }
        inptr += size;
        insize -= size;

        int cursor = sis_bits_stream_getbytes(stream);
        sis_memory_set_size(out_, cursor);
        sis_memory_move(out_, cursor);
        // printf("1 == [%d : %d] %zu\n",i, cursor, sis_memory_get_size(out_));
        sis_bits_stream_destroy(stream);
    }
    // 回到起点
    sis_memory_jumpto(out_, 0);
    // printf("2 == %zu\n",sis_memory_get_size(out_));
    // sis_out_binary("pack", sis_memory(out_), sis_memory_get_size(out_));

    return 0;
}
// 拆包前判断数据是否完整
int sis_net_unpack_ws_check(s_sis_ws_header *head_, s_sis_memory *in_)
{
    s_sis_bits_stream *stream = sis_bits_stream_create((uint8 *)sis_memory(in_), sis_memory_get_size(in_));
    
    int count = 0;
    int isbreak = true;
    int start = sis_memory_get_address(in_);
    // printf("==1==  %d - %d\n", start, sis_memory_get_size(in_));
    while (1)
    {
        if (sis_memory_get_size(in_) < 2)
        {
            break;
        }
        // sis_out_binary("unpack", sis_memory(in_), 32);//sis_memory_get_size(in_));

        head_->fin = sis_bits_stream_get(stream, 1);
        sis_bits_stream_get(stream, 3);
        if (count == 0)
        {
            head_->opcode = sis_bits_stream_get(stream, 4);
        }
        else
        {
            // 后续包 opcode = 0 无效 但仍然要移动指针
            sis_bits_stream_get(stream, 4);
        }
        head_->mask = sis_bits_stream_get(stream, 1);
        int signlen = sis_bits_stream_get(stream, 7);

        // printf("==2.1== fin = %d  %d\n", head_->fin, head_->length);
        sis_memory_move(in_, 2);
        if (signlen == 126)
        {
            if (sis_memory_get_size(in_) < 2)
            {
                break;
            }    
            head_->length = sis_bits_stream_get(stream, 16);
            sis_memory_move(in_, 2);
        }
        else if (signlen == 127)
        {
            if (sis_memory_get_size(in_) < 8)
            {    break;  }
            head_->length = sis_bits_stream_get(stream, 64);
            sis_memory_move(in_, 8);
        }
        else
        {
            head_->length = signlen;
        }
        
        // printf("==2.2== fin = %d  %d\n", head_->fin, head_->length);
        if (head_->mask == 1)
        {
            if (sis_memory_get_size(in_) < 4)
            {    
                break;
            }
            sis_bits_stream_get(stream, 32);
            sis_memory_move(in_, 4);
        }
        // printf("==2.3== fin = %d  %d\n", head_->fin, sis_memory_get_size(in_));
        if (sis_memory_get_size(in_) < head_->length)
        {
            break;
        }
        sis_bits_stream_move_bytes(stream, head_->length);
        sis_memory_move(in_, head_->length);
        if (head_->fin == 1)
        {
            // 数据包完整 退出
            isbreak = false;
            break;            
        }
        count++;
    }
    sis_bits_stream_destroy(stream);
    // 退回上次的设置
    sis_memory_jumpto(in_, start);
    if (isbreak)
    {
        // LOG(8)("==check 0 ==  %d - %d\n", head_->fin, sis_memory_get_size(in_));
        return 0;
    }
    return 1;
}
// 拆包
int sis_net_unpack_ws_message(s_sis_ws_header *head_, s_sis_memory *in_, s_sis_memory *out_)
{
    sis_memory_clear(out_);
    s_sis_bits_stream *stream = sis_bits_stream_create((uint8 *)sis_memory(in_), sis_memory_get_size(in_));
    
    int count = 0;
    int isbreak = true;
    int start = sis_memory_get_address(in_);
    // printf("==1==  %d - %d\n", start, sis_memory_get_size(in_));
    while (1)
    {
        if (sis_memory_get_size(in_) < 2)
        {
            break;
        }
        // sis_out_binary("unpack", sis_memory(in_), 32);//sis_memory_get_size(in_));

        head_->fin = sis_bits_stream_get(stream, 1);
        sis_bits_stream_get(stream, 3);
        if (count == 0)
        {
            head_->opcode = sis_bits_stream_get(stream, 4);
        }
        else
        {
            // 后续包 opcode = 0 无效 但仍然要移动指针
            sis_bits_stream_get(stream, 4);
        }
        head_->mask = sis_bits_stream_get(stream, 1);
        int signlen = sis_bits_stream_get(stream, 7);

        // printf("==2.1== fin = %d  %d\n", head_->fin, head_->length);
        sis_memory_move(in_, 2);
        if (signlen == 126)
        {
            if (sis_memory_get_size(in_) < 2)
            {
                break;
            }    
            head_->length = sis_bits_stream_get(stream, 16);
            sis_memory_move(in_, 2);
        }
        else if (signlen == 127)
        {
            if (sis_memory_get_size(in_) < 8)
            {    break;}
            head_->length = sis_bits_stream_get(stream, 64);
            sis_memory_move(in_, 8);
        }
        else
        {
            head_->length = signlen;
        }   
        // printf("==2.2== fin = %d  %d\n", head_->fin, head_->length);
        if (head_->mask == 1)
        {
            if (sis_memory_get_size(in_) < 4)
            {    
                break;
            }
            head_->maskkey[0] = sis_bits_stream_get(stream, 8);
            head_->maskkey[1] = sis_bits_stream_get(stream, 8);
            head_->maskkey[2] = sis_bits_stream_get(stream, 8);
            head_->maskkey[3] = sis_bits_stream_get(stream, 8);
            sis_memory_move(in_, 4);
        }
        // printf("==2.3== fin = %d  %d\n", head_->fin, sis_memory_get_size(in_));
        if (sis_memory_get_size(in_) < head_->length)
        {
            break;
        }
        int offset = sis_memory_get_size(out_);
        sis_memory_cat(out_, sis_memory(in_), head_->length);
        if (head_->mask == 1)
        {
            char *ptr = sis_memory(out_) + offset;
            for (int i = 0; i < head_->length; i++)
            {
                ptr[i] = ptr[i] ^ head_->maskkey[i % 4];
            }
        }
        // printf("==2.8==  %d - %d -- stream : %d\n", head_->fin, sis_memory_get_size(in_),
        //         sis_bits_stream_getbytes(stream));

        sis_bits_stream_move_bytes(stream, head_->length);
        sis_memory_move(in_, head_->length);

        // printf("==2.9==  %d -[%d] %d || %zu -> %zu stream : %d\n", head_->fin, count, 
        //         head_->length,
        //         sis_memory_get_size(in_),
        //         sis_memory_get_size(out_),
        //         sis_bits_stream_getbytes(stream));

        if (head_->fin == 1)
        {
            // 数据包完整 退出
            isbreak = false;
            break;            
        }
        count++;
    }
    sis_bits_stream_destroy(stream);
    if (isbreak)
    {
        // LOG(8)("==unpack 0 ==  %d - %d\n", head_->fin, sis_memory_get_size(in_));
        // 没有成功解析就退回上次的设置
        sis_memory_clear(out_);
        sis_memory_jumpto(in_, start);
        return 0;
    }
    else
    {

    }
    
    return 1;
}

int sis_net_pack_ws(s_sis_memory *in_, s_sis_memory_info *info, s_sis_memory *out_)
{
    if (info->is_bytes == 0)
    {
        sis_net_pack_ws_message(1, in_, out_, NULL);
    }
    else
    {
        sis_net_pack_ws_message(0, in_, out_, info);
        // sis_memory_cat(out_, (char *)info, sizeof(s_sis_memory_info));
    }
    return 1;
}
// 需要增加处理多包的能力
int sis_net_unpack_ws(s_sis_memory *in_, s_sis_memory_info *info_, s_sis_memory *out_)
{
    s_sis_ws_header wshead = {0};

    if (!sis_net_unpack_ws_check(&wshead, in_))
    {
        // 没有成功解析 等待下一个数据包
        return 0;
    }
    if (!sis_net_unpack_ws_message(&wshead, in_, out_))
    {
        // 没有成功解析 应该合并数据
        return 0;
    }
    if (wshead.opcode == 1)
    {
        info_->is_bytes = 0;   // 字符串
        info_->is_compress = 0;
        info_->is_crypt = 0;
        info_->is_crc16 = 0;
    }
    else 
    if (wshead.opcode == 2)
    {
        size_t size = sis_memory_get_size(out_);
        memmove(info_, sis_memory(out_) + size - sizeof(s_sis_memory_info),sizeof(s_sis_memory_info));
        if (info_->is_crc16)
        {
            // 这里处理crc16验证 如果不匹配返回错误
        }
        sis_memory_set_size(out_, size - sizeof(s_sis_memory_info));
        // 这里可以判断数据包是否可识别 不能识别就返回 -1  
    }
    else
    {
        LOG(5)("opcode error = %d.\n", wshead.opcode);
        return -1;
    }
    return 1;
}

// int sis_net_unpack_ws(s_sis_memory *in_, s_sis_memory_info *info_, s_sis_memory *out_)
// {
//     s_sis_ws_header wshead = {0};

//     if (!sis_net_unpack_ws_message(&wshead, in_, out_))
//     {
//         // 没有成功解析 应该合并数据
//         return 0;
//     }
//     else
//     {
//         // 其他值表示数据获取完整        
//     }
    
//     if (wshead.opcode == 1)
//     {
//         info_->is_bytes = 0;   // 字符串
//         info_->is_compress = 0;
//         info_->is_crypt = 0;
//         info_->is_crc16 = 0;
//     }
//     else 
//     if (wshead.opcode == 2)
//     {
//         size_t size = sis_memory_get_size(out_);
//         memmove(info_, sis_memory(out_) + size - sizeof(s_sis_memory_info), sizeof(s_sis_memory_info));
//         if (info_->is_crc16)
//         {
//             // 这里处理crc16验证 如果不匹配返回错误
//         }
//         sis_memory_set_size(out_, size - sizeof(s_sis_memory_info));
//         // 这里可以判断数据包是否可识别 不能识别就返回 -1  
//     }
//     else
//     {
//         LOG(5)("opcode error = %d.\n", wshead.opcode);
//         return -1;
//     }
//     return 1;
// }
