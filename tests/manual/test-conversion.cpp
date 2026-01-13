/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2025 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/*
 * Massive testing tool to convert image formats.
 *
 * Usage: ./test-conversion.sh <path to directory with input images> <output image format> [number of threads]
 *
 * For example: ./test-conversion.sh ~/images/jpeg png 6
 */

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

#include <sail-c++.h>

/*
 * Constants and configuration
 */
namespace
{

constexpr size_t MAX_PATH_LENGTH                     = 2048;
constexpr size_t MAX_EXTENDED_PATH_LENGTH            = 2560;
constexpr size_t MAX_LONG_PATH_LENGTH                = 2580;
constexpr size_t MAX_TEMP_TEMPLATE_LENGTH            = 256;
constexpr size_t MAX_BASENAME_LENGTH                 = 256;
constexpr size_t MAX_CMD_LENGTH                      = 4096;
constexpr size_t MAX_RESULT_LENGTH                   = 256;
constexpr int INITIAL_FRAME_CAPACITY                 = 10;
constexpr int INITIAL_FILE_CAPACITY                  = 1000;
constexpr int MIN_THREADS                            = 1;
constexpr int MAX_THREADS                            = 16;
constexpr std::string_view TEMP_FILE_PREFIX          = "sail-test";
constexpr std::string_view TEMP_FILE_TEMPLATE_SUFFIX = "XXXXXX";
constexpr mode_t DEFAULT_DIR_PERMISSIONS             = 0755;

} // namespace

/*
 * Global state
 */
namespace
{

std::atomic<int> tests_passed{0};
std::atomic<int> tests_failed{0};
std::atomic<int> tests_expected_failed{0};
std::mutex print_mutex;
std::mutex log_mutex;
std::mutex files_mutex;
std::mutex global_counter_mutex;
int global_files_processed{0};
size_t total_files_count{0};
std::unique_ptr<std::ofstream> log_file;

} // namespace

/*
 * Data structures
 */
enum class TestResult
{
    PASSED,
    FAILED,
    EXPECTED_FAIL
};

struct ImageFrames
{
    std::vector<sail::image> frames;
    bool is_animated    = false;
    bool is_multi_paged = false;

    ImageFrames()  = default;
    ~ImageFrames() = default;

    ImageFrames(ImageFrames&&)            = default;
    ImageFrames& operator=(ImageFrames&&) = default;

    ImageFrames(const ImageFrames&)            = delete;
    ImageFrames& operator=(const ImageFrames&) = delete;

    size_t frame_count() const
    {
        return frames.size();
    }
};

struct FileEntry
{
    std::string path;
    std::string relative;

    FileEntry() = default;
    FileEntry(std::string_view p, std::string_view r)
        : path(p)
        , relative(r)
    {
    }
};

struct ThreadData
{
    std::string output_dir;
    std::string target_ext;
    int thread_id;
};

// Global queue for files
std::queue<FileEntry> files_queue;

/*
 * Thread-safe printing
 */
template <typename... Args> void safe_print(std::format_string<Args...> fmt, Args&&... args)
{
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << std::format(fmt, std::forward<Args>(args)...);
}

/*
 * Logging
 */
constexpr std::string_view get_log_level_string(enum SailLogLevel level)
{
    switch (level)
    {
    case SAIL_LOG_LEVEL_ERROR: return "E";
    case SAIL_LOG_LEVEL_WARNING: return "W";
    case SAIL_LOG_LEVEL_INFO: return "I";
    case SAIL_LOG_LEVEL_MESSAGE: return "M";
    case SAIL_LOG_LEVEL_DEBUG: return "D";
    case SAIL_LOG_LEVEL_TRACE: return "T";
    default: return "?";
    }
}

std::string_view get_filename_from_path(std::string_view path)
{
    auto pos = path.rfind('/');
    return pos == std::string_view::npos ? path : path.substr(pos + 1);
}

bool sail_log_callback(enum SailLogLevel level, const char* file, int line, const char* format, va_list args)
{
    if (!log_file)
    {
        return true;
    }

    std::lock_guard<std::mutex> lock(log_mutex);

    auto now = std::chrono::system_clock::now();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    auto level_str = get_log_level_string(level);
    auto filename  = get_filename_from_path(file);

    // Convert va_list to string for C++ stream
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), format, args);

    *log_file << std::format("[{}] [{:%H:%M:%S}.{:03d}] [{}:{}] {}\n", level_str,
                             std::chrono::floor<std::chrono::seconds>(now), ms.count(), filename, line, buffer);
    log_file->flush();

    return true;
}

/*
 * Filesystem utilities
 */
