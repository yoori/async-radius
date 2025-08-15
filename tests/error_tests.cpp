#define BOOST_TEST_MODULE radius_liteErrorTests

#include <radius_lite/error.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

BOOST_AUTO_TEST_SUITE(ErrorTests)

BOOST_AUTO_TEST_CASE(ErrorCodeConstruction)
{
  const auto ec = radius_lite::make_error_code(radius_lite::Error::success);

  BOOST_CHECK_EQUAL(ec.category().name(), "radproto");
}

BOOST_AUTO_TEST_SUITE_END()
