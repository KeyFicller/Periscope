set(APP_NAME PERISCOPE_SANDBOX)

file(GLOB_RECURSE PRI_HEADERS CONFIGURE_DEPENDS
    inc/**.h
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/**.cpp
)

add_executable(${APP_NAME} ${SOURCES} ${PRI_HEADERS})

set_target_properties(${APP_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    DEFINE_SYMBOL "${APP_NAME}_APP"
)

target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${APP_NAME} PRIVATE Periscope::periscope)
