﻿
#include <sis_net.io.h>
#include <sis_malloc.h>
#include <sis_net.node.h>

void sis_net_ask_with_chars(s_sis_net_message *netmsg_, 
    char *cmd_, char *key_, char *val_, size_t vlen_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_ASK;
    netmsg_->style |= cmd_ ? SIS_NET_CMD: 0;
    netmsg_->style |= key_ ? SIS_NET_KEY: 0;
    netmsg_->style |= val_ ? SIS_NET_VAL: 0;
    netmsg_->cmd = cmd_ ? sis_sdsnew(cmd_) : NULL;
    netmsg_->key = key_ ? sis_sdsnew(key_) : NULL;
    netmsg_->val = val_ ? sis_sdsnewlen(val_, vlen_) : NULL;
}
void sis_net_ask_with_bytes(s_sis_net_message *netmsg_,
    char *cmd_, char *key_, char *val_, size_t vlen_)
{
    netmsg_->format = SIS_NET_FORMAT_BYTES;
    netmsg_->style = SIS_NET_ASK | SIS_NET_ARGVS;
    netmsg_->style |= cmd_ ? SIS_NET_CMD: 0;
    netmsg_->style |= key_ ? SIS_NET_KEY: 0;
    netmsg_->cmd = cmd_ ? sis_sdsnew(cmd_) : NULL;
    netmsg_->key = key_ ? sis_sdsnew(key_) : NULL;

    s_sis_object *obj = sis_object_create(SIS_OBJECT_SDS, sis_sdsnewlen(val_, vlen_));
	if (!netmsg_->argvs)
	{
		netmsg_->argvs = sis_pointer_list_create();
		netmsg_->argvs->vfree = sis_object_decr;
	}
    else
    {
        sis_pointer_list_clear(netmsg_->argvs);
    }   
    sis_pointer_list_push(netmsg_->argvs, obj);    
}
// void sis_net_ask_with_bytes(s_sis_net_message *netmsg_,
//     char *cmd_, char *key_, char *val_, size_t vlen_)
// {
//     netmsg_->format = SIS_NET_FORMAT_BYTES;
//     netmsg_->style = SIS_NET_ASK;
//     netmsg_->style |= cmd_ ? SIS_NET_CMD: 0;
//     netmsg_->style |= key_ ? SIS_NET_KEY: 0;
//     netmsg_->style |= val_ ? SIS_NET_VAL: 0;
//     netmsg_->cmd = cmd_ ? sis_sdsnew(cmd_) : NULL;
//     netmsg_->key = key_ ? sis_sdsnew(key_) : NULL;
//     netmsg_->val = val_ ? sis_sdsnewlen(val_, vlen_) : NULL;
// }
// 只增加数据 不清理
void sis_net_ask_with_argvs(s_sis_net_message *netmsg_, const char *in_, size_t ilen_)
{
	if (!netmsg_->argvs)
	{
		netmsg_->argvs = sis_pointer_list_create();
		netmsg_->argvs->vfree = sis_object_decr;
	}
    netmsg_->style = SIS_NET_ASK | SIS_NET_ARGVS;
    s_sis_object *obj = sis_object_create(SIS_OBJECT_SDS, sis_sdsnewlen(in_, ilen_));
    sis_pointer_list_push(netmsg_->argvs, obj);
}

void sis_net_ans_with_chars(s_sis_net_message *netmsg_, const char *in_, size_t ilen_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    if (netmsg_->rval)
    {
        sis_sdsfree(netmsg_->rval);
    }
    if (in_ && ilen_ <= 0 )
    {
        ilen_ = sis_strlen((char *)in_);
    }
    netmsg_->rval = in_ ? sis_sdsnewlen(in_, ilen_) : NULL;
    // sis_out_binary("rval:", in_, ilen_);
    netmsg_->rcmd = SIS_NET_ANS_OK;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->style |= netmsg_->rval ? SIS_NET_VAL: 0;
    netmsg_->style |= netmsg_->key ? SIS_NET_KEY: 0;
}
void sis_net_ans_set_key(s_sis_net_message *netmsg_, const char *kname_, const char *sname_)
{
    sis_sdsfree(netmsg_->key);
    if (sname_)
    {
        netmsg_->key =sis_sdsempty();
        netmsg_->key = sis_sdscatfmt(netmsg_->key, "%s.%s", kname_, sname_);
    }
    else
    {
        netmsg_->key = sis_sdsnew(kname_);
    }
}

