#ifndef __CLIB_COOR_
#define __CLIB_COOR_


typedef struct st_coor_node
{
	SL_NODE_S stnode;
	int coorid;
	int fd;
}coor_node_t;


API void coorfd_init();
API void coorfd_fini();
API void coor_add(int coorid, int fd);
API coor_node_t *coor_find(int coorid);
API int  coor_findfd(int coorid);
API void coor_del(int coorid);

API void coor_printall(char *pstr);



#endif
