#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "infra.h"
#include "list.h"
#include "coor.h"

SL_HEAD_S coorhead;

/**
*  @功能描述: fd节点初始化
*/
API void coorfd_init()
{
	sl_init(&coorhead);
}
/*
*  @添加节点
*/
API void coor_add(int coorid,
				  int fd)
{
	coor_node_t *node = NULL;
    node = coor_find(coorid);
    if(NULL != node)
    {
        node->fd = fd;
    }
    else
    {
        node = (coor_node_t *)malloc(sizeof(coor_node_t));
        if(NULL != node)
        {
            node->fd = fd;
        }
    }
}
/*
*  @查找节点
*/
API coor_node_t *coor_find(int coorid)
{

	coor_node_t *node = NULL;
	SL_NODE_S *pstnode = NULL;
	SL_NODE_S *pstnext = NULL;
	SL_FOREACH_SAFE(&coorhead,pstnode,pstnext){

		node = SL_ENTRY(pstnode,coor_node_t,stnode);

		if(coorid == node->coorid)
		{
			return node;
		}
	}
	return NULL;
}
API void coor_del(int coorid)
{

	coor_node_t *node = NULL;
	SL_NODE_S *pstnode = NULL;
	SL_NODE_S *pstnext = NULL;
	SL_FOREACH_SAFE(&coorhead,pstnode,pstnext){

		node = SL_ENTRY(pstnode,coor_node_t,stnode);
		if(coorid == node->coorid)
        {
			free(node);
			break;
		}
	}
}
API void coor_delall(void)
{

	coor_node_t *node = NULL;
	SL_NODE_S *pstnode = NULL;
	SL_NODE_S *pstnext = NULL;
	SL_FOREACH_SAFE(&coorhead,pstnode,pstnext){
		node = SL_ENTRY(pstnode, coor_node_t, stnode);
		free(node);
	}
}

API void coor_printall(char *pstr)
{

	coor_node_t *node = NULL;
	SL_NODE_S *pstnode = NULL;
	SL_NODE_S *pstnext = NULL;

    printf("/****************/\n");
	SL_FOREACH_SAFE(&coorhead,pstnode,pstnext)
    {
		node = SL_ENTRY(pstnode, coor_node_t, stnode);
        printf("/****************/\n");
        printf("coor id: 0X%04X\n",(ushort)node->coorid);
        printf("fd : %d\n",node->fd);
	}
}



#ifdef __cplusplus
}
#endif