void sis_net_ans_with_bytes(s_sis_net_message *netmsg_, const char *in_, size_t ilen_)
{
    netmsg_->format = SIS_NET_FORMAT_BYTES;
    netmsg_->rcmd = SIS_NET_ANS_OK;
    netmsg_->style = SIS_NET_RCMD | SIS_NET_ARGVS;
    netmsg_->style |= netmsg_->key ? SIS_NET_KEY: 0;
    // 可能in来源于argvs 所以先拷贝
    s_sis_object *obj = sis_object_create(SIS_OBJECT_SDS, sis_sdsnewlen(in_, ilen_));
	if (!netmsg_->argvs)
	{
		netmsg_->argvs = sis_pointer_list_create();
		netmsg_->argvs->vfree = sis_object_decr;
	}
    else
    {
        sis_pointer_list_clear(netmsg_->argvs);
    }
    sis_pointer_list_push(netmsg_->argvs, obj);
}
void sis_net_ans_with_argvs(s_sis_net_message *netmsg_, const char *in_, size_t ilen_)
{
    netmsg_->format = SIS_NET_FORMAT_BYTES;
    netmsg_->rcmd = SIS_NET_ANS_OK;
    netmsg_->style = SIS_NET_RCMD | SIS_NET_ARGVS;
    netmsg_->style |= netmsg_->key ? SIS_NET_KEY: 0;
	if (!netmsg_->argvs)
	{
		netmsg_->argvs = sis_pointer_list_create();
		netmsg_->argvs->vfree = sis_object_decr;
	}
    s_sis_object *obj = sis_object_create(SIS_OBJECT_SDS, sis_sdsnewlen(in_, ilen_));
    sis_pointer_list_push(netmsg_->argvs, obj);
}

s_sis_sds sis_net_get_argvs(s_sis_net_message *netmsg_, int index)
{
    if (!netmsg_ || !netmsg_->argvs)
    {
        return NULL;
    }
    s_sis_object *obj = sis_pointer_list_get(netmsg_->argvs, index);
    if (!obj)
    {
        return NULL;
    }
    return SIS_OBJ_SDS(obj);
}

void sis_net_ans_with_noreply(s_sis_net_message *netmsg_)
{
    netmsg_->style = SIS_NET_INSIDE;
}

void sis_net_ans_with_int(s_sis_net_message *netmsg_, int in_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    char rint[32];
    sis_lldtoa(in_, rint, 32, 10);
    netmsg_->rval = sis_sdsnew(rint);
    netmsg_->style = SIS_NET_RCMD | SIS_NET_VAL;
    netmsg_->rcmd = SIS_NET_ANS_OK;
}
void sis_net_ans_with_ok(s_sis_net_message *netmsg_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->rcmd = SIS_NET_ANS_OK;
}
void sis_net_ans_with_error(s_sis_net_message *netmsg_, char *rval_, size_t vlen_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->rcmd = SIS_NET_ANS_ERROR;
    netmsg_->rval = rval_ ? sis_sdsnewlen(rval_, vlen_) : NULL;
    netmsg_->style |= netmsg_->rval ? SIS_NET_VAL: 0;
}
void sis_net_ans_with_null(s_sis_net_message *netmsg_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->rcmd = SIS_NET_ANS_NIL;
}
void sis_net_ans_with_sub_start(s_sis_net_message *netmsg_, const char *info_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->rcmd = SIS_NET_ANS_SUB_START;
    netmsg_->rval = info_ ? sis_sdsnew(info_) : NULL;
    netmsg_->style |= netmsg_->rval ? SIS_NET_VAL: 0;
}
void sis_net_ans_with_sub_wait(s_sis_net_message *netmsg_, const char *info_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->rcmd = SIS_NET_ANS_SUB_WAIT;
    netmsg_->rval = info_ ? sis_sdsnew(info_) : NULL;
    netmsg_->style |= netmsg_->rval ? SIS_NET_VAL: 0;
}
void sis_net_ans_with_sub_stop(s_sis_net_message *netmsg_, const char *info_)
{
    netmsg_->format = SIS_NET_FORMAT_CHARS;
    netmsg_->style = SIS_NET_RCMD;
    netmsg_->rcmd = SIS_NET_ANS_SUB_STOP;
    netmsg_->rval = info_ ? sis_sdsnew(info_) : NULL;
    netmsg_->style |= netmsg_->rval ? SIS_NET_VAL: 0;
}

// int sis_net_send_ask(s_sis_net_class *net_, s_sis_net_message *msg)
// {
//     if (!msg->style)
//     {
//         msg->style = SIS_NET_ASK_NORMAL;
//     }
//     int o = sis_net_class_send(net_, msg);
//     return o;
// }

// static struct s_sis_method _sis_net_method_system[] = {
// 	{"auth",   cmd_net_auth,  SIS_NET_METHOD_SYSTEM, ""},
// 	{"sub",    cmd_net_sub,   SIS_NET_METHOD_SYSTEM, ""},
// 	{"unsub",  cmd_net_unsub, SIS_NET_METHOD_SYSTEM, ""}, 
// 	{"pub",    cmd_net_pub,   SIS_NET_METHOD_SYSTEM, ""},  
// };

