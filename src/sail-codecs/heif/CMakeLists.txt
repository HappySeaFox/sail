find_package(libheif CONFIG ${SAIL_CODEC_HEIF_REQUIRED_OPTION})

if (NOT libheif_FOUND)
    return()
endif()

# This will add the following CMake rules to the CMake config for static builds so a client
# application links against the required dependencies:
#
# find_dependency(libheif REQUIRED)
# set_property(TARGET SAIL::sail-codecs APPEND PROPERTY INTERFACE_LINK_LIBRARIES heif)
#
set(SAIL_CODECS_FIND_DEPENDENCIES ${SAIL_CODECS_FIND_DEPENDENCIES} "find_dependency,libheif,heif" PARENT_SCOPE)

# Check HEIF encoder backend bit depth support
# Depends on available backends (x265, kvazaar, etc.) and their configuration
#
cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LIBRARIES heif)

    # Test 8-bit support (always available)
    set(HEIF_CODEC_INFO_PIXEL_FORMATS_8BIT "BPP8-GRAYSCALE;BPP24-RGB;BPP32-RGBA;BPP24-YUV")

    # Test high bit depth support (10, 12, 16-bit)
    foreach(BIT_DEPTH 10 12 16)
        if(BIT_DEPTH EQUAL 16)
            set(TEST_COLORSPACE "heif_colorspace_monochrome")
            set(TEST_CHROMA "heif_chroma_monochrome")
        else()
            set(TEST_COLORSPACE "heif_colorspace_YCbCr")
            set(TEST_CHROMA "heif_chroma_444")
        endif()

        check_c_source_runs(
            "
            #include <libheif/heif.h>
            int main() {
                struct heif_context* ctx = heif_context_alloc();
                struct heif_encoder* encoder;
                heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);
                struct heif_image* img;
                heif_image_create(100, 100, ${TEST_COLORSPACE}, ${TEST_CHROMA}, &img);
                heif_image_add_plane(img, heif_channel_Y, 100, 100, ${BIT_DEPTH});
                struct heif_image_handle* handle;
                struct heif_error err = heif_context_encode_image(ctx, img, encoder, NULL, &handle);
                heif_image_release(img);
                heif_encoder_release(encoder);
                heif_context_free(ctx);
                return (err.code == heif_error_Ok) ? 0 : 1;
            }
            "
            HEIF_HAS_${BIT_DEPTH}BIT
        )
    endforeach()
cmake_pop_check_state()

# Build pixel format list based on detected bit depths
#
set(HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS "${HEIF_CODEC_INFO_PIXEL_FORMATS_8BIT}")

if (HEIF_HAS_10BIT)
    set(HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS "${HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS};BPP30-YUV")
endif()

if (HEIF_HAS_12BIT)
    set(HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS "${HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS};BPP36-YUV")
endif()

if (HEIF_HAS_16BIT)
    set(HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS "${HEIF_CODEC_INFO_SAVE_PIXEL_FORMATS};BPP16-GRAYSCALE;BPP48-RGB;BPP64-RGBA;BPP48-YUV")
endif()

# Common codec configuration
#
sail_codec(NAME heif
            SOURCES helpers.h helpers.c io_src.h io_src.c io_dest.h io_dest.c heif.c
            ICON heif.png
            DEPENDENCY_LIBS heif)
