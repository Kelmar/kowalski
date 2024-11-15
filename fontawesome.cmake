function(download_font_awesome)
    set(FA_VERSION "6.6.0")
    set(FA_URL "https://use.fontawesome.com/releases/v${FA_VERSION}/fontawesome-free-${FA_VERSION}-desktop.zip")
    set(FA_WORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

    set(FA_ZIP "fontawesome-free-${FA_VERSION}-desktop.zip")
    set(FA_ZIP_FULL "${FA_WORK_DIR}/${FA_ZIP}")
    
    set(FA_OUTPUT_DIR "${FA_WORK_DIR}/fontawesome-free-${FA_VERSION}-desktop/otfs")

    set(FA_FILE_REGULAR "Font Awesome 6 Free-Regular-400.otf")
    set(FA_FILE_SOLID   "Font Awesome 6 Free-Solid-900.otf")
    
    set(FA_FILE_REGULAR_FULL "${FA_OUTPUT_DIR}/${FA_FILE_REGULAR}")
    set(FA_FILE_SOLID_FULL   "${FA_OUTPUT_DIR}/${FA_FILE_SOLID}")
    
    if (NOT EXISTS "${FA_ZIP_FULL}")
        file(DOWNLOAD
            "${FA_URL}"
            "${FA_ZIP_FULL}"
            SHOW_PROGRESS
            INACTIVITY_TIMEOUT 10
            STATUS download_result)

        list(GET download_result 0 status_code)
        list(GET download_result 1 error_message)

        if (NOT status_code EQUAL 0)
            file(REMOVE "${FA_ZIP_FULL}")
            message(FATAL_ERROR "Failed to download ${FA_URL}: ${error_message}")
        endif()
    endif()

    if (NOT EXISTS "${FA_FILE_REGULAR_FULL}")
        file(ARCHIVE_EXTRACT
            INPUT "${FA_ZIP_FULL}"
            DESTINATION "${FA_WORK_DIR}"
        )
    endif()

    configure_file("${FA_FILE_REGULAR_FULL}" "icon-regular.otf" COPYONLY)
    configure_file("${FA_FILE_SOLID_FULL}" "icon-solid.otf" COPYONLY)
endfunction()
