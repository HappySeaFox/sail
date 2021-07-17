macro(sail_find_dependencies)
    find_package(TIFF)

    if (NOT TIFF_FOUND)
        return()
    endif()

    set(sail_tiff_include_dirs ${TIFF_INCLUDE_DIRS})
    set(sail_tiff_libs ${TIFF_LIBRARIES})

    # This will add the following CMake rules to the CMake config for static builds so a client
    # application links against the required dependencies:
    #
    # find_dependency(TIFF REQUIRED)
    # set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES TIFF::TIFF)
    #
    set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "TIFF,TIFF::TIFF" PARENT_SCOPE)
endmacro()

macro(sail_codec_post_add)
    set(TIFF_CODECS ADOBE_DEFLATE CCITTRLE CCITTRLEW CCITT_T4 CCITT_T6 DCS DEFLATE IT8BL IT8CTPAD IT8LW IT8MP
                    JBIG JPEG JP2000 LERC LZMA LZW NEXT NONE OJPEG PACKBITS PIXARFILM PIXARLOG SGILOG24 SGILOG
                    T43 T85 THUNDERSCAN WEBP ZSTD)

    foreach (tiff_codec IN LISTS TIFF_CODECS)
        # Check compression definitions
        #
        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_INCLUDES ${sail_tiff_include_dirs})

            check_c_source_compiles(
                "
                #include <tiff.h>

                int main(int argc, char *argv[]) {
                    int compression = COMPRESSION_${tiff_codec};
                    return 0;
                }
            "
            SAIL_HAVE_TIFF_${tiff_codec}
            )
        cmake_pop_check_state()

        if (SAIL_HAVE_TIFF_${tiff_codec})
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_${tiff_codec})
        endif()

        # Check if we can actually write defined compressions
        #
        if (SAIL_VCPKG)
            # Hardcode compressions as check_c_source_runs() fails in VCPKG mode.
            # It fails because it cannot find tiff.dll.
            #
            set(SAIL_TIFF_CODEC_INFO_COMPRESSIONS ADOBE-DEFLATE DEFLATE JPEG LZW NONE PACKBITS)
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_ADOBE_DEFLATE)
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_DEFLATE)
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_JPEG)
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_LZW)
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_NONE)
            target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_PACKBITS)
        else()
            cmake_push_check_state(RESET)
                set(CMAKE_REQUIRED_INCLUDES ${sail_tiff_include_dirs})
                set(CMAKE_REQUIRED_LIBRARIES ${sail_tiff_libs})

                check_c_source_runs(
                    "
                    #include <stdlib.h>
                    #include <string.h>
                    #include <tiffio.h>

                    int main(int argc, char *argv[]) {

                        TIFF *tiff = TIFFOpen(\"file.tiff\", \"w\");

                        if (tiff == NULL) {
                            return 1;
                        }

                        TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH,  1);
                        TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, 1);
                        TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
                        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
                        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);
                        TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
                        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
                        TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_${tiff_codec});
                        TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tiff, (uint32)-1));

                        unsigned char *scan = malloc(4);
                        memset(scan, 255, 4);

                        if (TIFFWriteScanline(tiff, scan, 0, 0) < 0) {
                            return 2;
                        }

                        TIFFClose(tiff);

                        free(scan);

                        return 0;
                    }
                "
                SAIL_HAVE_TIFF_WRITE_${tiff_codec}
                )
            cmake_pop_check_state()

            if (SAIL_HAVE_TIFF_WRITE_${tiff_codec})
                # Match the SAIL namings
                #
                string(REPLACE "_"         "-"          tiff_codec_fixed ${tiff_codec})
                string(REPLACE "CCITTRLE"  "CCITT-RLE"  tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "CCITTRLEW" "CCITT-RLEW" tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "IT8BL"     "IT8-BL"     tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "IT8CTPAD"  "IT8-CTPAD"  tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "IT8LW"     "IT8-LW"     tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "IT8MP"     "IT8-MP"     tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "JP2000"    "JPEG2000"   tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "PIXARLOG"  "PIXAR-LOG"  tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "PIXARFILM" "PIXAR-FILM" tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "SGILOG24"  "SGI-LOG24"  tiff_codec_fixed ${tiff_codec_fixed})
                string(REPLACE "SGILOG"    "SGI-LOG"    tiff_codec_fixed ${tiff_codec_fixed})
                list(APPEND SAIL_TIFF_CODEC_INFO_COMPRESSIONS ${tiff_codec_fixed})

                target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_${tiff_codec})
            endif()
        endif()
    endforeach()

    # Default compression
    #
    if (JPEG IN_LIST SAIL_TIFF_CODEC_INFO_COMPRESSIONS)
        set(SAIL_TIFF_CODEC_INFO_DEFAULT_COMPRESSION JPEG)
    else()
        set(SAIL_TIFF_CODEC_INFO_DEFAULT_COMPRESSION NONE)
    endif()
endmacro()
