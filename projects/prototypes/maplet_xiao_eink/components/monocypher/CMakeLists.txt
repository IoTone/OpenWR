idf_component_register(SRCS "monocypher.c" "monocypher-ed25519.c" INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE -O2 -Wno-unused-function)