bool create_directory_recursive(const std::string& path)
{
    try
    {
        std::filesystem::create_directories(path);
        return true;
    }
    catch (const std::filesystem::filesystem_error&)
    {
        return false;
    }
}

/*
 * Image frame management
 */
void detect_frame_type(ImageFrames& frames)
{
    if (frames.frame_count() <= 1)
    {
        return;
    }

    bool has_positive_delay = false;
    for (const auto& frame : frames.frames)
    {
        if (frame.delay() >= 0)
        {
            has_positive_delay = true;
            break;
        }
    }

    frames.is_animated    = has_positive_delay;
    frames.is_multi_paged = !has_positive_delay;
}

sail_status_t load_all_frames(const std::string& input_path, ImageFrames& result)
{
    result.frames.clear();
    result.is_animated    = false;
    result.is_multi_paged = false;

    try
    {
        sail::image_input input(input_path);
        result.frames.reserve(INITIAL_FRAME_CAPACITY);

        sail::image image;
        while (input.next_frame(&image) == SAIL_OK)
        {
            result.frames.push_back(std::move(image));
        }

        detect_frame_type(result);
        return SAIL_OK;
    }
    catch (const std::exception&)
    {
        result.frames.clear();
        return SAIL_ERROR_INVALID_IMAGE;
    }
}

/*
 * Image comparison
 */
