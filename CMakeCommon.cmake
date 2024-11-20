function(add_common_target_properties target)
    set_target_properties(${TARGET_NAME} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED On
            #ONLY_ACTIVE_ARCH "YES"
    )
endfunction()

function(add_precompiled_headers target)
    target_precompile_headers(${TARGET_NAME} PRIVATE
            "<cstdint>"
            "<iostream>"
            "<vector>"
            "<unordered_set>"
            "<algorithm>"
            "<tuple>"
            "<bitset>"
            "<cstring>"
            "<fstream>"
            "<sstream>"
            "<array>"
            "<memory>"
            "<string>"
            "<ranges>"
            "<any>"
            "<utility>"
            "<queue>"
            "<functional>"
            "<map>"
            "<assert.h>"
            "<mutex>"
    )
endfunction()
