message("Using CPP Delegates")
target_include_directories(${TARGET_NAME} PUBLIC ${LIBRARIES_DIRECTORY}/CppDelegates)
target_sources(${TARGET_NAME} PRIVATE ${LIBRARIES_DIRECTORY}/CppDelegates/Delegates.cpp)

