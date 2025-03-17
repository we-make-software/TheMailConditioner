#include "../ExpiryWorkBase/ExpiryWorkBase.h"

extern void BackgroundResetExpiryWorkBase(struct ExpiryWorkBase*expiry_work_base);

struct NetworkVersionOctetItemLoop{
    SetupEWB;
    u8 IsConnectToRouter:1,IsConnectToPointer:2,Index:5,octet;
    struct list_head list; 
    struct list_head Octets[4][16];
    struct mutex Mutex[16];
};
struct NetworkVersionOctetItemData{
    SetupEWB;
    u8 IsConnectToRouter:1,IsConnectToPointer:2,Index:5,octet;
    struct list_head list; 
    struct list_head Octets[4][16];
    struct mutex Mutex[16];
    void *Pointer;
};
struct NetworkVersionOctetItemData16{
    SetupEWB;
    u8 IsConnectToRouter:1,IsConnectToPointer:2,Index:5,octet;
    void *Pointer;
};
static struct list_head GlobelOctet[4][16];
static struct mutex GlobelMutex[16];
struct RouterTable{
    u8 MediaAccessControl[6];
    SetupEWB;
    struct NetworkAdapterTable*nat;
    struct list_head list;
    struct list_head Octets[4][16];
    struct mutex Mutex[16];
};

static void AutoDeleteNetworkVersionOctetItem(void*){

}
struct NetworkVersionOctetItemLoop*QuickGetNetworkPointer(u8*Value,u8 Index,bool IsVersion6,struct list_head(*octets)[16]){
    while((IsVersion6&&Index<16)||(!IsVersion6&&Index<4)){  
        u8 _value=Value[Index];
        struct list_head*head=&octets[(_value&1)+_value>>5>7?1:0][_value>>5];
        struct NetworkVersionOctetItemLoop*entry=NULL;
        if (!list_empty(head)){
            struct NetworkVersionOctetItemLoop*first_entry=list_first_entry(head, struct NetworkVersionOctetItemLoop, list),
                                            *last_entry=list_last_entry(head, struct NetworkVersionOctetItemLoop, list),*pos;
                if(_value>=first_entry->octet&&_value<=last_entry->octet)
                    if(_value==first_entry->octet)
                        entry=first_entry;
                    else if(_value==last_entry->octet)
                        entry=last_entry;
                    else 
                    list_for_each_entry(pos,head,list)
                        if(pos->octet==_value){
                            entry=pos;
                            break;
                        }                     
        }
        if(!entry)
            return NULL;
        if((IsVersion6&&Index==15)||(!IsVersion6&&Index==3)){
            if(entry->ewb.Invalid)return NULL;
            BackgroundResetExpiryWorkBase(&entry->ewb);
            return entry;
        }
        if(Index!=15)
            head=&entry->Octets;
        Index++;
    }
    return NULL;
}
struct BackgroundInstallList{
    struct NetworkVersionOctetItemLoop*entry;
    struct work_struct work;

};
static DEFINE_MUTEX(StopRaceAddNetworkPointer);
struct NetworkVersionOctetItemLoop*AddNetworkPointer(u8*Value,u8 Index,bool IsVersion6,struct list_head(*octets)[16],struct mutex(*mutex)[4],struct ExpiryWorkBase*Previous,bool IsConnectToRouter){
    {
        struct NetworkVersionOctetItemLoop*entry=QuickGetNetworkPointer(Value,Index,IsVersion6,octets);
        if(entry){
            if(entry->ewb.Invalid)
                return NULL;
            BackgroundResetExpiryWorkBase(&entry->ewb);
            return entry;
        }
    }
    mutex_lock(&StopRaceAddNetworkPointer);
    {
        struct NetworkVersionOctetItemLoop*entry=QuickGetNetworkPointer(Value,Index,IsVersion6,octets);
        if(entry){
            if(entry->ewb.Invalid){
                mutex_unlock(&StopRaceAddNetworkPointer);   
                return NULL;
            }
            mutex_unlock(&StopRaceAddNetworkPointer);
            BackgroundResetExpiryWorkBase(&entry->ewb);
            return entry;
        }
    }
    bool LocalStopRaceAddNetworkPointer=true;
    while((IsVersion6&&Index<16)||(!IsVersion6&&Index<4)){  
        u8 _value=Value[Index];
        struct mutex*lock=&mutex[(_value>>5)];
        mutex_lock(lock);
        struct list_head*head=&octets[(_value&1)+((_value>>5)>7?1:0)][_value>>5];
        struct NetworkVersionOctetItemLoop*entry=NULL;
        if (!list_empty(head)){
            struct NetworkVersionOctetItemLoop*first_entry=list_first_entry(head, struct NetworkVersionOctetItemLoop, list),
                                              *last_entry=list_last_entry(head, struct NetworkVersionOctetItemLoop, list),*pos;
            if(_value>=first_entry->octet&&_value<=last_entry->octet)
                if(_value==first_entry->octet)
                    entry=first_entry;
                else if(_value==last_entry->octet)entry=last_entry;
                else 
                    list_for_each_entry(pos,head,list)
                        if(pos->octet==_value){
                            entry=pos;
                            break;
                        }                            
        }
        else
            INIT_LIST_HEAD(head);
        if(!entry){
            entry=kmalloc(sizeof(struct NetworkVersionOctetItemData),GFP_KERNEL);
            if(!entry){
                mutex_unlock(lock);
                return NULL;
            }
            entry->octet=_value;
            entry->Index=Index;
            entry->IsConnectToRouter=IsConnectToRouter;
            entry->IsConnectToPointer=false;
            for(u8 i=0;i<4;i++)
                mutex_init(&entry->Mutex[i]);
            SetupExpiryWorkBase(&entry->ewb,Previous,entry,AutoDeleteNetworkVersionOctetItem);
            INIT_LIST_HEAD(&entry->list);
            if(list_empty(head))
                list_add(&entry->list,head);
            else{
                struct NetworkVersionOctetItemLoop*first_entry=list_first_entry(head, struct NetworkVersionOctetItemLoop, list),
                                                  *last_entry=list_last_entry(head,struct NetworkVersionOctetItemLoop, list),
                                                  *pos;
                if(_value<first_entry->octet)
                    list_add(&entry->list,head);
                else if(_value>last_entry->octet)
                    list_add_tail(&entry->list,head);
                else if(_value!=0&&(_value-1)==first_entry->octet)
                    list_add_tail(&entry->list,&first_entry->list);
                else
                    list_add(&entry->list,&last_entry->list);                                      
            }
            if(IsVersion6&&Index==15){
                if(entry->ewb.Invalid){
                    mutex_unlock(lock);
                    return NULL;
                }
                mutex_unlock(lock);
                BackgroundResetExpiryWorkBase(&entry->ewb);
                return entry;
            }
            if(!IsVersion6&&Index==3){
                if(entry->ewb.Invalid){
                    mutex_unlock(lock);
                    return NULL;
                }
                mutex_unlock(lock);
                BackgroundResetExpiryWorkBase(&entry->ewb);
                return entry;
            }
            mutex_unlock(lock);
        }
        if((IsVersion6&&Index==15)||(!IsVersion6&&Index==3)){
            BackgroundResetExpiryWorkBase(&entry->ewb);
            mutex_unlock(lock);
            return entry;
        }
        mutex_unlock(lock);
        struct ExpiryWorkBase*_previous=&entry->ewb;
        mutex=&entry->Mutex;
        octets=&entry->Octets;
        Previous=&entry->ewb;
        Index++;
        if(LocalStopRaceAddNetworkPointer){
            LocalStopRaceAddNetworkPointer=false;
            mutex_unlock(&StopRaceAddNetworkPointer);
        }
    }
    return NULL;
}