// s_sis_map_pointer *sis_net_command_create()
// {
// 	int nums = sizeof(_sis_net_method_system) / sizeof(struct s_sis_method);
// 	s_sis_map_pointer *map = sis_map_pointer_create_v(sis_free_call);
// 	for (int i = 0; i < nums; i++)
// 	{
// 		s_sis_method *c = SIS_MALLOC(s_sis_method, c);
// 		memmove(c, &_sis_net_method_system[i], sizeof(s_sis_method));
// 		char key[128];
// 		sis_sprintf(key, 128, "%s.%s", c->style, c->name);
// 		sis_map_pointer_set(map, key, c);
// 	}
// 	return map;
// }
// void sis_net_command_destroy(s_sis_map_pointer *map_)
// {
// 	sis_method_map_destroy(map_);
// }

// void sis_net_command_append(s_sis_net_class *sock_, s_sis_method *method_)
// {
// 	if(!method_)
// 	{
// 		return ;
// 	}
// 	char key[128];
// 	sis_sprintf(key, 128, "%s.%s", method_->style, method_->name);
// 	sis_map_pointer_set(sock_->map_command, key, method_);
// }
// void sis_net_command_remove(s_sis_net_class *sock_, const char *name_, const char *style_)
// {
// 	char key[128];
// 	sis_sprintf(key, 128, "%s.%s", style_, name_);
// 	sis_map_pointer_del(sock_->map_command, key);
// }

// ////////////////////////////////////////////////////////////////////////////////
// // command list
// ////////////////////////////////////////////////////////////////////////////////
// #define SIS_NET_ANS_SHORT_LEN 1024
// bool sis_socket_send_reply_info(s_sis_net_class *sock_, int cid_,const char *in_)
// {
// 	char str[SIS_NET_ANS_SHORT_LEN];
// 	sis_sprintf(str, SIS_NET_ANS_SHORT_LEN, "+%s\r\n", in_);
// 	if (sock_->connect_method == SIS_NET_IO_CONNECT)
// 	{
// 		// return sis_socket_client_send(sock_->client, str, strlen(str));
// 	}
// 	else
// 	{
// 		// return sis_socket_server_send(sock_->server, cid_, str, strlen(str));
// 	}
	
// }
// bool sis_socket_send_reply_error(s_sis_net_class *sock_, int cid_,const char *in_)
// {
// 	char str[SIS_NET_ANS_SHORT_LEN];
// 	sis_sprintf(str, SIS_NET_ANS_SHORT_LEN, "-%s\r\n", in_);
// 	if (sock_->connect_method == SIS_NET_IO_CONNECT)
// 	{
// 		// return  sis_socket_client_send(sock_->client, str, strlen(str));
// 	}
// 	else
// 	{
// 		// return sis_socket_server_send(sock_->server, cid_, str, strlen(str));
// 	}
// }
// bool sis_socket_send_reply_buffer(s_sis_net_class *sock_, int cid_,const char *in_, size_t ilen_)
// {
// 	if (sock_->connect_method == SIS_NET_IO_CONNECT)
// 	{
// 		// return sis_socket_client_send(sock_->client, (char *)in_, ilen_);
// 	}
// 	else
// 	{
// 		// return sis_socket_server_send(sock_->server, cid_, (char *)in_, ilen_);
// 	}
// }
// bool sis_socket_check_auth(s_sis_net_class *sock_, int cid_)
// {
// 	char key[32];
// 	sis_llutoa(sid_, key, 32, 10);
// 	s_sis_net_client *client = sis_map_pointer_get(sock_->sessions, key);
// 	if (!client->auth)
// 	{
// 		sis_socket_send_reply_error(sock_, cid_, "no auth.");
// 		return false;
// 	}
// 	return true;
// }

// int cmd_net_auth(void *sock_, void *mess_)
// {
// 	s_sis_net_class *sock = (s_sis_net_class *)sock_;
// 	s_sis_net_message *mess = (s_sis_net_message *)mess_;

// 	if (sock->cb_auth)
// 	{
// 		if (!sock->cb_auth(sock_, mess->key, mess->argv))
// 		{
// 			sis_socket_send_reply_error(sock, mess->cid, "auth fail.");
// 			return SIS_METHOD_ERROR;
// 		}
// 	}
// 	char key[32];
// 	sis_llutoa(sid_, key, 32, 10);
// 	s_sis_net_client *client = sis_map_pointer_get(sock->sessions, key);
// 	if (client)
// 	{
// 		client->auth = true;
// 	}
// 	sis_socket_send_reply_info(sock, mess->cid, SIS_REPLY_MSG_OK);
// 	return SIS_METHOD_OK;
// }
// int cmd_net_unsub(void *sock_, void *mess_)
// {
// 	s_sis_net_class *sock = (s_sis_net_class *)sock_;
// 	s_sis_net_message *mess = (s_sis_net_message *)mess_;
// 	if(!sis_socket_check_auth(sock, mess->cid))
// 	{
// 		return SIS_METHOD_ERROR;
// 	}

