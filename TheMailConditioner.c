#include "../ExpiryWorkBase/ExpiryWorkBase.h"

extern void BackgroundResetExpiryWorkBase(struct ExpiryWorkBase*expiry_work_base);
struct NetworkTabel{
    SetupEWB;
    u8 IsVersion6:1,IsRouter:1,IsGateway:1,IsBlocked:5;
    u8*Address;
    struct list_head list;
};
struct NetworkVersionOctetItemData{
    SetupEWB;
    u8 Index,octet;
    struct list_head list; 
    struct list_head Octets[4][16];
    struct mutex Mutex[64];
    struct NetworkTabel*Gateway;
};

static struct list_head GlobelOctet[4][16];
static struct mutex GlobelMutex[64];
struct RouterTable{
    u8 MediaAccessControl[6];
    SetupEWB;
    struct NetworkAdapterTable*nat;
    struct list_head list;
    struct list_head Octets[4][16];
    struct mutex Mutex[64],Set;

    struct list_head Gateways; 
};

static void AutoDeleteNetworkVersionOctetItem(void*){

}
static void AutoDeleteRouter(void*){

}
static void AutoDeleteNetwork(void*){

}
struct NetworkVersionOctetItemData*QuickGetNetworkPointer(u8*Value,u8 Index,bool IsVersion6,struct list_head(*octets)[4][16]){
    while((IsVersion6&&Index<16)||(!IsVersion6&&Index<4)){  
        u8 _value=Value[Index];
        struct list_head*head=octets[(_value&1)+((_value>>5)>7?2:0)][_value>>5];
        struct NetworkVersionOctetItemData*entry=NULL;
        if (!list_empty(head)){
            struct NetworkVersionOctetItemData*first_entry=list_first_entry(head, struct NetworkVersionOctetItemData, list),
                                            *last_entry=list_last_entry(head, struct NetworkVersionOctetItemData, list),*pos;
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
            head=(struct list_head*)entry->Octets;

        Index++;
    }
    return NULL;
}
static DEFINE_MUTEX(StopRaceAddNetworkPointer);
struct NetworkVersionOctetItemData*AddNetworkPointer(u8*Value,u8 Index,bool IsVersion6,struct list_head(*octets)[4][16],struct mutex(*mutex)[64],struct ExpiryWorkBase*Previous){
    {
        struct NetworkVersionOctetItemData*entry=QuickGetNetworkPointer(Value,Index,IsVersion6,octets);
        if(entry){
            if(entry->ewb.Invalid)
                return NULL;
            BackgroundResetExpiryWorkBase(&entry->ewb);
            return entry;
        }
    }
    mutex_lock(&StopRaceAddNetworkPointer);
    {
        struct NetworkVersionOctetItemData*entry=QuickGetNetworkPointer(Value,Index,IsVersion6,octets);
        if(entry){
            mutex_unlock(&StopRaceAddNetworkPointer);  
            if(entry->ewb.Invalid)return NULL;
            BackgroundResetExpiryWorkBase(&entry->ewb);
            return entry;
        }
    }
    bool LocalStopRaceAddNetworkPointer=true;
    while((IsVersion6&&Index<16)||(!IsVersion6&&Index<4)){  
        u8 _value=Value[Index];
        u8 group=(_value&1)+((_value>>5)>7?2:0);
        u8 slot=_value>>5;
        struct mutex* lock = mutex[group*16+slot];
        mutex_lock(lock);
        struct list_head*head=octets[group][slot];
        struct NetworkVersionOctetItemData*entry=NULL;
        if (!list_empty(head)){
            struct NetworkVersionOctetItemData*first_entry=list_first_entry(head, struct NetworkVersionOctetItemData, list),
                                              *last_entry=list_last_entry(head, struct NetworkVersionOctetItemData, list),*pos;
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
                if(LocalStopRaceAddNetworkPointer)
                mutex_unlock(&StopRaceAddNetworkPointer);
                return NULL;
            }
            entry->octet=_value;
            entry->Index=Index;
            entry->Gateway=NULL;
            for(u8 i=0;i<64;i++)
                mutex_init(&entry->Mutex[i]);
            SetupExpiryWorkBase(&entry->ewb,Previous,entry,AutoDeleteNetworkVersionOctetItem);
            INIT_LIST_HEAD(&entry->list);
            if(list_empty(head))
                list_add(&entry->list,head);
            else{
                struct NetworkVersionOctetItemData*first_entry=list_first_entry(head, struct NetworkVersionOctetItemData, list),
                                                  *last_entry=list_last_entry(head,struct NetworkVersionOctetItemData, list),
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
            if(LocalStopRaceAddNetworkPointer){
                LocalStopRaceAddNetworkPointer=false;
                mutex_unlock(&StopRaceAddNetworkPointer);
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
        if(LocalStopRaceAddNetworkPointer){
            LocalStopRaceAddNetworkPointer=false;
            mutex_unlock(&StopRaceAddNetworkPointer);
        }
        mutex=&entry->Mutex;
        octets=&entry->Octets;
        Previous=&entry->ewb;
        Index++;
    }
    return NULL;
}


static DEFINE_MUTEX(ControlGetGlobelNetworkPointer);
struct NetworkTabel*GetGlobelNetworkPointer(u8*Value,bool IsVersion6){
    struct NetworkVersionOctetItemData*entry=AddNetworkPointer(Value,0,IsVersion6,&GlobelOctet,&GlobelMutex,NULL);
    if(!entry||entry->ewb.Invalid)return NULL;
    if(entry->Gateway){
        BackgroundResetExpiryWorkBase(&entry->ewb);
        return entry->Gateway;
    }
    mutex_lock(&ControlGetGlobelNetworkPointer);
    entry=AddNetworkPointer(Value,0,IsVersion6,&GlobelOctet,&GlobelMutex,NULL);
    if(!entry||entry->ewb.Invalid){
        mutex_unlock(&ControlGetGlobelNetworkPointer);
        return NULL;
    }
    entry->Gateway=kmalloc(sizeof(struct NetworkTabel),GFP_KERNEL);
    if(!entry->Gateway){
        BackgroundResetExpiryWorkBase(&entry->ewb);
        return NULL;
    }
    entry->Gateway->Address=kmalloc(IsVersion6?16:4,GFP_KERNEL);
    if(!entry->Gateway->Address){
        kfree(entry->Gateway);
        BackgroundResetExpiryWorkBase(&entry->ewb);
        return NULL;
    }
    memcpy(entry->Gateway->Address,Value,IsVersion6?16:4);
    entry->Gateway->IsVersion6=IsVersion6;
    entry->Gateway->IsGateway=1;
    entry->Gateway->IsRouter=0;
    entry->Gateway->IsBlocked=0;
    SetupExpiryWorkBase(&entry->Gateway->ewb,&entry->ewb,entry->Gateway,AutoDeleteNetwork);
    BackgroundResetExpiryWorkBase(&entry->ewb);
    mutex_unlock(&ControlGetGlobelNetworkPointer);
    return entry->Gateway;
}
struct NetworkTabel*GetNetworkRouterPointer(struct RouterTable*router,u8*Value,bool IsVersion6){
    struct NetworkVersionOctetItemData*entry=AddNetworkPointer(Value,0,IsVersion6,&router->Octets,&router->Mutex,&router->ewb);
    if(!entry||entry->ewb.Invalid)return NULL;
    if(entry->Gateway){
        BackgroundResetExpiryWorkBase(&entry->ewb);
        return entry->Gateway;
    }
    mutex_lock(&router->Set); 
    entry=AddNetworkPointer(Value,0,IsVersion6,&router->Octets,&router->Mutex,&router->ewb);
    if(!entry||entry->ewb.Invalid){
        mutex_unlock(&router->Set);
        return NULL;
    }
    entry->Gateway=kmalloc(sizeof(struct NetworkTabel),GFP_KERNEL);
    if(!entry->Gateway){
        mutex_unlock(&router->Set);
        BackgroundResetExpiryWorkBase(&entry->ewb);
        return NULL;
    }
    entry->Gateway->Address=kmalloc(IsVersion6?16:4,GFP_KERNEL);
    if(!entry->Gateway->Address){
        mutex_unlock(&router->Set);
        kfree(entry->Gateway);
        BackgroundResetExpiryWorkBase(&entry->ewb);
        return NULL;
    }
    memcpy(entry->Gateway->Address,Value,IsVersion6?16:4);
    entry->Gateway->IsVersion6=IsVersion6;
    entry->Gateway->IsGateway=0;
    entry->Gateway->IsRouter=1;
    entry->Gateway->IsBlocked=0;
    SetupExpiryWorkBase(&entry->Gateway->ewb,&entry->ewb,entry->Gateway,AutoDeleteNetwork);
    list_add(&entry->Gateway->list,&router->Gateways);
    BackgroundResetExpiryWorkBase(&entry->ewb);
    mutex_unlock(&router->Set);
    return entry->Gateway;
}   


struct PacketConversion{
    struct NetworkTabel*Server,*Client;
    u8*data;
    u16 SourcePort;
    bool IsTransmissionControlProtocol;
    bool IsVersion6;
};

struct NetworkAdapterTable {
    SetupEWB;
    struct list_head routers;
};
struct RouterTable*GetRouter(struct NetworkAdapterTable*nat,u8*MediaAccessControl){
    struct RouterTable*router;
    if(list_empty(&nat->routers))return NULL;
    list_for_each_entry(router,&nat->routers,list)
        if(!memcmp(router->MediaAccessControl,MediaAccessControl,6))
            return router;
    return NULL;
}
struct RouterTable*AddRouter(struct NetworkAdapterTable*nat,u8*MediaAccessControl){
    struct RouterTable*router=GetRouter(nat,MediaAccessControl);
    if(router)return router;
    mutex_lock(&nat->ewb.Mutex);
    router=GetRouter(nat,MediaAccessControl);
    if(router){
        mutex_unlock(&nat->ewb.Mutex);
        return router;
    }
    router=kmalloc(sizeof(struct RouterTable),GFP_KERNEL);
    if(!router){
        mutex_unlock(&nat->ewb.Mutex);
        return NULL;
    }
    memcpy(router->MediaAccessControl,MediaAccessControl,6);
    router->nat=nat;
    INIT_LIST_HEAD(&router->list);
    INIT_LIST_HEAD(&router->Gateways);
    mutex_init(&router->Set);
    for(u8 i=0;i<64;i++)
        mutex_init(&router->Mutex[i]);
    SetupExpiryWorkBase(&router->ewb,&nat->ewb,router,AutoDeleteRouter);
    BackgroundResetExpiryWorkBase(&router->ewb);
    list_add(&router->list,&nat->routers);
    mutex_unlock(&nat->ewb.Mutex);
    return router;
}
void TheMailConditionerPacketWorkHandler(struct NetworkAdapterTable*,struct PacketConversion*);
void TheMailConditionerPacketWorkHandler(struct NetworkAdapterTable*nta,struct PacketConversion*pc){
    struct RouterTable*router=AddRouter(nta,pc->data);
    if(!router)return;
    pc->data+=2;
    pc->Server=GetGlobelNetworkPointer(pc->data+(pc->IsVersion6?8:12),pc->IsVersion6);
    if(!pc->Server||pc->Server->IsBlocked)return;
    pc->Client=GetNetworkRouterPointer(router,pc->data+(pc->IsVersion6?8:12),pc->IsVersion6);
    if(!pc->Client||pc->Client->IsBlocked)return;
    //here it will go to other project else to many lines in this project
}
EXPORT_SYMBOL(TheMailConditionerPacketWorkHandler);

static void Closing(void){
    struct NetworkVersionOctetItemData*entry,*tmp;
    for (u8 i=0; i<64; i++) {
        mutex_lock(&GlobelMutex[i]);
        list_for_each_entry_safe(entry,tmp,&GlobelOctet[i/16][i%16],list)
            CancelExpiryWorkBase(&entry->ewb);
        mutex_unlock(&GlobelMutex[i]);
    }
}
static void Starting(void){
    for (u8 i=0;i<64;i++)
        mutex_init(&GlobelMutex[i]);
}
Setup("The Mail Conditioner",Starting(),Closing())



