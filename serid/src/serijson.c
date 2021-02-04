#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>
#include "infra.h"
#include "serijson.h"


char *seri_dev_modestr[DEV_MODE_MAX]=
{
   	[DEV_MODE_ERROR]    =  "na",
	[DEV_MODE_COOR]    =   "coor",
    [DEV_MODE_SENSOR]  =   "sensor",
    [DEV_MODE_SWITCH]  =   "switch",
    [DEV_MODE_OTHER]   =   "other",
};


char *seri_tlv_tagstr[TLV_MAX]={
	[0]                     = "ERROR",
	[TLV_DO_LIGHT]          = "DO_LIGHT",
    [TLV_GET_KEY]           = "GET_KEY",
    [TLV_GET_TEMPER]        = "GET_TEMPER",
    [TLV_MO_GET_TEMPHUMI]   = "GET_HUMI",
    [TLV_RESP_DO_LIGHT]     = "MO_LIGHT",
    [TLV_RESP_GET_KEY]      = "MO_KEY",
    [TLV_RESP_GET_TEMPER]   = "MO_TEMPER",
    [TLV_RESP_GET_HUMI]     = "MO_HUMI",
    [TLV_SET_ADDR]          = "SET_ADDR",
    [TLV_MO_DISTANCE]       = "MO_DISTANCE",
    [TLV_REQ_DISCOVE]       = "REQ_DISTANCE",
    [TLV_RESP_DISCOVE]      = "RESP_DISTANCE",

    [TLV_REQ_SWITCH_STATE]  = "REQ_SWITCH_STATE",
    [TLV_RESP_SWITCH_STATE] = "RESP_SWITCH_STATE",
    [TLV_REQ_DO_SWITCH]     = "REQ_DO_SWITCH",
    [TLV_RESP_DO_SWITCH]    = "RESP_DO_SWITCH",
    [TLV_MO_GET_TEMPHUMI]   = "MO_GET_TEMPHUMI",
    [TLV_MO_DEVNAME]        = "MO_DEVNAME",
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
cJSON * seri2json(IN uchar *aucbuf, IN int msglen,
                     INOUT int *premainlen,
                      INOUT ushort *coor_id)
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
        /* 协调器的消息暂不处理 */
        return NULL;
    }

    *coor_id = pstmsg->addr3;

    ustmp  =  pstmsg->addr2 >> 8;   /* 转换字节序 */
    ustmp |= (pstmsg->addr2 << 8) & 0xff00;
    pstmsg->addr2 = ustmp;

    //strbuf
    root =  cJSON_CreateObject();

    /* json 填充设备基础信息 */
    sprintf(print_strbuf,"%05d", pstmsg->addr2);
    cJSON_AddItemToObject(root, "devid",    cJSON_CreateString(print_strbuf));
    sprintf(print_strbuf,"%05d", pstmsg->addr3);
    cJSON_AddItemToObject(root, "coorid",    cJSON_CreateString(print_strbuf));
    cJSON_AddItemToObject(root, "devrssi",  cJSON_CreateNumber(pstmsg->rssi));
    cJSON_AddItemToObject(root, "devrtype", cJSON_CreateString(seri_dev_modestr[pstmsg->devtype]));


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
                    syslog(LOG_WARNING, "temper or humi is  invalid %d %d\n",psttemper->temper, psttemper->humi);
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
        /* 整理TLV的信息，以json形式写入 */
        //sprintf(print_strbuf,"%s", seri_tlv_tagstr[pstmsgtlvhead->type]);
        //cJSON_AddItemToObject(item, "tag", cJSON_CreateString(print_strbuf));
        cJSON_AddItemToObject(item, "tag", cJSON_CreateNumber(pstmsgtlvhead->type));
        cJSON_AddItemToObject(item, "len", cJSON_CreateNumber(pstmsgtlvhead->len));
        print_strbuf[0] = '\0';
        strcat(print_strbuf, "0x");
        for (int i = 0; i < pstmsgtlvhead->len; i++)
        {
            sprintf(print_strbuf + 2 + 2*i,"%02x", strbuf[i]);
        }
        print_strbuf[pstmsgtlvhead->len*2 +2] = '\0';


        cJSON_AddItemToObject(item, "value", cJSON_CreateString(print_strbuf));
        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(root, "devtlv", array);
    return root;
}
/*
* @brief
*/
int hextodec(char s[])
{
    int i, m, temp=0, n;
    m = strlen(s);//十六进制是按字符串传进来的，所以要获得字符串长度
    for (i=0; i<m; i++)
    {
        if(s[i]>='A' && s[i]<='F')//十六进制还要判断字符是不是在A-F或者a-f之间
        {
            n = s[i] - 'A' + 10;
        }
        else if(s[i]>='a' && s[i]<='f')
        {
            n= s[i] - 'a' + 10;
        }
        else
        {
            n= s[i] - '0';
        }
        temp=temp*16+n;
    }
    return temp;
}
/*
* @brief
*/
int json_msgproc(   IN uchar *jsonbuf,
                    IN int msglen,
                    INOUT uchar *outbuf,
                    INOUT int *sendlen,
                    INOUT ushort *coorid)
{
    int ret = ERROR_FAILED;
    char *jsonstr = NULL;
    cJSON *json = NULL;
    cJSON *found = NULL;
    cJSON *arrayItem = NULL;
    cJSON *tlvobj = NULL;
    cJSON *devobj = NULL;
    ushort uslen_tmp;
    uchar seribuf[ZIGBEE_MAX_DATALEN];
    int serin = 0;

    SERIAL_RCVMSG_S *pstmsg = NULL;

    int parse_coorid = -1;
    int parse_devid = -1;

    jsonbuf[msglen] = '\0';
    int array_cnt = 0;
    int devtlvn = 0;

    memset(seribuf, 0, ZIGBEE_MAX_DATALEN);

    json = cJSON_Parse(jsonbuf); //获取整个大的句柄
    if (NULL == json)
    {
        printf("cJSON_Parse() failed\n");
        return ret;
    }

    jsonstr = cJSON_Print(json); //这个是可以输出的。为获取的整个json的值
    printf("cJSON_Parse() %s\n", jsonstr);
    arrayItem = cJSON_GetObjectItem(json, "dev"); //获取这个对象成员
    if (NULL == arrayItem)
    {
        printf("not found \"dev\"\n");
        return ret;
    }
    for (array_cnt = 0; array_cnt < 100; array_cnt++)
    {
        /* code */
        devobj = cJSON_GetArrayItem(arrayItem, array_cnt);//因为这个对象是个数组获取，且只有一个元素所以写下标为0获取
        if (NULL == devobj)
        {
            printf("cJSON_GetArrayItem(arrayItem, 0); failed\n");
            break;
        }

        found = cJSON_GetObjectItem(devobj, "coorid");
        if (NULL == found)
        {
            printf("not found \"coorid\"\n");
            break;
        }
        parse_coorid = atoi(found->valuestring);
        *coorid = parse_coorid;
        printf("success get coorid: %s\n",found->valuestring);
        printf("success get coorid: %d\n",parse_coorid);

        found = cJSON_GetObjectItem(devobj, "devid");
        if (NULL == found)
        {
            printf("not found \"coorid\"\n");
            break;
        }
        parse_devid = atoi(found->valuestring);


        /*准备组serid 报文*/


        pstmsg = (SERIAL_RCVMSG_S *)(seribuf + serin);

        pstmsg->sop = 0xFE;
        pstmsg->addr1 = (ushort)parse_devid;
        pstmsg->version = 0x01;

        serin += sizeof(SERIAL_RCVMSG_S);  /*偏移*/

        /*处理tlv部分*/
        arrayItem = cJSON_GetObjectItem(devobj, "devtlv");
        for (devtlvn = 0; devtlvn < 100; devtlvn++)
        {
            uchar tag, len;
            char *valstr, tlv_seristr[10];
            tlvobj = cJSON_GetArrayItem(arrayItem, devtlvn);
            if(NULL == tlvobj)
          break;

            found = cJSON_GetObjectItem(tlvobj, "tag");
            //seribuf[serin ++] = (uchar)((found->valueint >> 8) & (0xff));
            seribuf[serin ++] = (uchar)((found->valueint) & (0xff));

            found = cJSON_GetObjectItem(tlvobj, "len");
            seribuf[serin ++] = (uchar)((found->valueint) & (0xff));

            if (found->valueint > 0)
            {
                int len = found->valueint;
                int i;
                int data;
                uchar *pstr;
                uchar hex[3];
                hex[2] = '\0';
                found = cJSON_GetObjectItem(tlvobj, "value");
                pstr = found->valuestring + 2;
                for (i=0; i < len; i++)
                {
                    memcpy(hex, pstr, 2);

                    data = hextodec(hex);
                    seribuf[serin ++] = (uchar)data;
                    pstr = pstr + 2;
                }
            }
            printf("array_cnt:%d, evtlvn: %d\n", array_cnt, devtlvn);
        }
        pstmsg->len = serin - sizeof(SERIAL_RCVMSG_S);/*数据长度*/
        uslen_tmp  =  pstmsg->len >> 8;   /*转换字节序*/
        uslen_tmp |= (pstmsg->len << 8) & 0xff00;
        pstmsg->len = uslen_tmp;


    }
    /*判断填充条件*/
    if((0 < serin) && (serin < ZIGBEE_MAX_DATALEN))
    {
        memcpy(outbuf, seribuf, serin);
        *sendlen = serin;
        printf("send data:");
        for(int i=0; i < serin; i++)
        {
            printf("%02x ", seribuf[i]);
        }
        printf("\n");
        ret = ERROR_SUCCESS;
    }

    return ret;
}
//int main(int argn, char **argc)
int seri_msgproc(char* in, int in_len, char* out, ushort *coor_id)
{

    uchar *buf = (uchar *)in;
    char a = -2;
    uchar b = 0xfe;
    int remain_len = in_len;
    cJSON * ret_json  = NULL;
    cJSON * root =  cJSON_CreateObject();
    cJSON * array = cJSON_CreateArray();
    cJSON * next =  cJSON_CreateObject();
    int ret = ERROR_SUCCESS;
    ushort id,old_id  = 0;
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
        ret_json = seri2json(&buf[in_len - remain_len], remain_len, &remain_len, &id);
        if(NULL != ret_json)
        {
            cJSON_AddItemToArray(array, ret_json);
        }

        if (old_id != id)
        {
            syslog(LOG_WARNING, "Coor id is no same: %u %u\n", old_id, id);
        }
        old_id = id;

    }
    *coor_id = old_id;
    cJSON_AddItemToObject(root, "dev", array);

    strcat(out, cJSON_Print(root));
    if (NULL != root){
        cJSON_Delete(root);
        ret = ERROR_SUCCESS;
    }
    else
    {
        ret = ERROR_FAILED;
    }
    return ret;
}
/*
* @brief 原供python调用接口  已废弃
*/
int seri_to_json(char* in, int in_len, char* out, ushort *coor_id)
{
#if 0
    uchar *buf = (uchar *)in;
    char a = -2;
    uchar b = 0xfe;
    int remain_len = in_len;
    cJSON * ret_json  = NULL;
    cJSON * root =  cJSON_CreateObject();
    cJSON * array = cJSON_CreateArray();
    cJSON * next =  cJSON_CreateObject();
    int ret = ERROR_SUCCESS;
    ushort id,old_id  = 0;
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
        ret_json = seri_msgproc(&buf[in_len - remain_len], remain_len, &remain_len, &id);
        if(NULL != ret_json)
        {
            cJSON_AddItemToArray(array, ret_json);
        }

        if (old_id != id)
        {
            syslog(LOG_WARNING, "Coor id is no same: %u %u\n", old_id, id);
        }
        old_id = id;

    }
    *coor_id = old_id;
    cJSON_AddItemToObject(root, "dev", array);

    strcat(out, cJSON_Print(root));
    if (NULL != root){
        cJSON_Delete(root);
        ret = ERROR_SUCCESS;
    }
    else
    {
        ret = ERROR_FAILED;
    }
    return ret;
#endif
    return 0;
}