// 	s_sis_struct_list *cids = (s_sis_struct_list *)sis_map_pointer_get(sock->sub_clients, mess->key);
// 	if (!cids)
// 	{
// 		return SIS_METHOD_ERROR;
// 	}
// 	for (int i = 0; i < cids->count; i++)
// 	{
// 		int cid = *(int *)sis_struct_list_get(cids, i);
// 		if (cid == mess->cid)
// 		{
// 			sis_struct_list_delete(cids, i, 1);
// 			sis_socket_send_reply_info(sock, mess->cid, SIS_REPLY_MSG_OK);
// 			return SIS_METHOD_OK;
// 		}
// 	}
// 	sis_socket_send_reply_error(sock, mess->cid, "no find sub key.");
// 	return SIS_METHOD_ERROR;
// }

// int cmd_net_sub(void *sock_, void *mess_)
// {
// 	s_sis_net_class *sock = (s_sis_net_class *)sock_;
// 	s_sis_net_message *mess = (s_sis_net_message *)mess_;
// 	if(!sis_socket_check_auth(sock, mess->cid))
// 	{
// 		return SIS_METHOD_ERROR;
// 	}

// 	s_sis_struct_list *cids = (s_sis_struct_list *)sis_map_pointer_get(sock->sub_clients, mess->key);
// 	if (!cids)
// 	{
// 		cids = sis_struct_list_create(sizeof(int));
// 		sis_map_pointer_set(sock->sub_clients, mess->key, cids);
// 	}
// 	for (int i = 0; i < cids->count; i++)
// 	{
// 		int cid = *(int *)sis_struct_list_get(cids, i);
// 		if (cid == mess->cid)
// 		{
// 			sis_socket_send_reply_error(sock, mess->cid, "already sub.");
// 			return SIS_METHOD_ERROR;
// 		}
// 	}
// 	sis_struct_list_push(cids, &mess->cid);
// 	sis_socket_send_reply_info(sock, mess->cid, SIS_REPLY_MSG_OK);
// 	return SIS_METHOD_OK;
// }
// int cmd_net_pub(void *sock_, void *mess_)
// {
// 	s_sis_net_class *sock = (s_sis_net_class *)sock_;
// 	s_sis_net_message *mess = sis_net_message_clone((s_sis_net_message *)mess_);
// 	if(!sis_socket_check_auth(sock, mess->cid))
// 	{
// 		return SIS_METHOD_ERROR;
// 	}

// 	mess->style = SIS_NET_ANS_ARRAY;
// 	mess->subpub = 1;
// 	s_sis_list_node *newnode = NULL;
// 	if (mess->argv)
// 	{
// 		newnode = sis_sdsnode_create(mess->argv, sis_sdslen(mess->argv));
// 	}
// 	s_sis_list_node *node = mess->argvs;
// 	while (node != NULL)
// 	{
// 		newnode = sis_sdsnode_push_node(newnode, node->value, sis_sdslen((s_sis_sds)node->value));
// 		node = node->next;
// 	};
// 	mess->rlist = newnode;

// 	s_sis_sds out = sock->cb_pack(sock, mess);

// 	if (!out)
// 	{
// 		sis_socket_send_reply_error(sock, mess->cid, "unknown format.");
// 		sis_net_message_destroy(mess);
// 		return 0;
// 	}
// 	// 
// 	sis_socket_send_reply_info(sock, mess->cid, SIS_REPLY_MSG_OK);

// 	int64 oks = 0;
// 	s_sis_struct_list *cids = (s_sis_struct_list *)sis_map_pointer_get(sock->sub_clients, mess->key);
// 	if (cids)
// 	{
// 		for (int i = 0; i < cids->count; i++)
// 		{
// 			int cid = *(int *)sis_struct_list_get(cids, i);
// 			if (sis_socket_send_reply_buffer(sock, cid, out, sis_sdslen(out)))
// 			// if (_sis_net_class_sendto(sock_, cid, out, sis_sdslen(out)))
// 			{
// 				oks++;
// 			}
// 		}
// 	}
// 	printf("publish key =%lld\n",oks);
// 	sis_sdsfree(out);
// 	sis_net_message_destroy(mess);
// 	return (int) oks;	
// }

