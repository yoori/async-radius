#include "error.h"
#include <string>

using Exception = radius_lite::Exception;
Exception::Exception(const boost::system::error_code& errorCode, const std::string& message)
    : std::runtime_error(message),
      m_errorCode(errorCode)
{
}

Exception::Exception(const boost::system::error_code& errorCode)
    : std::runtime_error(errorCode.message()),
      m_errorCode(errorCode)
{
}

using ErrorCategory = radius_lite::ErrorCategory;
const char* ErrorCategory::name() const noexcept
{
    return "radproto";
}

using Error = radius_lite::Error;
std::string ErrorCategory::message(int ev) const noexcept
{
    switch (static_cast<Error>(ev))
    {
        case Error::success:
            return "No error";
        case Error::numberOfBytesIsLessThan20:
            return "Request length is less than 20 bytes";
        case Error::requestLengthIsShort:
            return "Request length is less than specified in the request";
        case Error::eapMessageAttributeError:
            return "The EAP-Message attribute is present, but the Message-Authenticator attribute is missing";
        case Error::invalidAttributeType:
            return "Invalid attribute type";
        case Error::invalidAttributeSize:
            return "Invalid attribute size";
        case Error::invalidVendorSpecificAttributeId:
            return "Invalid Vendor Specific attribute Id";
        case Error::suchAttributeNameAlreadyExists:
            return "Such attribute name already exists";
        case Error::suchAttributeCodeAlreadyExists:
            return "Such attribute code already exists";
       default:
            return "(Unrecognized error)";
    }
}

boost::system::error_code radius_lite::make_error_code(Error e)
{
    static const ErrorCategory instance;
    return boost::system::error_code(static_cast<int>(e), instance);
}
