#ifndef TheMailConditioner_H
#define TheMailConditioner_H
#include "../ExpiryWorkBase/ExpiryWorkBase.h"
struct TheMailConditioner;
struct TheMailConditioner*GetTheMailConditionerByAddress(u8*,u8,bool);
extern bool SetAutoDeleteTheMailConditioner(struct TheMailConditioner*,void(*)(void*,struct ExpiryWorkBaseBenchmark));
#endif