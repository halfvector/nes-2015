#include "manu343726/bandit/bandit.h"
#include "Cartridge.h"
#include "CartridgeLoader.h"

using namespace bandit;

go_bandit([]() {
    describe("rom:", []() {
        it("can load super mario brothers", [&]() {
            CartridgeLoader loader;
            Cartridge rom = loader.loadCartridge("../roms/Super Mario Bros (E).nes");
        });
    });
});

// run all tests
int main(int argc, char *argv[]) {
    return bandit::run(argc, argv);
}