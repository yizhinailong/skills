export module status;

import std;

export class Status {
public:
    enum class Code {
        kNotFound = 1,
        kCorruption = 2,
        kNotSupported = 3,
        kInvalidArgument = 4,
        kIOError = 5
    };

    [[nodiscard]] static Status NotFound(std::string_view message)
    {
        return { Code::kNotFound, message };
    }

    [[nodiscard]] static Status NotFound(std::string_view message, std::string_view detail)
    {
        return { Code::kNotFound, message, detail };
    }

    [[nodiscard]] static Status Corruption(std::string_view message)
    {
        return { Code::kCorruption, message };
    }

    [[nodiscard]] static Status Corruption(std::string_view message, std::string_view detail)
    {
        return { Code::kCorruption, message, detail };
    }

    [[nodiscard]] static Status NotSupported(std::string_view message)
    {
        return { Code::kNotSupported, message };
    }

    [[nodiscard]] static Status NotSupported(std::string_view message, std::string_view detail)
    {
        return { Code::kNotSupported, message, detail };
    }

    [[nodiscard]] static Status InvalidArgument(std::string_view message)
    {
        return { Code::kInvalidArgument, message };
    }

    [[nodiscard]] static Status InvalidArgument(std::string_view message, std::string_view detail)
    {
        return { Code::kInvalidArgument, message, detail };
    }

    [[nodiscard]] static Status IOError(std::string_view message)
    {
        return { Code::kIOError, message };
    }

    [[nodiscard]] static Status IOError(std::string_view message, std::string_view detail)
    {
        return { Code::kIOError, message, detail };
    }

    [[nodiscard]] Code code() const noexcept { return m_code; }

    [[nodiscard]] std::string_view message() const & noexcept { return m_message; }
    std::string_view message() const && = delete;

    [[nodiscard]] bool IsNotFound() const noexcept { return code() == Code::kNotFound; }

    [[nodiscard]] bool IsCorruption() const noexcept { return code() == Code::kCorruption; }

    [[nodiscard]] bool IsNotSupported() const noexcept { return code() == Code::kNotSupported; }

    [[nodiscard]] bool IsInvalidArgument() const noexcept { return code() == Code::kInvalidArgument; }

    [[nodiscard]] bool IsIOError() const noexcept { return code() == Code::kIOError; }

    [[nodiscard]] std::string ToString() const
    {
        return std::format("{}: {}", codeName(m_code), m_message);
    }

private:
    Code m_code;
    std::string m_message;

    Status(Code code, std::string_view message)
        : m_code{ code },
          m_message{ message } {}

    Status(Code code, std::string_view message, std::string_view detail)
        : m_code{ code },
          m_message{ std::format("{}: {}", message, detail) } {}

    [[nodiscard]] static std::string_view codeName(Code code) noexcept
    {
        using enum Code;
        switch (code) {
            case kNotFound       : return "NotFound";
            case kCorruption     : return "Corruption";
            case kNotSupported   : return "Not supported";
            case kInvalidArgument: return "Invalid argument";
            case kIOError        : return "IO error";
        }
        return "Unknown error";
    }
};

export template <typename T>
using Result = std::expected<T, Status>;
