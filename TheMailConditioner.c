#include "../ExpiryWorkBase/ExpiryWorkBase.h"
struct RouterTable{
    u8 MediaAccessControl[6];
    SetupEWB;
    struct NetworkAdapterTable*nta;
    struct list_head list,octets;
};

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

Setup("The Mail Conditioner",{},{})

