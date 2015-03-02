#include "manu343726/bandit/bandit.h"
#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Instructions.h"

using namespace bandit;

go_bandit([]() {
    describe("rom:", []() {
        it("can load super mario brothers", [&]() {
            CartridgeLoader loader;
            Cartridge rom = loader.loadCartridge("../roms/Super Mario Bros (E).nes");
        });
    });

    describe("opcodes - partial specialization", []() {

        // bootstrap nes cpu instructions
        Opcode opcodes[0x100];
        Instructions* instructions = new Instructions(opcodes);
        instructions->initialize();

        InstructionContext* ctx = new InstructionContext();

        it("SEI is implemented", [&]() {
            instructions->execute(SEI, ctx);
        });

        it("ORA is not implemented", [&]() {
            instructions->execute(ORA, ctx);
        });
    });
});

// run all tests
int main(int argc, char *argv[]) {
    return bandit::run(argc, argv);
}