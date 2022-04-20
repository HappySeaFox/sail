/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

    Copyright (c) 2021 Dmitry Baryshev

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

#include <utility>

#include "sail-c++.h"

#include "munit.h"

static MunitResult test_save_features(const MunitParameter params[], void *user_data) {
    (void)params;
    (void)user_data;

    const std::vector<sail::codec_info> codecs = sail::codec_info::list();
    munit_assert(codecs.size() > 0U);

    const sail::codec_info first_codec = codecs.front();

    // Copy
    {
        const sail::save_features save_features = first_codec.save_features();

        munit_assert(save_features.features() == first_codec.save_features().features());

        munit_assert(save_features.compression_level().is_valid() == first_codec.save_features().compression_level().is_valid());
        if (save_features.compression_level().is_valid()) {
            const sail::compression_level &l1 = save_features.compression_level();
            const sail::compression_level &l2 = first_codec.save_features().compression_level();

            munit_assert(l1.min_level()     == l2.min_level());
            munit_assert(l1.max_level()     == l2.max_level());
            munit_assert(l1.default_level() == l2.default_level());
            munit_assert(l1.step()          == l2.step());
        }

        munit_assert(save_features.supported_tuning() == first_codec.save_features().supported_tuning());
    }

    // Move
    {
        const sail::save_features save_features1 = std::move(first_codec.save_features());
        const sail::save_features save_features = std::move(save_features1);

        munit_assert(save_features.features() == first_codec.save_features().features());

        munit_assert(save_features.compression_level().is_valid() == first_codec.save_features().compression_level().is_valid());
        if (save_features.compression_level().is_valid()) {
            const sail::compression_level &l1 = save_features.compression_level();
            const sail::compression_level &l2 = first_codec.save_features().compression_level();

            munit_assert(l1.min_level()     == l2.min_level());
            munit_assert(l1.max_level()     == l2.max_level());
            munit_assert(l1.default_level() == l2.default_level());
            munit_assert(l1.step()          == l2.step());
        }

        munit_assert(save_features.supported_tuning() == first_codec.save_features().supported_tuning());
    }

    // Construct save options
    {
        sail::save_options save_options;
        munit_assert(first_codec.save_features().to_options(&save_options) == SAIL_OK);
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/save-features", test_save_features, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
    (char *)"/bindings/c++",
    test_suite_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, NULL, argc, argv);
}
