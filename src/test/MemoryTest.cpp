#include "manu343726/bandit/bandit.h"
#include "../Memory.h"
#include "../MemoryStack.h"

using namespace bandit;

go_bandit([]() {
    Memory* memory = new Memory(NULL);
    Registers* registers = new Registers();
    Stack* stack = new Stack(memory, registers);

    describe("stack:", [&]() {
        it("can push and pop a byte correctly", [&]() {
            stack->pushStackByte(0x12);
            tCPU::byte value = stack->popStackByte();
            AssertThat(value, Equals(0x12));
        });

        it("can push and pop a word correctly", [&]() {
            stack->pushStackWord(0x1234);
            tCPU::word value = stack->popStackWord();
            AssertThat(value, Equals(0x1234));
        });
    });
});

/**
 * Run tests
 */
int main(int argc, char *argv[]) {
    el::Configurations defaultConf;
    defaultConf.setToDefault();

    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "%datetime{%h:%m:%s.%g %F} | %level | %msg");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%datetime{%h:%m:%s.%g %F} | %level | %msg");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Format, "%datetime{%h:%m:%s.%g %F} | %level | %msg");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format, "%datetime{%h:%m:%s.%g %F} | %level | %msg");

    el::Loggers::reconfigureLogger("default", defaultConf);

    return bandit::run(argc, argv);
}