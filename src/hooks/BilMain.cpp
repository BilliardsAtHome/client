#include "hooks/BilMain.h"

#include "core/Simulation.h"

namespace BAH {

/**
 * @brief Shot end callback
 */
void BilMain::OnEndShot() {
    // Let simulation record results
    Simulation::GetInstance().Finish();
}
KM_BRANCH_MF(0x802c57ec, BilMain, OnEndShot);
KM_BRANCH_MF(0x802c563c, BilMain, OnEndShot);

namespace {

/**
 * @brief Shorten intro camera (OPENINGDEMO state)
 */
KM_WRITE_32(0x802c5b30, 0x60000000);

} // namespace
} // namespace BAH