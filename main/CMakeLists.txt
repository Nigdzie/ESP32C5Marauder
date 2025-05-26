idf_component_register(SRCS "main.c"
                      INCLUDE_DIRS ".")
idf_build_set_property(COMPILE_OPTIONS "-Os" APPEND)  # optymalizacja rozmiaru
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=unused-variable" APPEND)
