/*************************************************************************
> File Name       : Log.cpp
> Brief           : 日志模块
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月05日  17时36分03秒
************************************************************************/
#include "Log.h"
#include "spdlog/sinks/base_sink.h"
#include "spdlog/details/file_helper.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <filesystem>
#include <chrono>

namespace Log
{
    template <typename Mutex>
    class HtmlFormatSink final : public spdlog::sinks::base_sink<Mutex>
    {
    public:
        HtmlFormatSink(spdlog::filename_t                 baseFileName,
                       std::size_t                        maxSize,
                       std::size_t                        maxFiles,
                       bool                               rotateOnOpen  = false,
                       const spdlog::file_event_handlers &eventHandlers = {})
            : _mBaseFilename(std::move(baseFileName))
            , _maxSize(maxSize)
            , _maxFiles(maxFiles)
            , _fileHelper(eventHandlers)
        {
            if (maxSize == 0)
            {
                spdlog::throw_spdlog_ex("rotating sink constructor: max_size arg cannot be zero");
            }

            if (maxFiles > 200000)
            {
                spdlog::throw_spdlog_ex("rotating sink constructor: max_files arg cannot exceed 200000");
            }
            _fileHelper.open(calc_filename(_mBaseFilename, 0));

            // 写入html头
            spdlog::memory_buf_t htmlHeader;
            const char          *pHtmlHeader =
                R"(<html>
                <head>
                <meta http-equiv="content-type" content="text/html; charset-gb2312">
                <title>Html Output</title>
                </head>
                <body>
                <font face="Fixedsys" size="2" color="#0000FF">)";
            htmlHeader.append(pHtmlHeader, pHtmlHeader + std::strlen(pHtmlHeader));
            _fileHelper.write(htmlHeader);

            _currentSize = _fileHelper.size(); // expensive. called only once
            if (rotateOnOpen && _currentSize > 0)
            {
                rotate_();
                _currentSize = 0;
            }
        }

        static spdlog::filename_t calc_filename(const spdlog::filename_t &fileName, std::size_t index)
        {
            // if (index == 0u)
            //{
            //     return fileName;
            // }

            const auto &[basename, ext] = spdlog::details::file_helper::split_by_extension(fileName);
            std::string strTimeStr      = std::format(
                "{:%Y_%m_%d_%H_%M_%OS}",
                std::chrono::zoned_time {std::chrono::current_zone(), std::chrono::system_clock::now()});

            return spdlog::fmt_lib::format(SPDLOG_FILENAME_T("{}{}.{}{}"), basename, strTimeStr, index, ext);
        }

        spdlog::filename_t filename()
        {
            std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
            return _fileHelper.filename();
        }

    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override
        {
            spdlog::memory_buf_t formatted;
            const char          *pPrefix = GetLogLevelHtmlPrefix(msg.level);
            // 填充html前缀
            formatted.append(pPrefix, pPrefix + std::strlen(pPrefix));

            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

            // 填充后缀
            const char *pSuffix = R"(<br></font>)";
            formatted.append(pSuffix, pSuffix + std::strlen(pSuffix));

            auto newSize = _currentSize + formatted.size();

            // rotate if the new estimated file size exceeds max size.
            // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
            // we only check the real size when new_size > max_size_ because it is relatively expensive.
            if (newSize > _maxSize)
            {
                _fileHelper.flush();
                if (_fileHelper.size() > 0)
                {
                    rotate_();
                    newSize = formatted.size();
                }
            }
            _fileHelper.write(formatted);
            _currentSize = newSize;
        }

        void flush_() override
        {
            _fileHelper.flush();
        }

