#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 #include <arpa/inet.h>
#include <cjson/cJSON.h>
#include "infra.h"
#include "serijson.h"

char *seri_dev_modestr[DEV_MODE_MAX]=
{
	[DEV_MODE_COOR]    =   "coor",
    [DEV_MODE_SENSOR]  = "sensor",
    [DEV_MODE_SWITCH]  = "switch",
    [DEV_MODE_OTHER]   =  "other",
};


char *seri_tlv_tagstr[TLV_MAX]={
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

void seri_msgntoh(SERIAL_RCVMSG_S *pstmsg)
{

    pstmsg->addr1 = ntohs(pstmsg->addr1);
    pstmsg->addr2 = ntohs(pstmsg->addr2);
    pstmsg->addr3 = ntohs(pstmsg->addr3);
    pstmsg->crc =   ntohs(pstmsg->crc);
    pstmsg->len =   ntohs(pstmsg->len);
    return;
}
/*
* @brief
*/
cJSON * seri_msgproc(IN uchar *aucbuf, IN int msglen, INOUT int *premainlen)
{

    uint uiptrn = 0;
    uint uitmp = 0;
    SERIAL_RCVMSG_S *pstmsg = NULL;
    SERIAL_RCVMSG_TLV_HEAD_S *pstmsgtlvhead = NULL;
	uchar strbuf[ZIGBEE_MAX_DATALEN];
    uchar print_strbuf[ZIGBEE_MAX_DATALEN];
    ushort ustmp;
    int datalen;
    cJSON * root = NULL;
    cJSON * array = NULL;

    printf("seri_msgproc %d\n",msglen);
    /*校验长度*/
    if (sizeof(SERIAL_RCVMSG_S) > msglen)
    {
       *premainlen = 0;
        return NULL;
    }
    /*解析头部*/
    pstmsg = (SERIAL_RCVMSG_S *)aucbuf;

    /*字节序转换*/
    seri_msgntoh(pstmsg);

    /*校验长度*/

    if (msglen  < (pstmsg->len + sizeof(SERIAL_RCVMSG_S)))
    {
        printf("pstmsg->len %d\n", pstmsg->len);
        *premainlen = 0;
        return NULL;
    }
    else
    {
        *premainlen = msglen - (pstmsg->len + sizeof(SERIAL_RCVMSG_S));
    }


    /*目的地址不属于自己的消息不处理*/
    if(LOCAL_ADDR != pstmsg->addr1 )
    {
        return NULL;
    }
    else if(LOCAL_COOR == pstmsg->addr2 )
    {
        /* 协调器的消息暂不处理*/
        return NULL;
    }


    ustmp  =  pstmsg->addr2 >> 8;   /*转换字节序*/
    ustmp |= (pstmsg->addr2 << 8) & 0xff00;
    pstmsg->addr2 = ustmp;

    //strbuf
    root =  cJSON_CreateObject();

    cJSON_AddItemToObject(root, "devid", cJSON_CreateNumber(pstmsg->addr2));
    cJSON_AddItemToObject(root, "devrssi", cJSON_CreateNumber(pstmsg->rssi));



    array = cJSON_CreateArray();

    for (uiptrn += sizeof(SERIAL_RCVMSG_S);
         uiptrn < (pstmsg->len + sizeof(SERIAL_RCVMSG_S));
         uiptrn += pstmsgtlvhead->len)
    {
        cJSON * item =  cJSON_CreateObject();
        pstmsgtlvhead = (SERIAL_RCVMSG_TLV_HEAD_S *)&aucbuf[uiptrn];
        uiptrn += sizeof(SERIAL_RCVMSG_TLV_HEAD_S);

        memset(strbuf, 0, ZIGBEE_MAX_DATALEN);
        memcpy(&strbuf,&aucbuf[uiptrn],pstmsgtlvhead->len);
        switch(pstmsgtlvhead->type)
        {

            case TLV_RESP_DISCOVE:
            {
                SERIAL_TLV_RESP_DISCOVE_S *pstrespdiscove;
                pstrespdiscove = (SERIAL_TLV_RESP_DISCOVE_S *)strbuf;
                cJSON_AddItemToObject(root, "devtype", cJSON_CreateString(seri_dev_modestr[pstrespdiscove->devmode]));
                break;
            }
            case TLV_RESP_SWITCH_STATE:
            {
                cJSON_AddItemToObject(root, "switchstate", cJSON_CreateNumber(*(uchar *)strbuf));
                break;
            }
            case TLV_MO_GET_TEMPHUMI:
            {

                SERIAL_TLV_MO_GET_TEMPHUMI *psttemper;
                psttemper = (SERIAL_TLV_MO_GET_TEMPHUMI *)strbuf;
                /*检查合法性*/
                if ((100 <= psttemper->temper) || (100 <= psttemper->humi))
                {
                    printf(" temper or humi is  invalid \n");
                    //zigbee_debug(ZIGBEE_DEBUG_ERROR, " temper or humi is  invalid \n");
                    break;
                }
                cJSON_AddItemToObject(root, "temper", cJSON_CreateNumber(psttemper->temper));
                cJSON_AddItemToObject(root, "humi", cJSON_CreateNumber(psttemper->humi));
                break;
            }

            default:
            {
                break;
            }
        }
        sprintf(print_strbuf,"%s", seri_tlv_tagstr[pstmsgtlvhead->type]);
        cJSON_AddItemToObject(item, "tag", cJSON_CreateString(print_strbuf));
        cJSON_AddItemToObject(item, "len", cJSON_CreateNumber(pstmsgtlvhead->len));
        for (int i = 0; i < pstmsgtlvhead->len; i++)
        {
            sprintf(print_strbuf + 2*i,"%02x", strbuf[i]);
        }
        print_strbuf[pstmsgtlvhead->len*2] = '\0';
        strcat(print_strbuf, " HEX");

        cJSON_AddItemToObject(item, "value", cJSON_CreateString(print_strbuf));

        cJSON_AddItemToArray(array, item);
        cJSON_AddItemToObject(root, "devtlv", array);
    }

    //zigbee_devnode_printall();
    //zigbee_serialsenddiscover();
    return root;
}

//int main(int argn, char **argc)
int seri_to_json(char* in, int in_len, char* out)
{

    uchar *buf = (uchar *)in;
    char a = -2;
    uchar b = 0xfe;
    int remain_len = in_len;
    cJSON * ret_json  = NULL;
    cJSON * root =  cJSON_CreateObject();
    cJSON * array = cJSON_CreateArray();
    cJSON * next =  cJSON_CreateObject();

    printf("seritojson: ");
    for(int i=0; i<in_len; i++)
    {
         printf("%2x ", buf[i]);
    }
    printf("\n");

/*     uchar buf_temp[21] = {
        0xfe,0x01,0x00,0x05,0xde,0xaf,0xee,0xee,0xdd,0xdd,0x23,0x00,0x00,0x00,0x12,0x34,0x0c,0x03,0x01,0x02,0x44
    };
    buf = buf_temp;
    remain_len = in_len = 21; */
    for ( ; 0 < remain_len; )
    {
        ret_json = seri_msgproc(&buf[in_len - remain_len], remain_len, &remain_len);
        if(NULL != ret_json)
        {
            cJSON_AddItemToArray(array, ret_json);
        }
    }
    cJSON_AddItemToObject(root, "dev", array);

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



