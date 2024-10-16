﻿/*************************************************************************
> File Name       : Asio.h
> Brief           : asio类型定义
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年05月24日  15时36分04秒
************************************************************************/
#pragma once

#include "Common/Util/Platform.h"
#include "asio.hpp"

namespace Asio
{
    using default_token = asio::as_tuple_t<asio::use_awaitable_t<>>;
    using acceptor      = default_token::as_default_on_t<asio::ip::tcp::acceptor>;
    using socket        = default_token::as_default_on_t<asio::ip::tcp::socket>;
    using steady_timer  = default_token::as_default_on_t<asio::steady_timer>;

    using asio::io_context;
    using asio::signal_set;
    using asio::ip::address;
    using endpoint = asio::ip::tcp::endpoint;

    using asio::co_spawn;
    using asio::ip::make_address;
} // namespace Asio