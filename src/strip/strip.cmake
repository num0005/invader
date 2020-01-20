# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_STRIP})
    set(INVADER_STRIP true CACHE BOOL "Build invader-strip (strips hidden values from tags)")
endif()

if(${INVADER_STRING})
    add_executable(invader-strip
        src/strip/strip.cpp
    )
    target_link_libraries(invader-strip invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-strip)
endif()