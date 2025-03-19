#include "../TheRequirements_0_1/TheRequirements.h"
struct TheMailConditioner{
    SetupEWB;
    u8 octet;
    struct list_head bind[4][16];
    struct mutex magic[64];
    void(*bindDelete)(void*,struct ExpiryWorkBaseBenchmark);
    void*data;
    struct list_head list;
};
static struct kmem_cache*tmccache;
void CancelTheMailConditioner(struct TheMailConditioner*);
void CancelTheMailConditioner(struct TheMailConditioner*tmc){
    if (!tmc) return;
    if(!LockExpiryWorkBase(tmc->ewb)||!GetExpiryWorkBaseParent(tmc->ewb))return;
    if(tmc->bindDelete)
        tmc->bindDelete(tmc->data,TheBenchmarksExpiryWorkBase(tmc->ewb,false,false));
    list_del(&tmc->list);
    void *tempData = tmc->data;
    CancelExpiryWorkBase(tmc->ewb);
    UnlockExpiryWorkBase(tmc->ewb);
    kmem_cache_free(tmccache,tmc);
    if(tempData)kfree(tempData);
}
EXPORT_SYMBOL(CancelTheMailConditioner);
void AutoDeleteData(void* data) {
    struct TheMailConditioner* tmc = (struct TheMailConditioner*)data;
    if (!tmc) return;
    void *tempData = tmc->data;
    void (*deleteFunc)(void*, struct ExpiryWorkBaseBenchmark) = tmc->bindDelete;
    kmem_cache_free(tmccache, tmc);
    if (deleteFunc) deleteFunc(tempData, TheBenchmarksExpiryWorkBase(tmc->ewb, false, false));
    if (tempData) kfree(tempData);
}
bool SetAutoDeleteTheMailConditioner(struct TheMailConditioner*,void(*)(void*,struct ExpiryWorkBaseBenchmark));
bool SetAutoDeleteTheMailConditioner(struct TheMailConditioner*tmc,void(*bindDelete)(void*,struct ExpiryWorkBaseBenchmark)){
    if(!GetExpiryWorkBaseParent(tmc->ewb))return false;
    tmc->bindDelete=bindDelete;
    return true;
}
EXPORT_SYMBOL(SetAutoDeleteTheMailConditioner);
void*GetTheMailConditionerData(struct TheMailConditioner*tmc){
    if(!GetExpiryWorkBaseParent(tmc->ewb))return NULL;
    return tmc->data;
}
EXPORT_SYMBOL(GetTheMailConditionerData);
bool SetTheMailConditionerData(struct TheMailConditioner*,void*);
bool SetTheMailConditionerData(struct TheMailConditioner*tmc,void*data){
    if(!GetExpiryWorkBaseParent(tmc->ewb))return false;
    tmc->data=data;
    return true;
}
EXPORT_SYMBOL(SetTheMailConditionerData);
static struct mutex Magic[64];
static struct list_head Bind[4][16];

struct TheMailConditioner*GetTheMailConditioner(u8*,u8,bool);
struct TheMailConditioner*GetTheMailConditioner(u8*value,u8 size,bool set){
    struct mutex*lastMagic=Magic;
    struct list_head(*lastBind)[16]=Bind;
    for(u8 i=0;i<size;i++){
        u8 octet=value[i],group=(octet&1)+((octet>>5)>7?2:0),slot=octet>>5;
        struct mutex*firstMagic=&lastMagic[group*16+slot];
        struct list_head*firstList=&lastBind[group][slot];
        mutex_lock(firstMagic);
        if(!set&&list_empty(firstList)){
            mutex_unlock(firstMagic);
            return NULL;
        }
        if(!list_empty(firstList)){
            struct TheMailConditioner*connection;
            bool next=false;
            list_for_each_entry(connection,firstList,list)
                if(connection->octet==octet&&GetExpiryWorkBaseParent(connection->ewb)){
                    if(i==size-1){
                        mutex_unlock(firstMagic);
                        return connection;
                    }
                    lastMagic=connection->magic;
                    lastBind=(struct list_head(*)[16])connection->bind;
                    next=true;
                    break;
                }
            
            if(next){
                mutex_unlock(firstMagic);
                continue;
            }
        }
        if(!set){
            mutex_unlock(firstMagic);
            return NULL;
        }
        for(;i<size;i++){
            octet=value[i];
            group=(octet&1)+((octet>>5)>7?2:0);
            slot=octet>>5;
            firstMagic=&lastMagic[group*16+slot];
            firstList=&lastBind[group][slot];
            mutex_trylock(firstMagic);
            if(list_empty(firstList))INIT_LIST_HEAD(firstList);
            struct TheMailConditioner*connection=kmem_cache_alloc(tmccache,GFP_KERNEL);
            if(!connection){
                mutex_unlock(firstMagic);
                return NULL;
            }
            struct TheMailConditioner*firstconnection=NULL;
            if(i!=0)
                firstconnection=list_first_entry(firstList,struct TheMailConditioner,list);
            if(!SetupExpiryWorkBase(&connection->ewb,(i==0)?NULL:firstconnection->ewb,connection,AutoDeleteData)){
                kmem_cache_free(tmccache,connection);
                mutex_unlock(firstMagic);
                return NULL;
            }
            for(u8 j=0;j<64;j++)
                mutex_init(&connection->magic[j]);
            connection->octet=octet;
            list_add(&connection->list,firstList);
            mutex_unlock(firstMagic);
            if(i==size-1)return connection;
            lastBind=(struct list_head(*)[16])connection->bind;
            lastMagic=connection->magic;
        }
    }
    return NULL;
}
EXPORT_SYMBOL(GetTheMailConditioner);
static void tmcinit(void *obj) {
    struct TheMailConditioner *tmc = (struct TheMailConditioner *)obj;
    tmc->data=NULL;
    tmc->bindDelete=NULL;
    tmc->octet=0;
    for(u8 i=0;i<64;i++)
        mutex_init(&tmc->magic[i]);
    INIT_LIST_HEAD(&tmc->list);
}

static void Layer1End(void){
    if(tmccache)kmem_cache_destroy(tmccache);
}
static void Layer0Start(void){
    tmccache=kmem_cache_create("tmccache", sizeof(struct TheMailConditioner),0, SLAB_HWCACHE_ALIGN,tmcinit);
    for(u8 i=0;i<64;i++)
        mutex_init(&Magic[i]);
}
Layer0_1Setup("TheMailConditioner",0,0)