void*GetGlobelNetworkPointer(u8*Value,bool IsVersion6){
    return AddNetworkPointer(Value,0,IsVersion6,GlobelOctet,GlobelMutex,NULL,false);
}
void*GetNetworkRouterPointer(struct RouterTable*router,u8*Value,bool IsVersion6){
    //AddNetworkPointer(u8*Value,u8 Index,bool IsVersion6,struct list_head(*octets)[16],struct mutex(*mutex)[4],struct ExpiryWorkBase*Previous,bool IsConnectToRouter){
    return AddNetworkPointer(Value,0,IsVersion6,router->Octets,router->Mutex,&router->ewb,true);
}   


struct NetworkAdapterTable {
    struct list_head routers;
};
struct PacketConversion{
    u16 SourcePort;
    bool IsTransmissionControlProtocol;
    bool IsVersion6;
    struct sk_buff *skb;
};
void TheMailConditionerPacketWorkHandler(struct NetworkAdapterTable*,struct PacketConversion*);
void TheMailConditionerPacketWorkHandler(struct NetworkAdapterTable*nta,struct PacketConversion*pc){

}
EXPORT_SYMBOL(TheMailConditionerPacketWorkHandler);

static void Closing(void){
    struct NetworkVersionOctetItemLoop *entry, *tmp;
    for (u8 i=0;i<16;i++) {
        mutex_lock(&GlobelMutex[i]);
        for(u8 j=0;j<16;j++)
            list_for_each_entry_safe(entry, tmp, &GlobelOctet[i][j], list)
                CancelExpiryWorkBase(&entry->ewb);
        mutex_unlock(&GlobelMutex[i]);
    }
}
static void Starting(void){
    for (u8 i=0;i<16;i++)
        mutex_init(&GlobelMutex[i]);
}
Setup("The Mail Conditioner",Starting(),Closing())



