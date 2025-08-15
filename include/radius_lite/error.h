#pragma once

#include <boost/system/error_code.hpp>
#include <string>

namespace radius_lite
{
  class ErrorCategory: public boost::system::error_category
  {
  public:
    const char* name() const noexcept;
    std::string message(int ev) const noexcept;
  };

  enum class Error
  {
    success = 0,

    numberOfBytesIsLessThan20,
    requestLengthIsShort,
    eapMessageAttributeError,
    invalidAttributeType,
    invalidAttributeSize,
    invalidVendorSpecificAttributeId,
    suchAttributeNameAlreadyExists,
    suchAttributeCodeAlreadyExists
  };

  class Exception: public std::runtime_error
  {
  public:
    explicit Exception(const boost::system::error_code& errorCode, const std::string& message);
    explicit Exception(const boost::system::error_code& errorCode);
    boost::system::error_code getErrorCode() const {return m_errorCode;}

  private:
    boost::system::error_code m_errorCode;
  };

  boost::system::error_code make_error_code(Error);
}

namespace boost
{
  namespace system
  {
    template <>
    struct is_error_code_enum<::radius_lite::Error> : std::true_type
    {
    };
  }
}
