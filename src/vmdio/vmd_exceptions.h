#pragma once

#include <stdexcept>

/**
 * @namespace vmdio::exceptions
 * @brief Namespace for custom exception classes used in vmdio library.
 * @details All exceptions thrown by vmdio library inherit from the base class VMDIOError,
 *          which itself inherits from std::runtime_error.
 */
namespace vmdio::exceptions
{
    /**
     * @class VMDIOError
     * @brief Base class for all exceptions thrown by vmdio library.
     */
    class VMDIOError : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    /**
     * @class FileSystemError
     * @brief Exception related to file I/O operations.
     */
    class FileSystemError : public VMDIOError
    {
    public:
        using VMDIOError::VMDIOError;
    };

    /**
     * @class FrameConflictError
     * @brief Exception thrown when there are conflicting frames in the VMD data.
     */
    class FrameConflictError : public VMDIOError
    {
    public:
        using VMDIOError::VMDIOError;
    };

    /**
     * @class FrameOverflowError
     * @brief Exception thrown when the number of frames exceeds the maximum allowed by the MMD.
     */
    class FrameOverflowError : public VMDIOError
    {
    public:
        using VMDIOError::VMDIOError;
    };

    /**
     * @class IncompatibleFormatError
     * @brief Exception thrown when the file format is incompatible with the expected format.
     * @details This is thrown when the passed file is not VMD file, or loaded VMD file header
     *          and vmdio function being called are incompatible, such as trying to read
     *          a model edit VMD file using camera edit functions.
     */
    class IncompatibleFormatError : public VMDIOError
    {
    public:
        using VMDIOError::VMDIOError;
    };

    /**
     * @class InvalidFieldValueError
     * @brief Exception thrown when a field in the frame data structures contains an invalid value.
     * @details This exception is generally thrown when the value of a field in the frame data
     *          structures is out of the valid range or is not one of the expected values.
     */
    class InvalidFieldValueError : public VMDIOError
    {
    public:
        using VMDIOError::VMDIOError;
    };

    /**
     * @class StringProcessError
     * @brief Exception thrown when there is an error in encoding conversion.
     */
    class StringProcessError : public VMDIOError
    {
    public:
        using VMDIOError::VMDIOError;
    };
}