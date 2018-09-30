
add_library(qni.core SHARED
    qni-command.c
    qni-console-context.c
    qni-connector-context.c
    qni-alloc.c
    impls/qni-hashmap.c)

target_link_libraries(
    qni.core qni.api
)

target_include_directories(
    qni.core PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
)

install(
    TARGETS qni.core
    LIBRARY DESTINATION lib
)
