# Actually create targets: EXEcutables and libraries.
ADD_BIICODE_TARGETS()

TARGET_COMPILE_OPTIONS(${BII_BLOCK_TARGET} INTERFACE "-std=c++1y")