    private:
        void rotate_()
        {
            using namespace spdlog;
            // using details::os::filename_to_str;
            // using details::os::path_exists;

            _fileHelper.close();
            for (auto i = _maxFiles; i > 0; --i)
            {
                std::filesystem::path src {calc_filename(_mBaseFilename, i - 1)};
                std::error_code       err;
                bool                  bExist = std::filesystem::exists(src, err);
                if (!bExist)
                {
                    continue;
                }
                filename_t target = calc_filename(_mBaseFilename, i);

                if (!rename_file_(src.string(), target))
                {
                    // if failed try again after a small delay.
                    // this is a workaround to a windows issue, where very high rotation
                    // rates can cause the rename to fail with permission denied (because of antivirus?).
                    // todo 是否需要写入头？
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    if (!rename_file_(src.string(), target))
                    {
                        _fileHelper.reopen(
                            true); // truncate the log file anyway to prevent it to grow beyond its limit!
                        _currentSize = 0;
                        throw_spdlog_ex("rotating_file_sink: failed renaming " + src.string() + " to "
                                            + target,
                                        errno);
                    }
                }
            }
            _fileHelper.reopen(true);

            // 写入html头
            spdlog::memory_buf_t htmlHeader;
            const char          *pHtmlHeader =
                R"(<html>
                <head>
                <meta http-equiv="content-type" content="text/html; charset-gb2312">
                <title>Html Output</title>
                </head>
                <body>
                <font face="Fixedsys" size="2" color="#0000FF">)";

            htmlHeader.append(pHtmlHeader, pHtmlHeader + std::strlen(pHtmlHeader));
            _fileHelper.write(htmlHeader);
        }

        bool rename_file_(const spdlog::filename_t &srcFileName, const spdlog::filename_t &targetFileName)
        {
            // try to delete the target file in case it already exists.
            // (void)spdlog::details::os::remove(targetFileName);
            (void)std::remove(targetFileName.c_str());
            // return spdlog::details::os::rename(srcFileName, targetFileName) == 0;
            return std::rename(srcFileName.c_str(), targetFileName.c_str()) == 0;
        }

        constexpr const char *GetLogLevelHtmlPrefix(spdlog::level::level_enum level)
        {
            const char *pPrefix = "";
            switch (level)
            {
                case spdlog::level::trace:
                    pPrefix = R"(<font color=" #DCDFE4">)";
                    break;
                case spdlog::level::debug:
                    pPrefix = R"(<font color=" #56B6C2">)";
                    break;
                case spdlog::level::info:
                    pPrefix = R"(<font color=" #98C379">)";
                    break;
                case spdlog::level::warn:
                    pPrefix = R"(<font color=" #E5C07B">)";
                    break;
                case spdlog::level::err:
                    pPrefix = R"(<font color=" #E06C75">)";
                    break;
                case spdlog::level::critical:
                    pPrefix = R"(<font color=" #DCDFE4" style="background-color:#E06C75;">)";
                    break;
                case spdlog::level::off:
                case spdlog::level::n_levels:
                    break;
            }

            return pPrefix;
        }

    private:
        spdlog::filename_t           _mBaseFilename;
        std::size_t                  _maxSize;
        std::size_t                  _maxFiles;
        std::size_t                  _currentSize;
        spdlog::details::file_helper _fileHelper;
    };

    using HtmlFormatSinkMt = HtmlFormatSink<std::mutex>;
    using HtmlFormatSinkSt = HtmlFormatSink<spdlog::details::null_mutex>;

    void CLogger::InitLogger(std::string_view fileName,
                             size_t           level,
                             size_t           maxFileSize,
                             size_t           maxFiles,
                             std::string_view pattern)
    {
        auto fileSink =
            std::make_shared<HtmlFormatSinkMt>(std::string(fileName), maxFileSize * 1024 * 1024, maxFiles);
        fileSink->set_level(static_cast<spdlog::level::level_enum>(level));
        fileSink->set_pattern(std::string(pattern));

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(static_cast<spdlog::level::level_enum>(level));
        consoleSink->set_pattern(std::string(pattern));

        std::vector<spdlog::sink_ptr> sinks {fileSink, consoleSink};
        _logger = std::make_shared<spdlog::logger>("MultiLogger", std::begin(sinks), std::end(sinks));
        _logger->set_level(static_cast<spdlog::level::level_enum>(level));

        spdlog::set_default_logger(_logger);
    }
} // namespace Log