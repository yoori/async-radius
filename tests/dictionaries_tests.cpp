#define BOOST_TEST_MODULE radius_litedictionaries_tests

#include <string>
#include <map>
#include <cstdint>
#include <stdexcept>

#include <radius_lite/dictionaries.h>
#include <radius_lite/error.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop

BOOST_AUTO_TEST_SUITE(dictionaries_tests)

BOOST_AUTO_TEST_SUITE(basic_dictionary_tests)

BOOST_AUTO_TEST_CASE(TestAdd)
{
  radius_lite::BasicDictionary b;

  b.add(1, "User-Name");
  b.add(1, "User");
  b.add(1, "abc");
  BOOST_CHECK_THROW(b.add(2, "abc"), radius_lite::Exception);
  b.add(3, "def");
  b.add(1, "abc");

  BOOST_CHECK_EQUAL(b.name(1), "abc");
  BOOST_CHECK_EQUAL(b.name(3), "def");
  BOOST_CHECK_EQUAL(b.code("User-Name"), 1);
  BOOST_CHECK_EQUAL(b.code("User"), 1);
  BOOST_CHECK_EQUAL(b.code("abc"), 1);
  BOOST_CHECK_EQUAL(b.code("def"), 3);

  BOOST_CHECK_THROW(b.name(2), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(TestAppend)
{
  radius_lite::BasicDictionary a;

  a.add(2, "def");
  a.add(3, "ghi");
  a.add(4, "User-Name");

  BOOST_CHECK_EQUAL(a.name(2), "def");
  BOOST_CHECK_EQUAL(a.name(3), "ghi");
  BOOST_CHECK_EQUAL(a.name(4), "User-Name");
  BOOST_CHECK_EQUAL(a.code("def"), 2);
  BOOST_CHECK_EQUAL(a.code("ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("User-Name"), 4);

  radius_lite::BasicDictionary b;

  b.add(1, "User-Name");

  BOOST_CHECK_EQUAL(b.name(1), "User-Name");
  BOOST_CHECK_EQUAL(b.code("User-Name"), 1);

  BOOST_CHECK_THROW(a.append(b), radius_lite::Exception);

  BOOST_CHECK_THROW(a.name(1), std::out_of_range);

  radius_lite::BasicDictionary c;

  c.add(4, "User");

  BOOST_CHECK_EQUAL(c.name(4), "User");
  BOOST_CHECK_EQUAL(c.code("User"), 4);

  a.append(c);

  BOOST_CHECK_EQUAL(a.name(2), "def");
  BOOST_CHECK_EQUAL(a.name(3), "ghi");
  BOOST_CHECK_EQUAL(a.name(4), "User");
  BOOST_CHECK_EQUAL(a.code("def"), 2);
  BOOST_CHECK_EQUAL(a.code("ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("User"), 4);
  BOOST_CHECK_EQUAL(a.code("User-Name"), 4);

  radius_lite::BasicDictionary d;

  d.add(4, "jkl");

  BOOST_CHECK_EQUAL(d.name(4), "jkl");
  BOOST_CHECK_EQUAL(d.code("jkl"), 4);

  a.append(d);

  BOOST_CHECK_EQUAL(a.name(2), "def");
  BOOST_CHECK_EQUAL(a.name(3), "ghi");
  BOOST_CHECK_EQUAL(a.name(4), "jkl");
  BOOST_CHECK_EQUAL(a.code("def"), 2);
  BOOST_CHECK_EQUAL(a.code("ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("User"), 4);
  BOOST_CHECK_EQUAL(a.code("User-Name"), 4);
  BOOST_CHECK_EQUAL(a.code("jkl"), 4);

  radius_lite::BasicDictionary e;

  e.add(2, "def");

  BOOST_CHECK_EQUAL(e.name(2), "def");
  BOOST_CHECK_EQUAL(e.code("def"), 2);

  a.append(e);

  BOOST_CHECK_EQUAL(a.name(2), "def");
  BOOST_CHECK_EQUAL(a.name(3), "ghi");
  BOOST_CHECK_EQUAL(a.name(4), "jkl");
  BOOST_CHECK_EQUAL(a.code("def"), 2);
  BOOST_CHECK_EQUAL(a.code("ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("User"), 4);
  BOOST_CHECK_EQUAL(a.code("User-Name"), 4);
  BOOST_CHECK_EQUAL(a.code("jkl"), 4);

  radius_lite::BasicDictionary f;

  f.add(4, "User-Name");

  BOOST_CHECK_EQUAL(f.name(4), "User-Name");
  BOOST_CHECK_EQUAL(f.code("User-Name"), 4);

  a.append(f);

  BOOST_CHECK_EQUAL(a.name(2), "def");
  BOOST_CHECK_EQUAL(a.name(3), "ghi");
  BOOST_CHECK_EQUAL(a.name(4), "User-Name");
  BOOST_CHECK_EQUAL(a.code("def"), 2);
  BOOST_CHECK_EQUAL(a.code("ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("User-Name"), 4);
  BOOST_CHECK_EQUAL(a.code("User"), 4);
  BOOST_CHECK_EQUAL(a.code("jkl"), 4);
}

BOOST_AUTO_TEST_CASE(TestConstructor)
{
  radius_lite::BasicDictionary b;

  BOOST_CHECK_THROW(b.name(0), std::out_of_range);

  BOOST_CHECK_THROW(b.code(""), std::out_of_range);

  b.add(1, "User-Name");
  b.add(1, "User");
  b.add(5, "ijk");
  b.add(3, "def");
  BOOST_CHECK_THROW(b.add(2, "User"), radius_lite::Exception);

  BOOST_CHECK_EQUAL(b.name(1), "User");
  BOOST_CHECK_EQUAL(b.name(5), "ijk");
  BOOST_CHECK_EQUAL(b.name(3), "def");
  BOOST_CHECK_EQUAL(b.code("User-Name"), 1);
  BOOST_CHECK_EQUAL(b.code("User"), 1);
  BOOST_CHECK_EQUAL(b.code("ijk"), 5);
  BOOST_CHECK_EQUAL(b.code("def"), 3);
  BOOST_CHECK_THROW(b.name(2), std::out_of_range);

  radius_lite::BasicDictionary c;

  c.add(2, "def");
  c.add(5, "ghi");

  BOOST_CHECK_EQUAL(c.name(2), "def");
  BOOST_CHECK_EQUAL(c.name(5), "ghi");
  BOOST_CHECK_EQUAL(c.code("def"), 2);
  BOOST_CHECK_EQUAL(c.code("ghi"), 5);

  BOOST_CHECK_THROW(c.append(b), radius_lite::Exception);

  radius_lite::BasicDictionary a;

  a.add(2, "def");

  BOOST_CHECK_EQUAL(a.name(2), "def");
  BOOST_CHECK_EQUAL(a.code("def"), 2);

  c.append(a);

  BOOST_CHECK_EQUAL(c.name(2), "def");
  BOOST_CHECK_EQUAL(c.name(5), "ghi");
  BOOST_CHECK_EQUAL(c.code("def"), 2);
  BOOST_CHECK_EQUAL(c.code("ghi"), 5);
}
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(dependent_dictionary_tests)

BOOST_AUTO_TEST_CASE(test_add)
{
  radius_lite::DependentDictionary b;

  b.add(1, "Login-User", "Service-Type");
  b.add(2, "Framed-User", "Service-Type");
  b.add(3, "def", "abc");
  b.add(3, "ghi", "abc");
  BOOST_CHECK_THROW(b.add(4, "Framed-User", "Service-Type"), radius_lite::Exception);
  b.add(3, "ghi", "abc");

  BOOST_CHECK_EQUAL(b.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(b.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(b.name("abc", 3), "ghi");
  BOOST_CHECK_EQUAL(b.code("Service-Type", "Login-User"), 1);
  BOOST_CHECK_EQUAL(b.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(b.code("abc", "def"), 3);
  BOOST_CHECK_EQUAL(b.code("abc", "ghi"), 3);

  BOOST_CHECK_THROW(b.name("Service-Type", 4), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(TestAppend)
{
  radius_lite::DependentDictionary a;

  a.add(1, "Login-User", "Service-Type");
  a.add(2, "Framed-User", "Service-Type");
  a.add(3, "def", "abc");

  BOOST_CHECK_EQUAL(a.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(a.name("abc", 3), "def");
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Login-User"), 1);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(a.code("abc", "def"), 3);

  radius_lite::DependentDictionary b;

  b.add(4, "def", "abc");

  BOOST_CHECK_EQUAL(b.name("abc", 4), "def");
  BOOST_CHECK_EQUAL(b.code("abc", "def"), 4);

  BOOST_CHECK_THROW(a.append(b), radius_lite::Exception);

  BOOST_CHECK_THROW(a.name("abc", 4), std::out_of_range);

  radius_lite::DependentDictionary c;

  c.add(3, "ghi", "abc");

  BOOST_CHECK_EQUAL(c.name("abc", 3), "ghi");
  BOOST_CHECK_EQUAL(c.code("abc", "ghi"), 3);

  a.append(c);

  BOOST_CHECK_EQUAL(a.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(a.name("abc", 3), "ghi");
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Login-User"), 1);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(a.code("abc", "def"), 3);
  BOOST_CHECK_EQUAL(a.code("abc", "ghi"), 3);

  radius_lite::DependentDictionary d;

  d.add(1, "Login-User", "Service-Type");

  BOOST_CHECK_EQUAL(d.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(d.code("Service-Type", "Login-User"), 1);

  a.append(d);

  BOOST_CHECK_EQUAL(a.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(a.name("abc", 3), "ghi");
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Login-User"), 1);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(a.code("abc", "def"), 3);
  BOOST_CHECK_EQUAL(a.code("abc", "ghi"), 3);

  radius_lite::DependentDictionary e;

  e.add(10, "Call-Check", "Service-Type");

  BOOST_CHECK_EQUAL(e.name("Service-Type", 10), "Call-Check");
  BOOST_CHECK_EQUAL(e.code("Service-Type", "Call-Check"), 10);

  a.append(e);

  BOOST_CHECK_EQUAL(a.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(a.name("abc", 3), "ghi");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 10), "Call-Check");
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Login-User"), 1);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(a.code("abc", "def"), 3);
  BOOST_CHECK_EQUAL(a.code("abc", "ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Call-Check"), 10);


  radius_lite::DependentDictionary f;

  f.add(3, "def", "abc");

  BOOST_CHECK_EQUAL(f.name("abc", 3), "def");
  BOOST_CHECK_EQUAL(f.code("abc", "def"), 3);

  a.append(f);

  BOOST_CHECK_EQUAL(a.name("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(a.name("abc", 3), "def");
  BOOST_CHECK_EQUAL(a.name("Service-Type", 10), "Call-Check");
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Login-User"), 1);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(a.code("abc", "def"), 3);
  BOOST_CHECK_EQUAL(a.code("abc", "ghi"), 3);
  BOOST_CHECK_EQUAL(a.code("Service-Type", "Call-Check"), 10);
}

BOOST_AUTO_TEST_CASE(TestConstructor)
{
  radius_lite::DependentDictionary b;

  BOOST_CHECK_THROW(b.name("", 0), std::out_of_range);

  BOOST_CHECK_THROW(b.code("", ""), std::out_of_range);

  b.add(2, "Framed-User", "Service-Type");
  b.add(3, "def", "abc");
  b.add(3, "ghi", "abc");
  BOOST_CHECK_THROW(b.add(4, "Framed-User", "Service-Type"), radius_lite::Exception);
  b.add(3, "ghi", "abc");

  BOOST_CHECK_EQUAL(b.name("Service-Type", 2), "Framed-User");
  BOOST_CHECK_EQUAL(b.name("abc", 3), "ghi");
  BOOST_CHECK_EQUAL(b.code("Service-Type", "Framed-User"), 2);
  BOOST_CHECK_EQUAL(b.code("abc", "def"), 3);
  BOOST_CHECK_EQUAL(b.code("abc", "ghi"), 3);

  BOOST_CHECK_THROW(b.name("Service-Type", 4), std::out_of_range);

  radius_lite::DependentDictionary c;

  c.add(3, "Framed-User", "Service-Type");
  c.add(5, "jkl", "abc");

  BOOST_CHECK_EQUAL(c.name("Service-Type", 3), "Framed-User");
  BOOST_CHECK_EQUAL(c.name("abc", 5), "jkl");
  BOOST_CHECK_EQUAL(c.code("Service-Type", "Framed-User"), 3);
  BOOST_CHECK_EQUAL(c.code("abc", "jkl"), 5);

  BOOST_CHECK_THROW(c.append(b), radius_lite::Exception);

  radius_lite::DependentDictionary a;

  a.add(5, "jkl", "abc");

  BOOST_CHECK_EQUAL(a.name("abc", 5), "jkl");
  BOOST_CHECK_EQUAL(a.code("abc", "jkl"), 5);

  c.append(a);

  BOOST_CHECK_EQUAL(c.name("Service-Type", 3), "Framed-User");
  BOOST_CHECK_EQUAL(c.name("abc", 5), "jkl");
  BOOST_CHECK_EQUAL(c.code("Service-Type", "Framed-User"), 3);
  BOOST_CHECK_EQUAL(c.code("abc", "jkl"), 5);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(dictionaries_tests)

BOOST_AUTO_TEST_CASE(TestAttributeName)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.attributeName(1), "User-Name");
}

BOOST_AUTO_TEST_CASE(TestAttributeCode)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.attributeCode("User-Password"), 2);
}

BOOST_AUTO_TEST_CASE(TestAttributeValueName)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.attributeValueName("Service-Type", 1), "Login-User");
}

BOOST_AUTO_TEST_CASE(TestAttributeValueCode)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.attributeValueCode("Service-Type", "Framed-User"), 2);
}

BOOST_AUTO_TEST_CASE(TestVendorName)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.vendorName(171), "Dlink");
}

BOOST_AUTO_TEST_CASE(TestVendorCode)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.vendorCode("Dlink"), 171);
}

BOOST_AUTO_TEST_CASE(TestVendorAttributeName)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.vendorAttributeName("Dlink", 1), "Dlink-User-Level");
}

BOOST_AUTO_TEST_CASE(TestVendorAttributeCode)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.vendorAttributeCode("Dlink", "Dlink-VLAN-Name"), 10);
}

BOOST_AUTO_TEST_CASE(TestVendorAttributeValueName)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.vendorAttributeValueName("Dlink-User-Level", 1), "User-Legacy");
}

BOOST_AUTO_TEST_CASE(TestVendorAttributeValueCode)
{
  radius_lite::Dictionaries a("dictionary");

  BOOST_CHECK_EQUAL(a.vendorAttributeValueCode("Dlink-User-Level", "User"), 3);
}

BOOST_AUTO_TEST_CASE(TestAppend)
{
  radius_lite::Dictionaries a("dictionary.1");
  radius_lite::Dictionaries b("dictionary.dlink");

  a.append(b);

  BOOST_CHECK_EQUAL(a.attributeName(1), "User-Name");
  BOOST_CHECK_EQUAL(a.attributeCode("User-Password"), 2);

  BOOST_CHECK_EQUAL(a.attributeValueName("Service-Type", 1), "Login-User");
  BOOST_CHECK_EQUAL(a.attributeValueCode("Service-Type", "Framed-User"), 2);

  BOOST_CHECK_EQUAL(a.vendorName(171), "Dlink");
  BOOST_CHECK_EQUAL(a.vendorCode("Dlink"), 171);

  BOOST_CHECK_EQUAL(a.vendorAttributeName("Dlink", 1), "Dlink-User-Level");
  BOOST_CHECK_EQUAL(a.vendorAttributeCode("Dlink", "Dlink-VLAN-Name"), 10);

  BOOST_CHECK_EQUAL(a.vendorAttributeValueName("Dlink-User-Level", 1), "User-Legacy");
  BOOST_CHECK_EQUAL(a.vendorAttributeValueCode("Dlink-User-Level", "User"), 3);
}

BOOST_AUTO_TEST_CASE(TestConstructor)
{
  radius_lite::Dictionaries b("dictionary");

  BOOST_CHECK_THROW(b.attributeName(0), std::out_of_range);
  BOOST_CHECK_THROW(b.attributeCode(""), std::out_of_range);

  BOOST_CHECK_THROW(b.attributeValueName("", 0), std::out_of_range);
  BOOST_CHECK_THROW(b.attributeValueCode("", ""), std::out_of_range);

  BOOST_CHECK_THROW(b.vendorName(0), std::out_of_range);
  BOOST_CHECK_THROW(b.vendorCode(""), std::out_of_range);

  BOOST_CHECK_THROW(b.vendorAttributeName("", 0), std::out_of_range);
  BOOST_CHECK_THROW(b.vendorAttributeCode("", ""), std::out_of_range);

  BOOST_CHECK_THROW(b.vendorAttributeValueName("", 0), std::out_of_range);
  BOOST_CHECK_THROW(b.vendorAttributeValueCode("", ""), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
