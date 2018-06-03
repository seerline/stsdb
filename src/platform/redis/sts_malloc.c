

#include <sts_malloc.h>
#include <string.h>
#include <stdlib.h>

s_memory_node *__memory_first, *__memory_last;

void check_memory_start()
{
    __memory_first = NULL;
    __memory_last = __memory_first;
}
void check_memory_stop()
{
    s_memory_node *node = __memory_first;
    if(node) {
        printf("no free memory:\n"); 
    }
    while(node)
    {
        printf("    func:%s, lines:%d\n",node->info, node->line);
        node = node->next;
    }
}

void check_memory_newnode(void *__p__,int line_,const char *func_)
{
    s_memory_node *__n = (s_memory_node *)__p__; 
    __n->next = NULL; __n->line = line_; 
    memmove(__n->info, func_, MEMORY_INFO_SIZE); __n->info[MEMORY_INFO_SIZE - 1] = 0; 
    if (!__memory_first) { 
        __n->prev = NULL;  
        __memory_first = __n; __memory_last = __memory_first; 
    } else { 
        __n->prev = __memory_last; 
        __memory_last->next = __n; 
        __memory_last = __n; 
    }
}   
void check_memory_freenode(void *__p__)
{   
    s_memory_node *__n = (s_memory_node *)__p__; 
    if(__n->prev) { __n->prev->next = __n->next; } 
    else { __memory_first = __n->next; }  
    if(__n->next)  { __n->next->prev = __n->prev; } 
    else { __memory_last = __n->prev; } 
}

void sts_free(void *__p__)
{
    if (!__p__) return;
    void *p = (char *)__p__ - MEMORY_NODE_SIZE; 
    check_memory_freenode(p); 
    free(p); 
}

#if 1
#include <sts_db_io.h>
int main()
{
    check_memory_start();
    stsdb_open("../conf/stsdb.conf");

    stsdb_close();
    check_memory_stop();
    return 0;
}
int main1()
{
    check_memory_start();

    void *aa = sts_malloc(100);
    memmove(aa,"my is aa\n",10);
    void *bb = sts_calloc(100);
    memmove(bb,"my is bb\n",10);
    void *dd = NULL;
    void *cc = sts_realloc(dd,1000);
    sprintf(cc,"my is cc %p-->%p\n",aa,cc);
    printf(aa);
    printf(bb);
    printf(cc);
    sts_free(cc);
    check_memory_stop();
    return 0;
}
#endif