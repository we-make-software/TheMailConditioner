#include "../ExpiryWorkBase/ExpiryWorkBase.h"
struct NetworkAdapterTable {
    struct net_device*dev;
    struct ExpiryWorkBase expiryWorkBase;
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

    /*
bool ThePostOfficeSendPacket(struct IEEE8021Router* router){
    struct sk_buff *skb;
    skb=netdev_alloc_skb(router->NetworkInterfaces->dev,14+NET_IP_ALIGN);
    if(!skb)return false;
    skb_reserve(skb,NET_IP_ALIGN);
    skb_put_data(skb,router->MediaAccessControl,6);
    skb_put_data(skb,router->NetworkInterfaces->dev->dev_addr,6); 
    u16 Ethertype=htons(router->IsVersion6?34525:2048); 
    skb_put_data(skb,&Ethertype,2); 
    skb->dev=router->NetworkInterfaces->dev;
    skb->protocol=htons(ETH_P_IP);
    skb->priority=0;
    if(dev_queue_xmit(skb)<0){
        kfree_skb(skb);
        return false;
    }
    return true;
}
*/