find_package(TIFF)

if (NOT TIFF_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(TIFF REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES TIFF::TIFF)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,TIFF,TIFF::TIFF" PARENT_SCOPE)

# Check for TIFF features
set(TIFF_CODECS ADOBE_DEFLATE CCITTRLE CCITTRLEW CCITT_T4 CCITT_T6 DCS DEFLATE IT8BL IT8CTPAD IT8LW IT8MP
                JBIG JPEG JP2000 JXL LERC LZMA LZW NEXT NONE OJPEG PACKBITS PIXARFILM PIXARLOG SGILOG24 SGILOG
                T43 T85 THUNDERSCAN WEBP ZSTD)

foreach (tiff_codec IN LISTS TIFF_CODECS)
    # Check compression definitions
    #
    cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_INCLUDES ${TIFF_INCLUDE_DIRS})

        check_c_source_compiles(
            "
            #include <tiff.h>

            int main(int argc, char *argv[]) {
                int compression = COMPRESSION_${tiff_codec};
                return 0;
            }
        "
        HAVE_TIFF_${tiff_codec}
        )
    cmake_pop_check_state()

    # Check if we can actually save defined compressions
    #
    if (SAIL_VCPKG OR CMAKE_CROSSCOMPILING)
        # Enable compression and hope for the best. The reason is that check_c_source_runs()
        # fails in VCPKG or cross-compilation mode.
        #
        # For VCPKG, it fails as it cannot find tiff.dll, see https://github.com/microsoft/vcpkg/issues/16793
        #
        message("-- Skipping Test and Explicitly Enabling HAVE_TIFF_WRITE_${tiff_codec}")
        set(HAVE_TIFF_WRITE_${tiff_codec} ON)
    else()
        cmake_push_check_state(RESET)
            set(CMAKE_REQUIRED_INCLUDES ${TIFF_INCLUDE_DIRS})
            set(CMAKE_REQUIRED_LIBRARIES ${TIFF_LIBRARIES})

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

                    /*
                     * This check is not quite correct as some codecs may not support
                     * these parameters. For example, JBIG doesn't support RGBA.
                     */
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
            HAVE_TIFF_WRITE_${tiff_codec}
            )
        cmake_pop_check_state()

        if (HAVE_TIFF_WRITE_${tiff_codec})
            # Match the SAIL namings
            #
            string(REPLACE "_"         "-"          tiff_codec_fixed ${tiff_codec})
            string(REPLACE "CCITTRLE"  "CCITT-RLE"  tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "CCITTRLEW" "CCITT-RLEW" tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "IT8BL"     "IT8-BL"     tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "IT8CTPAD"  "IT8-CTPAD"  tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "IT8LW"     "IT8-LW"     tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "IT8MP"     "IT8-MP"     tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "JP2000"    "JPEG-2000"  tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "JXL"       "JPEG-XL"    tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "PIXARLOG"  "PIXAR-LOG"  tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "PIXARFILM" "PIXAR-FILM" tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "SGILOG24"  "SGI-LOG24"  tiff_codec_fixed ${tiff_codec_fixed})
            string(REPLACE "SGILOG"    "SGI-LOG"    tiff_codec_fixed ${tiff_codec_fixed})

            # Used in .codec.info
            #
            list(APPEND TIFF_CODEC_INFO_COMPRESSIONS ${tiff_codec_fixed})
        endif()
    endif()
endforeach()

# Default compression. Used in .codec.info
#
if (JPEG IN_LIST TIFF_CODEC_INFO_COMPRESSIONS)
    set(TIFF_CODEC_INFO_DEFAULT_COMPRESSION JPEG)
else()
    set(TIFF_CODEC_INFO_DEFAULT_COMPRESSION NONE)
endif()

# Common codec configuration
#
sail_codec(NAME tiff
            SOURCES helpers.h helpers.c io.h io.c tiff.c
            ICON tiff.png
            DEPENDENCY_INCLUDE_DIRS ${TIFF_INCLUDE_DIRS}
            DEPENDENCY_LIBS ${TIFF_LIBRARIES})

foreach (tiff_codec IN LISTS TIFF_CODECS)
    if (HAVE_TIFF_${tiff_codec})
        target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_${tiff_codec})
    endif()

    if (HAVE_TIFF_WRITE_${tiff_codec})
        target_compile_definitions(${TARGET} PRIVATE SAIL_HAVE_TIFF_WRITE_${tiff_codec})
    endif()
endforeach()
