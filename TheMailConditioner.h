#ifndef TheMailConditioner_H
#define TheMailConditioner_H
#include "../ExpiryWorkBase/ExpiryWorkBase.h"
#include "../TheRequirements_0_1/TheRequirements.h"
struct TheMailConditioner;
extern struct TheMailConditioner*GetTheMailConditioner(u8*,u8,bool);
extern bool SetAutoDeleteTheMailConditioner(struct TheMailConditioner*,void(*)(void*,struct ExpiryWorkBaseBenchmark));
extern void*GetTheMailConditionerData(struct TheMailConditioner*);
extern bool SetTheMailConditionerData(struct TheMailConditioner*,void*);
#define SetupTMC struct TheMailConditioner*tmc
#endif