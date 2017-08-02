
#include "amlstreaminfo.h"
#include "amlvideoinfo.h"
#include "amlaudioinfo.h"

//media stream info class ,in clude audio and video
//typedef int (*AmlCallBackFunc)(void *data);

int amlStreamInfoWriteHeader(AmlStreamInfo *info, codec_para_t *pcodec)
{
	GstMapInfo map;
    if(NULL == info->configdata){
        GST_WARNING("configdata is null");
        return 0;
    }
    gst_buffer_map(info->configdata, &map, GST_MAP_READ);
    guint8 *configbuf = map.data;
    gint configsize = map.size;
    if(configbuf && (configsize > 0)){
        codec_write(pcodec, configbuf, configsize);
    }
    gst_buffer_unmap(info->configdata, &map);
    return 0;
}

AmlStreamInfo *createStreamInfo(gint size)
{
    AmlStreamInfo *info = g_malloc(size);
    info->init = NULL;
    info->writeheader = amlStreamInfoWriteHeader;
    info->add_startcode = NULL;
    info->finalize = amlStreamInfoFinalize;
    info->configdata = NULL;
    return info;
}

void amlStreamInfoFinalize(AmlStreamInfo *info)
{
    if (info->configdata) {
        GST_WARNING("Configdata=%p", info->configdata);
        gst_buffer_unref(info->configdata);
    }
	
    if(info){
        g_free(info);
    }
}
AmlStreamInfo *amlStreamInfoInterface(gchar *format, AmlStreamInfoPool *amlStreamInfoPool)
{
    AmlStreamInfoPool *p  = amlStreamInfoPool; 
    AmlStreamInfo *info = NULL;
    int i = 0; 		
    while(p&&p->name){
        if(!strcmp(p->name, format)){
            if(p->newStreamInfo){
                info = p->newStreamInfo();
            }
            break;
        }
        p++;
    }
    return info;
}

int amlCodecWrite(codec_para_t *pcodec, void *data, int size)
{
    int written = 0;
    int total = 0;
    int retry = 0;
    while (size > 0 && retry < 10) {
        written = codec_write(pcodec, data, size);
        if (written >= 0) {
            size -= written;
            data += written;
            total += written;
            retry = 0;
        } else if (errno == EAGAIN || errno == EINTR) {
            retry++;
            usleep(20000);
        } else {
            break;
        }
    }
    return total;

}
