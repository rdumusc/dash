# Copyright (c) 2011-2015 Stefan Eilemann <Stefan.Eilemann@epfl.ch>
# Change this number when adding tests to force a CMake run: 3

include_directories(${PROJECT_SOURCE_DIR}/tests)

set(TEST_LIBRARIES dash)
include(CommonCTest)

install(FILES ${TEST_FILES} DESTINATION share/dash/tests COMPONENT examples)