bool compare_pixels_rgb(const sail::image& img1, const sail::image& img2)
{
    if (!img1.is_valid() || !img2.is_valid())
    {
        return false;
    }

    try
    {
        auto rgb1 = img1.convert_to(SAIL_PIXEL_FORMAT_BPP24_RGB);
        auto rgb2 = img2.convert_to(SAIL_PIXEL_FORMAT_BPP24_RGB);

        if (!rgb1.is_valid() || !rgb2.is_valid())
        {
            return false;
        }

        for (unsigned row = 0; row < rgb1.height(); row++)
        {
            auto* row1 = static_cast<const unsigned char*>(rgb1.scan_line(row));
            auto* row2 = static_cast<const unsigned char*>(rgb2.scan_line(row));

            if (std::memcmp(row1, row2, rgb1.bytes_per_line()) != 0)
            {
                return false;
            }
        }

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool compare_pixels_direct(const sail::image& img1, const sail::image& img2)
{
    if (!img1.is_valid() || !img2.is_valid())
    {
        return false;
    }

    for (unsigned row = 0; row < img1.height(); row++)
    {
        auto* row1 = static_cast<const unsigned char*>(img1.scan_line(row));
        auto* row2 = static_cast<const unsigned char*>(img2.scan_line(row));

        if (std::memcmp(row1, row2, img1.bytes_per_line()) != 0)
        {
            return false;
        }
    }
    return true;
}

bool compare_pixels(const sail::image& img1, const sail::image& img2)
{
    if (!img1.is_valid() || !img2.is_valid())
    {
        return false;
    }

    if (img1.width() != img2.width() || img1.height() != img2.height())
    {
        return false;
    }

    if (img1.pixel_format() != img2.pixel_format())
    {
        return false;
    }

    // For indexed formats, compare actual colors, not indices
    if (img1.is_indexed())
    {
        return compare_pixels_rgb(img1, img2);
    }

    return compare_pixels_direct(img1, img2);
}

/*
 * Codec utilities
 */
bool is_lossy_codec(std::string_view codec_name, enum SailPixelFormat pixel_format)
{
    static constexpr std::array<std::string_view, 11> lossy_codecs = {
        "jpeg", "jpg", "jp2", "jpeg2000", "webp", "avif", "jxl", "jpegxl", "heif", "heic", "hif"};

    for (const auto& codec : lossy_codecs)
    {
        if (std::equal(codec_name.begin(), codec_name.end(), codec.begin(), codec.end(),
                       [](char a, char b) { return std::tolower(a) == std::tolower(b); }))
        {
            return true;
        }
    }

    // Lossy pixel formats due to color space conversion or palette quantization
    if (sail::image::is_indexed(pixel_format) || pixel_format == SAIL_PIXEL_FORMAT_BPP24_YCBCR
        || pixel_format == SAIL_PIXEL_FORMAT_BPP24_CIE_LAB)
    {
        return true;
    }

    return false;
}

bool is_pixel_format_supported(enum SailPixelFormat format, const sail::codec_info& codec_info)
{
    if (!codec_info.is_valid())
    {
        return false;
    }

    const auto& formats = codec_info.save_features().pixel_formats();
    return std::ranges::find(formats, format) != formats.end();
}

/*
 * Image saving utilities
 */
sail_status_t save_image_to_file(const sail::image& image,
                                 const std::string& output_path,
                                 const sail::codec_info& codec_info)
{
    try
    {
        sail::image_output output(output_path);
        output.with(codec_info);
        return output.next_frame(image);
    }
    catch (const std::exception&)
    {
        return SAIL_ERROR_EOF;
    }
}

/*
 * ImageMagick comparison
 */
constexpr double LOSSY_THRESHOLD_PERCENT = 8.0; // Maximum 8% pixel difference for lossy codecs

int run_imagemagick_compare(const std::string& file1, const std::string& file2)
{
    auto cmd = "compare -metric AE '" + file1 + "' '" + file2 + "' null: 2>&1";

    FILE* fp = popen(cmd.c_str(), "r");
    if (!fp)
    {
        throw std::runtime_error("Failed to run ImageMagick compare");
    }

    std::string result;
    char buffer[MAX_RESULT_LENGTH];
    if (fgets(buffer, sizeof(buffer), fp))
    {
        result = buffer;
    }
    pclose(fp);

    if (result.empty())
    {
        return -1;
    }

    // Remove trailing whitespace and newlines
    result.erase(result.find_last_not_of(" \t\n\r") + 1);

    // Check if result contains only digits
    if (result.empty() || !std::all_of(result.begin(), result.end(), ::isdigit))
    {
        SAIL_LOG_DEBUG("TEST: ImageMagick compare returned non-numeric result: '%s'", result.c_str());
        return -1;
    }

    try
    {
        return std::stoi(result);
    }
    catch (const std::exception& e)
    {
        SAIL_LOG_DEBUG("TEST: Failed to parse ImageMagick result '%s': %s", result.c_str(), e.what());
        return -1;
    }
}

bool validate_with_imagemagick(const sail::image& converted_image,
                               const std::string& output_path,
                               const sail::codec_info& codec_info,
                               const std::string& target_ext,
                               bool is_lossy)
{
    char* temp_file = nullptr;
    if (sail_temp_file_path("sail_imagemagick_compare", &temp_file) != SAIL_OK)
    {
        return false;
    }

    auto temp_template = std::string(temp_file) + "." + target_ext;
    sail_free(temp_file);

    auto status = save_image_to_file(converted_image, temp_template, codec_info);
    if (status != SAIL_OK)
    {
        std::filesystem::remove(temp_template);
        return false;
    }

    auto diff_pixels = run_imagemagick_compare(temp_template, output_path);

    std::filesystem::remove(temp_template);

    if (diff_pixels < 0)
    {
        // ImageMagick not available or failed
        return true;
    }

    // Calculate percentage of different pixels
    double total_pixels = static_cast<double>(converted_image.width()) * converted_image.height();
    double diff_percent = (diff_pixels / total_pixels) * 100.0;

    SAIL_LOG_DEBUG("TEST: ImageMagick compare: %d different pixels, %.2f%% of total", diff_pixels, diff_percent);

    // For lossless codecs, require perfect match
    if (!is_lossy)
    {
        if (diff_pixels > 0)
        {
            SAIL_LOG_ERROR("TEST: Lossless codec has pixel differences: %.2f%%", diff_percent);
            return false;
        }
        return true;
    }

    // For lossy codecs, check against threshold
    if (diff_percent > LOSSY_THRESHOLD_PERCENT)
    {
        SAIL_LOG_ERROR("TEST: Lossy codec difference %.2f%% exceeds threshold %.2f%%", diff_percent,
                       LOSSY_THRESHOLD_PERCENT);
        return false;
    }

    SAIL_LOG_DEBUG("TEST: Lossy codec difference %.2f%% is within threshold %.2f%%", diff_percent,
                   LOSSY_THRESHOLD_PERCENT);
    return true;
}

/*
 * Test functions
 */
TestResult test_static_conversion(const sail::image& source_frame,
                                  enum SailPixelFormat target_format,
                                  const std::string& output_path,
                                  const sail::codec_info& codec_info,
                                  const std::string& target_ext)
{
    auto is_lossy = is_lossy_codec(target_ext, target_format);

    if (source_frame.is_valid())
    {
        SAIL_LOG_DEBUG("TEST: Static conversion [%s → %s] (%s)",
                       sail_pixel_format_to_string(source_frame.pixel_format()),
                       sail_pixel_format_to_string(target_format), is_lossy ? "lossy" : "lossless");
    }
    else
    {
        SAIL_LOG_DEBUG("TEST: Invalid source frame");
        return TestResult::FAILED;
    }

    if (!is_pixel_format_supported(target_format, codec_info))
    {
        SAIL_LOG_DEBUG("TEST: Format not supported by codec");
        return TestResult::EXPECTED_FAIL;
    }

    SAIL_LOG_DEBUG("TEST: Converting from %s to %s", sail_pixel_format_to_string(source_frame.pixel_format()),
                   sail_pixel_format_to_string(target_format));
    sail::image converted_image = source_frame.convert_to(target_format);
    auto status                 = converted_image.is_valid() ? SAIL_OK : SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT;

    if (status != SAIL_OK)
    {
        SAIL_LOG_DEBUG("TEST: Conversion failed: error %d", status);
        return (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT) ? TestResult::EXPECTED_FAIL : TestResult::FAILED;
    }

    status = save_image_to_file(converted_image, output_path, codec_info);
    if (status != SAIL_OK)
    {
        SAIL_LOG_DEBUG("TEST: Failed to save: error %d", status);
        // Treat unsupported formats/bit depths as expected failures
        if (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT || status == SAIL_ERROR_UNSUPPORTED_BIT_DEPTH
            || status == SAIL_ERROR_UNSUPPORTED_IMAGE_PROPERTY)
        {
            return TestResult::EXPECTED_FAIL;
        }
        return TestResult::FAILED;
    }

    sail::image reloaded_image(output_path);
    if (!reloaded_image.is_valid())
    {
        SAIL_LOG_DEBUG("TEST: Failed to reload image");
        return TestResult::FAILED;
    }

    // Normalize pixel format for comparison
    if (reloaded_image.pixel_format() != target_format)
    {
        sail::image normalized_image = reloaded_image.convert_to(target_format);
        if (!normalized_image.is_valid())
        {
            return TestResult::FAILED;
        }
        reloaded_image = std::move(normalized_image);
    }

    // Check dimensions always match
    if (converted_image.width() != reloaded_image.width() || converted_image.height() != reloaded_image.height()
        || converted_image.pixel_format() != reloaded_image.pixel_format())
    {
        SAIL_LOG_ERROR("TEST: Dimension/format mismatch after reload (w:%u/%u h:%u/%u fmt:%s/%s)",
                       converted_image.width(), reloaded_image.width(), converted_image.height(),
                       reloaded_image.height(), sail_pixel_format_to_string(converted_image.pixel_format()),
                       sail_pixel_format_to_string(reloaded_image.pixel_format()));
        return TestResult::FAILED;
    }

    // For lossless codecs, do pixel-perfect comparison
    if (!is_lossy)
    {
        bool pixels_match = compare_pixels(converted_image, reloaded_image);

        if (!pixels_match)
        {
            SAIL_LOG_ERROR("TEST: Lossless codec has pixel differences");
            return TestResult::FAILED;
        }
    }

    // Validation through ImageMagick (for both lossy and lossless)
    auto imagemagick_ok = validate_with_imagemagick(converted_image, output_path, codec_info, target_ext, is_lossy);

    return imagemagick_ok ? TestResult::PASSED : TestResult::FAILED;
}

TestResult save_all_frames(const std::vector<sail::image>& frames,
                           const std::string& output_path,
                           const sail::codec_info& codec_info)
{
    try
    {
        sail::image_output output(output_path);
        output.with(codec_info);

        for (const auto& frame : frames)
        {
            auto status = output.next_frame(frame);
            if (status != SAIL_OK)
            {
                return (status == SAIL_ERROR_UNSUPPORTED_PIXEL_FORMAT) ? TestResult::EXPECTED_FAIL : TestResult::FAILED;
            }
        }

        return TestResult::PASSED;
    }
    catch (const std::exception&)
    {
        return TestResult::FAILED;
    }
}

TestResult test_multiframe_conversion(const ImageFrames& source_frames,
                                      enum SailPixelFormat target_format,
                                      const std::string& output_path,
                                      const sail::codec_info& codec_info,
                                      const std::string& target_ext)
{
    sail_status_t status;
    auto is_lossy = is_lossy_codec(target_ext, target_format);

    SAIL_LOG_DEBUG("TEST: Multi-frame conversion [%zu frames, %s → %s] (%s)", source_frames.frame_count(),
                   sail_pixel_format_to_string(source_frames.frames[0].pixel_format()),
                   sail_pixel_format_to_string(target_format), is_lossy ? "lossy" : "lossless");

    if (!is_pixel_format_supported(target_format, codec_info))
    {
        SAIL_LOG_DEBUG("TEST: Format not supported by codec");
        return TestResult::EXPECTED_FAIL;
    }

    std::vector<sail::image> converted_frames;
    converted_frames.reserve(source_frames.frame_count());

    for (const auto& frame : source_frames.frames)
    {
        sail::image converted = frame.convert_to(target_format);
        if (!converted.is_valid())
        {
            return TestResult::EXPECTED_FAIL;
        }
        converted.set_delay(frame.delay());
        converted_frames.push_back(std::move(converted));
    }

    TestResult save_result = save_all_frames(converted_frames, output_path, codec_info);

    if (save_result != TestResult::PASSED)
    {
        return save_result;
    }

    ImageFrames reloaded_frames;
    status = load_all_frames(output_path, reloaded_frames);
    if (status != SAIL_OK)
    {
        return TestResult::FAILED;
    }

    if (reloaded_frames.frame_count() != source_frames.frame_count())
    {
        SAIL_LOG_ERROR("TEST: Frame count mismatch: %zu vs %zu", converted_frames.size(),
                       reloaded_frames.frame_count());
        return TestResult::FAILED;
    }

    // For lossless codecs, require pixel-perfect match
    if (!is_lossy)
    {
        for (size_t i = 0; i < converted_frames.size(); i++)
        {
            if (!compare_pixels(converted_frames[i], reloaded_frames.frames[i]))
            {
                SAIL_LOG_ERROR("TEST: Lossless codec has pixel differences in frame %zu", i);
                return TestResult::FAILED;
            }
        }
    }

    return TestResult::PASSED;
}

TestResult test_separate_frames(const ImageFrames& source_frames,
                                enum SailPixelFormat target_format,
                                const std::string& output_base_path,
                                const sail::codec_info& codec_info,
                                const std::string& target_ext)
{
    auto passed = 0;
    auto failed = 0;

    SAIL_LOG_DEBUG("TEST: Separate frames [%zu frames, %s → %s]", source_frames.frame_count(),
                   sail_pixel_format_to_string(source_frames.frames[0].pixel_format()),
                   sail_pixel_format_to_string(target_format));

    for (size_t i = 0; i < source_frames.frame_count(); i++)
    {
        auto output_path = output_base_path + "_frame"
                           + std::to_string(i).insert(0, 3 - std::to_string(i).length(), '0') + "." + target_ext;

        TestResult result =
            test_static_conversion(source_frames.frames[i], target_format, output_path, codec_info, target_ext);

        if (result == TestResult::PASSED)
        {
            passed++;
        }
        else if (result != TestResult::EXPECTED_FAIL)
        {
            failed++;
        }
    }

    if (failed > 0)
    {
        return TestResult::FAILED;
    }
    else if (passed > 0)
    {
        return TestResult::PASSED;
    }
    else
    {
        return TestResult::EXPECTED_FAIL;
    }
}

/*
 * Test execution
 */
std::string build_output_path(const std::string& output_base_dir,
                              const std::string& format_name,
                              const std::string& relative_path)
{
    if (relative_path.empty())
    {
        return output_base_dir + "/" + format_name;
    }
    else
    {
        return output_base_dir + "/" + format_name + "/" + relative_path;
    }
}

std::string get_basename_from_path(const std::string& input_path)
{
    auto pos = input_path.rfind('/');
    return pos == std::string::npos ? input_path : input_path.substr(pos + 1);
}

TestResult run_test_for_format(const ImageFrames& source_frames,
                               enum SailPixelFormat target_format,
                               const std::string& output_base_path,
                               const sail::codec_info& codec_info,
                               const std::string& target_ext,
                               bool supports_animated,
                               bool supports_multi_paged,
                               int& test_count,
                               int& test_passed,
                               int& test_expected_failed)
{
    TestResult result;

    // Test 1: Static conversion
    auto test_path = output_base_path + "_static." + target_ext;
    result         = test_static_conversion(source_frames.frames[0], target_format, test_path, codec_info, target_ext);

    SAIL_LOG_DEBUG("TEST: Static test result: %s", (result == TestResult::PASSED)          ? "PASSED"
                                                   : (result == TestResult::EXPECTED_FAIL) ? "EXPECTED_FAIL"
                                                                                           : "FAILED");

    if (result == TestResult::PASSED)
    {
        test_passed++;
    }
    else if (result == TestResult::EXPECTED_FAIL)
    {
        test_expected_failed++;
    }
    test_count++;

    // Test 2: Multi-frame conversion
    if (source_frames.frame_count() > 1)
    {
        if ((source_frames.is_animated && supports_animated) || (source_frames.is_multi_paged && supports_multi_paged))
        {
            test_path = output_base_path + "_multiframe." + target_ext;
            result    = test_multiframe_conversion(source_frames, target_format, test_path, codec_info, target_ext);

            SAIL_LOG_DEBUG("TEST: Multi-frame test result: %s", (result == TestResult::PASSED) ? "PASSED"
                                                                : (result == TestResult::EXPECTED_FAIL)
                                                                    ? "EXPECTED_FAIL"
                                                                    : "FAILED");

            if (result == TestResult::PASSED)
            {
                test_passed++;
            }
            else if (result == TestResult::EXPECTED_FAIL)
            {
                test_expected_failed++;
            }
        }
        else
        {
            SAIL_LOG_DEBUG("TEST: Multi-frame test (skipped, codec doesn't support animation/multi-page)");
            test_expected_failed++;
        }
        test_count++;

        // Test 3: Separate frames
        result = test_separate_frames(source_frames, target_format, output_base_path, codec_info, target_ext);

        SAIL_LOG_DEBUG("TEST: Separate frames test result: %s", (result == TestResult::PASSED) ? "PASSED"
                                                                : (result == TestResult::EXPECTED_FAIL)
                                                                    ? "EXPECTED_FAIL"
                                                                    : "FAILED");

        if (result == TestResult::PASSED)
        {
            test_passed++;
        }
        else if (result == TestResult::EXPECTED_FAIL)
        {
            test_expected_failed++;
        }
        test_count++;
    }

    return (test_passed == test_count) ? TestResult::PASSED
           : (test_passed > 0)         ? TestResult::PASSED
                                       : TestResult::EXPECTED_FAIL;
}

bool test_file(const std::string& input_path,
               const std::string& output_base_dir,
               const std::string& relative_path,
               const std::string& target_ext)
{
    sail_status_t status;

    ImageFrames source_frames;
    status = load_all_frames(input_path, source_frames);
    if (status != SAIL_OK)
    {
        SAIL_LOG_ERROR("TEST: Failed to load: error %d", status);
        return false;
    }

    SAIL_LOG_DEBUG("TEST: Loaded %zu frames", source_frames.frame_count());

    sail::codec_info codec_info = sail::codec_info::from_extension(target_ext);
    if (!codec_info.is_valid())
    {
        SAIL_LOG_ERROR("TEST: Failed to get codec info");
        return false;
    }

    if (codec_info.save_features().pixel_formats().empty())
    {
        SAIL_LOG_ERROR("TEST: Codec has no supported pixel formats for saving");
        return false;
    }

    bool supports_animated    = (codec_info.save_features().features() & SAIL_CODEC_FEATURE_ANIMATED) != 0;
    bool supports_multi_paged = (codec_info.save_features().features() & SAIL_CODEC_FEATURE_MULTI_PAGED) != 0;

    if (source_frames.frame_count() > 0 && source_frames.frames[0].is_valid())
    {
        SAIL_LOG_DEBUG("TEST: Image: %ux%u %s, %zu frame(s)%s%s", source_frames.frames[0].width(),
                       source_frames.frames[0].height(),
                       sail_pixel_format_to_string(source_frames.frames[0].pixel_format()), source_frames.frame_count(),
                       source_frames.is_animated ? " [animated]" : "",
                       source_frames.is_multi_paged ? " [multi-paged]" : "");
    }
    else
    {
        SAIL_LOG_DEBUG("TEST: Invalid image loaded");
        return false;
    }

    auto passed                  = 0;
    auto failed                  = 0;
    auto expected_failed_formats = 0;

    const auto& formats = codec_info.save_features().pixel_formats();

    for (const auto& target_format : formats)
    {
        const char* format_name = sail_pixel_format_to_string(target_format);

        SAIL_LOG_DEBUG("TEST: Testing format %s", format_name);

        auto output_dir = build_output_path(output_base_dir, format_name, relative_path);
        create_directory_recursive(output_dir);

        auto basename         = get_basename_from_path(input_path);
        auto output_base_path = output_dir + "/" + basename;

        auto test_count           = 0;
        auto test_passed          = 0;
        auto test_expected_failed = 0;

        TestResult format_result =
            run_test_for_format(source_frames, target_format, output_base_path, codec_info, target_ext,
                                supports_animated, supports_multi_paged, test_count, test_passed, test_expected_failed);

        SAIL_LOG_DEBUG("TEST: Format result: %d passed, %d expected fail, %d failed (total %d)", test_passed,
                       test_expected_failed, test_count - test_passed - test_expected_failed, test_count);

        if (format_result == TestResult::PASSED)
        {
            passed++;
        }
        else if (test_expected_failed == test_count)
        {
            expected_failed_formats++;
        }
        else
        {
            failed++;
        }
    }

    SAIL_LOG_DEBUG("TEST: File summary: %d passed, %d expected fail, %d failed (total %zu formats)", passed,
                   expected_failed_formats, failed, formats.size());

    tests_passed.fetch_add(passed);
    tests_failed.fetch_add(failed);
    tests_expected_failed.fetch_add(expected_failed_formats);

    return passed > 0;
}

/*
 * File collection
 */
void collect_files(const std::string& dir_path, std::vector<FileEntry>& files)
{
    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path))
        {
            if (entry.is_regular_file())
            {
                auto full_path          = entry.path().string();
                auto relative_file_path = std::filesystem::relative(entry.path(), dir_path).string();

                sail::codec_info codec_info = sail::codec_info::from_path(full_path);
                if (codec_info.is_valid())
                {
                    files.emplace_back(full_path, relative_file_path);
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        SAIL_LOG_ERROR("Filesystem error: %s", ex.what());
    }
}

/*
 * Threading
 */
void worker_thread(ThreadData&& data)
{
    while (true)
    {
        FileEntry file_entry;

        {
            std::lock_guard<std::mutex> lock(files_mutex);
            if (files_queue.empty())
            {
                safe_print("[Thread {}] Finished\n", data.thread_id);
                break;
            }
            else
            {
                file_entry = files_queue.front();
                files_queue.pop();
            }
        }

        // Get global counter
        int global_processed;
        {
            std::lock_guard<std::mutex> lock(global_counter_mutex);
            global_processed = ++global_files_processed;
        }

        safe_print("[Thread {}] [{}/{}] [✓ {} ✓ {} {} {}] [{}] Processing {}\n", data.thread_id, global_processed,
                   total_files_count, tests_passed.load(), tests_expected_failed.load(),
                   (tests_failed.load() ? "✗" : "✓"), tests_failed.load(), data.target_ext, file_entry.path);

        try
        {
            test_file(file_entry.path, data.output_dir, file_entry.relative, data.target_ext);
        }
        catch (const std::exception& e)
        {
            safe_print("[Thread {}] ERROR: Exception in test_file: {}\n", data.thread_id, e.what());
            tests_failed.fetch_add(1);
        }
    }
}

/*
 * Statistics printing
 */
void print_statistics(int passed, int failed, int expected_failed)
{
    std::cout << "\n";
    std::cout << "=======================\n";
    std::cout << std::format(" {}  FINAL RESULTS  {}\n", failed > 0 ? "❌" : "✅", failed > 0 ? "❌" : "✅");
    std::cout << "=======================\n";
    std::cout << std::format("Total formats tested: {}\n", passed + failed + expected_failed);
    std::cout << std::format("Passed: {}\n", passed);
    std::cout << std::format("Expected fail: {}\n", expected_failed);
    std::cout << std::format("Failed: {}\n", failed);

    if (passed + failed + expected_failed > 0)
    {
        std::cout << std::format("Success rate (all): {:.1f}%\n", 100.0 * passed / (passed + failed + expected_failed));
    }

    if (passed + failed > 0)
    {
        double success_rate = 100.0 * passed / (passed + failed);
        std::cout << std::format("Success rate (excl. expected): {:.1f}%\n", success_rate);
    }
}

void print_test_plan(const std::string& target_ext)
{
    std::cout << "Tests for each file:\n";
    std::cout << "   1. Static frame conversion (first frame only)\n";
    std::cout << "   2. Multi-frame conversion (all frames, if animated)\n";
    std::cout << "   3. Separate frames conversion (each frame to separate file, if animated)\n";

    if (!is_lossy_codec(target_ext, SAIL_PIXEL_FORMAT_UNKNOWN))
    {
        std::cout << "   4. Pixel-perfect comparison (static & multi-frame)\n";
    }
    else
    {
        std::cout << "   4. Pixel comparison skipped (lossy codec)\n";
    }

    std::cout << "\n";
    std::cout << "Status report: (✓ PASSED ✓ EXPECTED FAILED ✓|✗ FAILED)\n";
    std::cout << "\n";
}

/*
 * Input validation
 */
bool validate_target_codec(const std::string& target_ext)
{
    sail::codec_info target_codec_info = sail::codec_info::from_extension(target_ext);

    if (!target_codec_info.is_valid())
    {
        std::cerr << std::format("Error: Unknown output format '{}'\n", target_ext);
        std::cerr << "Use 'sail list' to see supported formats\n";
        return false;
    }

    if (target_codec_info.save_features().pixel_formats().empty())
    {
        std::cerr << std::format("Error: Format '{}' does not support saving\n", target_ext);
        std::cerr << "Use 'sail list' to see formats that support writing\n";
        return false;
    }

    std::cout << std::format("Target codec: {} [{}] v{}\n", target_codec_info.name(), target_codec_info.description(),
                             target_codec_info.version());
    std::cout << std::format("   Supports {} pixel formats for saving\n\n",
                             target_codec_info.save_features().pixel_formats().size());

    return true;
}

int collect_input_files(const std::string& input_path, std::vector<FileEntry>& files)
{
    try
    {
        std::filesystem::path path(input_path);

        if (!std::filesystem::exists(path))
        {
            std::cerr << std::format("Error: Cannot access '{}': Path does not exist\n", input_path);
            return -1;
        }

        if (std::filesystem::is_regular_file(path))
        {
            sail::codec_info codec_info = sail::codec_info::from_path(input_path);
            if (codec_info.is_valid())
            {
                files.emplace_back(input_path, "");
            }
            else
            {
                std::cerr << std::format("Error: File '{}' is not a supported image format\n", input_path);
                return -1;
            }
        }
        else if (std::filesystem::is_directory(path))
        {
            std::cout << "Collecting files...\n";
            collect_files(input_path, files);
        }
        else
        {
            std::cerr << std::format("Error: '{}' is neither a file nor a directory\n", input_path);
            return -1;
        }
    }
    catch (const std::filesystem::filesystem_error& ex)
    {
        std::cerr << std::format("Error: Filesystem error accessing '{}': {}\n", input_path, ex.what());
        return -1;
    }

    std::cout << std::format("Collected {} files\n\n", files.size());
    return 0;
}

/*
 * Logging setup
 */
void setup_logging(const std::string& output_dir,
                   const std::string& input_path,
                   const std::string& target_ext,
                   int num_threads)
{
    auto log_path = output_dir + "/sail-debug.log";

    log_file = std::make_unique<std::ofstream>(log_path);
    if (log_file->is_open())
    {
        sail::log::set_logger(sail_log_callback);
        sail::log::set_barrier(SAIL_LOG_LEVEL_DEBUG);
        *log_file << "=== SAIL Test Conversion Log ===\n";
        *log_file << "Input: " << input_path << "\n";
        *log_file << "Target: " << target_ext << "\n";
        *log_file << "Threads: " << num_threads << "\n";
        *log_file << "===================================\n\n";
        std::cout << std::format("Debug log: {}\n", log_path);
    }
    else
    {
        throw std::runtime_error("Failed to open log file");
    }
}

/*
 * Main program
 */
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << std::format("Usage: {} <input_path> <output_format> [num_threads]\n", argv[0]);
        std::cerr << "  input_path: path to image file or directory\n";
        std::cerr << "  output_format: file extension (e.g., jp2, png, webp, jxl)\n";
        return 1;
    }

    auto input_path = argv[1];
    auto target_ext = argv[2];
    int num_threads = (argc >= 4) ? std::atoi(argv[3]) : 1;

    num_threads = std::clamp(num_threads, MIN_THREADS, MAX_THREADS);

    char* temp_file_path = nullptr;
    sail_status_t status = sail_temp_file_path("sail_test_dir", &temp_file_path);
    if (status != SAIL_OK)
    {
        std::cerr << "Error: Failed to create temporary file path\n";
        return 1;
    }

    // Create a directory with the temp file path as base
    std::string output_dir_template = std::string(temp_file_path) + "_" + target_ext + "_XXXXXX";
    sail_free(temp_file_path);

    auto* output_dir = mkdtemp(output_dir_template.data());
    if (output_dir == nullptr)
    {
        std::cerr << std::format("Error: Failed to create temporary directory: {}\n", std::strerror(errno));
        return 1;
    }

    std::cout << "Starting image format conversion test\n";
    std::cout << std::format("Input: {}\n", input_path);
    std::cout << std::format("Output: {}\n", output_dir);
    std::cout << std::format("Target format: {}\n", target_ext);
    std::cout << std::format("Threads: {}\n", num_threads);

    setup_logging(output_dir, input_path, target_ext, num_threads);
    std::cout << "\n";

    if (!validate_target_codec(target_ext))
    {
        return 1;
    }

    std::vector<FileEntry> files;
    if (collect_input_files(input_path, files) != 0)
    {
        return 1;
    }

    if (files.empty())
    {
        std::cout << "No files to process\n";
        return 0;
    }

    print_test_plan(target_ext);

    // Fill the queue with files
    for (const auto& file : files)
    {
        files_queue.push(file);
    }
    total_files_count = files.size();

    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; i++)
    {
        threads.emplace_back(worker_thread, ThreadData{
                                                .output_dir = std::string(output_dir),
                                                .target_ext = std::string(target_ext),
                                                .thread_id  = i,
                                            });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    std::cout << "All threads completed\n";

    int passed          = tests_passed.load();
    int failed          = tests_failed.load();
    int expected_failed = tests_expected_failed.load();

    print_statistics(passed, failed, expected_failed);

    log_file.reset();

    return failed;
}
