#ifndef RP_GRAPHICS_MODEL_MANAGER_H
#define RP_GRAPHICS_MODEL_MANAGER_H
#include <types_RP.h>

class RPGrpModelManager {
public:
    static RPGrpModelManager* getInstance() {
        return spInstance;
    }

    void CreateModelScene(u32, u8, u32, RPGrpLightManager*, RPGrpFogManager*);

private:
    static RPGrpModelManager* spInstance;
};

#endif
