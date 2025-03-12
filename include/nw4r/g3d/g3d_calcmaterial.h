#ifndef NW4R_G3D_CALCMATERIAL_H
#define NW4R_G3D_CALCMATERIAL_H
#include <nw4r/g3d/g3d_resmdl.h>
#include <nw4r/types_nw4r.h>

namespace nw4r {
namespace g3d {

// Forward declarations
class AnmObjTexPat;
class AnmObjTexSrt;
class AnmObjMatClr;

void CalcMaterialDirectly(ResMdl, AnmObjTexPat*, AnmObjTexSrt*, AnmObjMatClr*);

} // namespace g3d
} // namespace nw4r

#endif