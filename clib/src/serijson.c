#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "serijson.h"

char *g_pcdevmodestr[DEV_MODE_MAX]=
{
	[DEV_MODE_COOR]    =   "coor",
    [DEV_MODE_SENSOR]  = "sensor",
    [DEV_MODE_SWITCH]  = "switch",
    [DEV_MODE_OTHER]   =  "other",
};


char *shelpstr[TLV_MAX]={
	[0]            ="ERROR",
	[TLV_DO_LIGHT] = "DO_LIGHT",
    [TLV_GET_KEY] = "GET_KEY",
    [TLV_GET_TEMPER] = "GET_TEMPER",
    [TLV_MO_GET_TEMPHUMI] = "GET_HUMI",

    [TLV_RESP_DO_LIGHT] ="MO_LIGHT",
    [TLV_RESP_GET_KEY] = "MO_KEY",
    [TLV_RESP_GET_TEMPER] = "MO_TEMPER",
    [TLV_RESP_GET_HUMI] = "MO_HUMI",
    [TLV_SET_ADDR] = "SET_ADDR",
    [TLV_MO_DISTANCE] = "MO_DISTANCE",
    [TLV_REQ_DISCOVE] = "REQ_DISTANCE",
    [TLV_RESP_DISCOVE] = "RESP_DISTANCE",

    [TLV_REQ_SWITCH_STATE] = "REQ_SWITCH_STATE",
    [TLV_RESP_SWITCH_STATE] ="RESP_SWITCH_STATE",
    [TLV_REQ_DO_SWITCH] ="REQ_DO_SWITCH",
    [TLV_RESP_DO_SWITCH] = "RESP_DO_SWITCH",
    [TLV_MO_GET_TEMPHUMI] ="MO_GET_TEMPHUMI",
    [TLV_MO_DEVNAME] ="MO_DEVNAME",
};


//int main(int argn, char **argc)
int seri_to_json(char* in, int in_len, char* out)
{
    cJSON * root =  cJSON_CreateObject();
    cJSON * item =  cJSON_CreateObject();
    cJSON * next =  cJSON_CreateObject();

    cJSON_AddItemToObject(root, "rc", cJSON_CreateNumber(0));//根节点下添加
    cJSON_AddItemToObject(root, "operation", cJSON_CreateString("CALL"));
    cJSON_AddItemToObject(root, "service", cJSON_CreateString("telephone"));
    cJSON_AddItemToObject(root, "text", cJSON_CreateString("打电话给张三"));
    cJSON_AddItemToObject(root, "semantic", item);//root节点下添加semantic节点
    cJSON_AddItemToObject(item, "slots", next);//semantic节点下添加item节点
    cJSON_AddItemToObject(next, "name", cJSON_CreateString("张三"));//添加name节点

    printf("seritojson: ");
    for(int i=0; i<in_len; i++)
    {
         printf("%c", in[i]);
    }
    printf("\n");




    strcat(out, cJSON_Print(root));
    if (NULL != root){
        cJSON_Delete(root);
    }
    return 0;
}


int json_to_seri(char* in, int in_len, char* out)
{
    cJSON * root =  cJSON_CreateObject();
    cJSON * item =  cJSON_CreateObject();
    cJSON * next =  cJSON_CreateObject();

    cJSON_AddItemToObject(root, "rc", cJSON_CreateNumber(0));//根节点下添加
    cJSON_AddItemToObject(root, "operation", cJSON_CreateString("CALL"));
    cJSON_AddItemToObject(root, "service", cJSON_CreateString("telephone"));
    cJSON_AddItemToObject(root, "text", cJSON_CreateString("打电话给张三"));
    cJSON_AddItemToObject(root, "semantic", item);//root节点下添加semantic节点
    cJSON_AddItemToObject(item, "slots", next);//semantic节点下添加item节点
    cJSON_AddItemToObject(next, "name", cJSON_CreateString("张三"));//添加name节点

    printf("seritojson: ");
    for(int i=0; i<in_len; i++)
    {
         printf("%c", in[i]);
    }
    printf("\n");




    strcat(out, cJSON_Print(root));
    if (NULL != root){
        cJSON_Delete(root);
    }
    return 0;
}