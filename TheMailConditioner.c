#include "../TheRequirements_0_1/TheRequirements.h"
struct TheMailConditioner{
    SetupEWB;
    u8 octet;
    struct list_head bind[4][16];
    struct mutex magic[255];
    void(*bindDelete)(void*,struct ExpiryWorkBaseBenchmark);
    void*data;
    struct list_head list;
};
static struct TheMailConditioner*GetPrev(struct ExpiryWorkBase*ewb){
    return (struct TheMailConditioner*) GetExpiryWorkBasePrevious(ewb);
}
static struct mutex Magic[255];
static struct list_head Bind[4][16];
static struct kmem_cache*tmccache;
void CancelTheMailConditioner(struct TheMailConditioner*);
void CancelTheMailConditioner(struct TheMailConditioner*tmc){
    if(!tmc)return;
    if(!LockExpiryWorkBase(tmc->ewb)||!GetExpiryWorkBaseParent(tmc->ewb))return;
    if(tmc->bindDelete)
        tmc->bindDelete(tmc->data,TheBenchmarksExpiryWorkBase(tmc->ewb,false,false));
    struct TheMailConditioner*prev=GetPrev(tmc->ewb);
    mutex_lock(&(prev?prev->magic:Magic)[tmc->octet]);    
    list_del(&tmc->list);
    mutex_unlock(&(prev?prev->magic:Magic)[tmc->octet]);
    void *tempData = tmc->data;
    CancelExpiryWorkBase(tmc->ewb);
    UnlockExpiryWorkBase(tmc->ewb);
    kmem_cache_free(tmccache,tmc);
    if(tempData)kfree(tempData);
}
EXPORT_SYMBOL(CancelTheMailConditioner);
static void AutoDeleteData(void*);
static void AutoDeleteData(void* data) {
    struct TheMailConditioner*tmc=(struct TheMailConditioner*)data;
    if(!tmc)return;
    void*tempData=tmc->data;
    if(tmc->bindDelete)tmc->bindDelete(tempData,TheBenchmarksExpiryWorkBase(tmc->ewb,false,false));
    kmem_cache_free(tmccache,tmc);
    if(tempData)kfree(tempData);
}
bool SetAutoDeleteTheMailConditioner(struct TheMailConditioner*,void(*)(void*,struct ExpiryWorkBaseBenchmark));
bool SetAutoDeleteTheMailConditioner(struct TheMailConditioner*tmc,void(*bindDelete)(void*,struct ExpiryWorkBaseBenchmark)){
    if(!GetExpiryWorkBaseParent(tmc->ewb))return false;
    tmc->bindDelete=bindDelete;
    return true;
}
EXPORT_SYMBOL(SetAutoDeleteTheMailConditioner);
void*GetTheMailConditionerData(struct TheMailConditioner*);
void*GetTheMailConditionerData(struct TheMailConditioner*tmc){
    return GetExpiryWorkBaseParent(tmc->ewb)?tmc->data:NULL;
}
EXPORT_SYMBOL(GetTheMailConditionerData);
bool SetTheMailConditionerData(struct TheMailConditioner*,void*);
bool SetTheMailConditionerData(struct TheMailConditioner*tmc,void*data){
    if(!GetExpiryWorkBaseParent(tmc->ewb))return false;
    tmc->data=data;
    return true;
}
EXPORT_SYMBOL(SetTheMailConditionerData);


struct TheMailConditioner*GetTheMailConditioner(u8*,u8,bool);
struct TheMailConditioner*GetTheMailConditioner(u8*value,u8 size,bool set){
    struct mutex*lastMagic=Magic,*firstMagic=NULL;
    struct list_head(*lastBind)[16]=Bind,*firstList=NULL;
    struct TheMailConditioner*connection=NULL,*tmp=NULL;
    u8 octet,group,slot;
    for(u8 i=0;i<size;i++){
        octet=value[i];
        group=(octet&1)+((octet>>5)>7?2:0);
        slot=octet>>5;
        firstMagic=&lastMagic[octet];
        firstList=&lastBind[group][slot];
        if(!list_empty(firstList))
            list_for_each_entry_safe(connection,tmp,firstList,list)
                    if(connection->octet==octet&&GetExpiryWorkBaseParent(connection->ewb)){
                        if(i==(size-1))return connection;
                        goto next;
                    }
        if(!set)return NULL;
        cond_resched();
        mutex_lock(firstMagic);
        list_for_each_entry_safe(connection,tmp,firstList,list)
            if(connection->octet==octet&&GetExpiryWorkBaseParent(connection->ewb)){
                mutex_unlock(firstMagic);
                if(i==(size-1))return connection;
                goto next;
            }
        connection=kmem_cache_alloc(tmccache,GFP_KERNEL);
        if(!connection){
            mutex_unlock(firstMagic);
            return NULL;
        }
        if(!SetupExpiryWorkBase(&connection->ewb,(i==0)?NULL:list_first_entry(firstList,struct TheMailConditioner,list)->ewb,connection,AutoDeleteData)){
            kmem_cache_free(tmccache,connection);
            mutex_unlock(firstMagic);
            return NULL;
        }
        for(u8 m=0;m<255;m++)
            mutex_init(&connection->magic[m]);
        for(u8 g=0;g<4;g++)
            for(u8 s=0;s<16;s++)
                INIT_LIST_HEAD(&connection->bind[g][s]);
        INIT_LIST_HEAD(&connection->list);
        connection->octet=octet;
        list_add_tail(&connection->list,firstList);
        mutex_unlock(firstMagic);
        if(i==size-1)return connection;
    next:
        lastBind=connection->bind;
        lastMagic=connection->magic;  
    } 
    return NULL;
}

EXPORT_SYMBOL(GetTheMailConditioner);

static void Layer1End(void){
   kmem_cache_destroy(tmccache);
}
static void Layer0Start(void){
    tmccache=kmem_cache_create("tmccache", sizeof(struct TheMailConditioner),0, SLAB_HWCACHE_ALIGN,NULL);
    for(u8 i=0;i<255;i++)
        mutex_init(&Magic[i]);
    for(u8 i=0;i<4;i++)
        for(u8 j=0;j<16;j++)
        INIT_LIST_HEAD(&Bind[i][j]);
}
Layer0_1Setup("TheMailConditioner",0,0)