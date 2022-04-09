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

static MunitResult test_save_options(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    const std::vector<sail::codec_info> codecs = sail::codec_info::list();
    munit_assert(codecs.size() > 0U);

    const sail::codec_info first_codec = codecs.front();

    {
        sail::save_options save_options;
        munit_assert(first_codec.save_features().to_options(&save_options) == SAIL_OK);
    }

    return MUNIT_OK;
}

static MunitResult test_save_options_copy(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    const sail::codec_info first_codec = sail::codec_info::list().front();

    {
        sail::save_options save_options;
        munit_assert(first_codec.save_features().to_options(&save_options) == SAIL_OK);
        munit_assert(save_options.tuning().empty());

        const sail::save_options save_options2 = save_options;
        munit_assert(save_options.options()           == save_options2.options());
        munit_assert(save_options.compression()       == save_options2.compression());
        munit_assert(save_options.compression_level() == save_options2.compression_level());
        munit_assert(save_options.tuning()            == save_options2.tuning());
    }

    {
        sail::save_options save_options;
        munit_assert(first_codec.save_features().to_options(&save_options) == SAIL_OK);
        save_options.tuning()["key"] = 10.0;
        munit_assert_double(save_options.tuning()["key"].value<double>(), ==, 10.0);

        const sail::save_options save_options2 = save_options;
        munit_assert(save_options.options()           == save_options2.options());
        munit_assert(save_options.compression()       == save_options2.compression());
        munit_assert(save_options.compression_level() == save_options2.compression_level());
        munit_assert(save_options.tuning()            == save_options2.tuning());
    }

    return MUNIT_OK;
}

static MunitResult test_save_options_move(const MunitParameter params[], void *user_data) {

    (void)params;
    (void)user_data;

    const sail::codec_info first_codec = sail::codec_info::list().front();

    {
        sail::save_options save_options;
        munit_assert(first_codec.save_features().to_options(&save_options) == SAIL_OK);

        sail::save_options save_options2 = save_options;
        const sail::save_options save_options3 = std::move(save_options2);

        munit_assert(save_options.options()           == save_options3.options());
        munit_assert(save_options.compression()       == save_options3.compression());
        munit_assert(save_options.compression_level() == save_options3.compression_level());
        munit_assert(save_options.tuning()            == save_options3.tuning());
    }

    {
        sail::save_options save_options;
        munit_assert(first_codec.save_features().to_options(&save_options) == SAIL_OK);
        save_options.tuning()["key"] = 10.0;
        munit_assert_double(save_options.tuning()["key"].value<double>(), ==, 10.0);

        sail::save_options save_options2 = save_options;
        const sail::save_options save_options3 = std::move(save_options2);

        munit_assert(save_options.options()           == save_options3.options());
        munit_assert(save_options.compression()       == save_options3.compression());
        munit_assert(save_options.compression_level() == save_options3.compression_level());
        munit_assert(save_options.tuning()            == save_options3.tuning());
    }

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char *)"/save-options", test_save_options,           NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/save-options/copy", test_save_options_copy, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
    { (char *)"/save-options/move", test_save_options_move, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },

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
