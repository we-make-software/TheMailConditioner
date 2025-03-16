#include "../ExpiryWorkBase/ExpiryWorkBase.h"

bool IsOdd(u8 value){
    return value&1;
}
EXPORT_SYMBOL(IsOdd);


struct NetworkVersionOctetItemLoop{
    SetupEWB;
    bool IsConnectToRouter:1,IsConnectToPointer:2;
    u8 Index:5,octet;
    struct list_head list; 
    struct list_head Odd[16],Even[16];
    struct mutex OddMutex;
};
struct NetworkVersionOctetItemData{
    SetupEWB;
    bool IsConnectToRouter:1,IsConnectToPointer:2;
    u8 Index:5,octet;
    struct list_head list; 
    struct list_head Odd[16],Even[16];
    struct mutex OddMutex;
    void *Pointer;
};
struct NetworkVersionOctetItemData16{
    SetupEWB;
    bool IsConnectToRouter:1,IsConnectToPointer:2;
    u8 Index:5,octet;
    void *Pointer;
};
static struct list_head NetworkVersionOctetOdd[16], NetworkVersionOctetEven[16];

static DEFINE_MUTEX(NetworkVersionOctetListMutex);
static DEFINE_MUTEX(OddNetworkVersionOctetListMutex);
struct RouterTable{
    struct list_head Odd[16],Even[16];
    u8 MediaAccessControl[6];
    SetupEWB;
    struct NetworkAdapterTable*nat;
    struct list_head list;
    struct mutex OddMutex;
};
u8 WhatGroup(u8 value);
u8 WhatGroup(u8 value){
    return value >> 5;
}
EXPORT_SYMBOL(WhatGroup);
static void AutoDeleteNetworkVersionOctetItem(void*){

}
struct NetworkVersionOctetItemLoop*GetNetworkPointer(u8*Value,u8 Index,bool IsVersion6,struct list_head*Odd,struct list_head*Even,struct mutex*mutexPrevious,struct mutex*OddmutexPrevious,struct ExpiryWorkBase*Previous,bool IsConnectToRouter){
    while((IsVersion6&&Index<16)||(!IsVersion6&&Index<4)){  
        struct mutex*mutex=IsOdd(Value[0])?&OddmutexPrevious:mutexPrevious;
        mutex_lock(mutex);
        u8 _value=Value[Index];
        struct list_head*head=IsOdd(_value)?&Odd[WhatGroup(_value)]:&Even[WhatGroup(_value)];
        struct NetworkVersionOctetItemLoop*entry=NULL;
        if (!list_empty(head)){
            struct NetworkVersionOctetItemLoop*first_entry=list_first_entry(head, struct NetworkVersionOctetItemLoop, list),
                                            *last_entry=list_last_entry(head, struct NetworkVersionOctetItemLoop, list),*pos;
            if (_value>=first_entry->octet&&_value<=last_entry->octet)
            if(_value==first_entry->octet)
                entry=first_entry;
            else if(_value==last_entry->octet)
                entry=last_entry;
            else if(_value-first_entry->octet<last_entry->octet-_value)
                list_for_each_entry(pos,head,list)
                    if(pos->octet==_value){
                        entry=pos;
                        break;
                    }
            else
                list_for_each_entry_reverse(pos,head,list)
                    if(pos->octet==_value){
                        entry=pos;
                        break;
                    }
        }
        if(!entry){
            entry=kmalloc(sizeof(struct NetworkVersionOctetItemData),GFP_KERNEL);
            if(!entry){
                mutex_unlock(mutex);
                return NULL;
            }
            entry->octet=_value;
            entry->Index=Index;
            entry->IsConnectToRouter=IsConnectToRouter;
            entry->IsConnectToPointer=false;
            mutex_init(&entry->OddMutex);
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
                else if(_value-first_entry->octet<last_entry->octet-_value)
                    list_for_each_entry(pos,head,list)
                        if(pos->octet>_value){
                            list_add_tail(&entry->list,&pos->list);
                            break;
                        }
                else
                    list_for_each_entry_reverse(pos,head,list)
                        if(pos->octet<_value){
                            list_add(&entry->list,&pos->list);
                            break;
                        }
            }
            if(IsVersion6&&Index==15){
                mutex_unlock(mutex);
                ResetExpiryWorkBase(&entry->ewb);
                return entry;
            }else for (u8 i=0;i<16;i++) {
                INIT_LIST_HEAD(&entry->Odd[i]);
                INIT_LIST_HEAD(&entry->Even[i]);
            }
            if(!IsVersion6&&Index==3){
                mutex_unlock(mutex);
                ResetExpiryWorkBase(&entry->ewb);
                return entry;
            }
            mutex_unlock(mutex);
        }
        if((IsVersion6&&Index==15)||(!IsVersion6&&Index==3)){
            ResetExpiryWorkBase(&entry->ewb);
            return entry;
        }
        mutex_unlock(mutex);
        struct ExpiryWorkBase*_previous=&entry->ewb;
        mutexPrevious=&entry->ewb.Mutex;
        OddmutexPrevious=&entry->OddMutex;
        Odd=entry->Odd;
        Even=entry->Even;
        Previous=_previous;
        Index++;
    }
    return NULL;
}


void*GetGlobelNetworkPointer(u8*Value,bool IsVersion6){
    return GetNetworkPointer(Value,0,IsVersion6,NetworkVersionOctetOdd,NetworkVersionOctetEven,&NetworkVersionOctetListMutex,&OddNetworkVersionOctetListMutex,NULL,false);
}
void*GetNetworkRouterPointer(struct RouterTable*router,u8*Value,bool IsVersion6){
    return GetNetworkPointer(Value,0,IsVersion6,router->Odd,router->Even,&router->ewb.Mutex,&router->OddMutex,&router->ewb,true);
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
        list_for_each_entry_safe(entry,tmp,&NetworkVersionOctetOdd[i],Odd[i]) 
            CancelExpiryWorkBase(&entry->ewb);
        list_for_each_entry_safe(entry,tmp,&NetworkVersionOctetEven[i],Even[i])
            CancelExpiryWorkBase(&entry->ewb);
    }
}
static void Starting(void){
    for (u8 i=0;i<16;i++) {
        INIT_LIST_HEAD(&NetworkVersionOctetOdd[i]);
        INIT_LIST_HEAD(&NetworkVersionOctetEven[i]);
    }
}
Setup("The Mail Conditioner",Starting(),Closing())



