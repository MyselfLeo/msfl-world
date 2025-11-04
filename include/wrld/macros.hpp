//
// Created by leo on 19/10/2025.
//

#pragma once

#if defined(__clang__)
    #define MSFL_WRLD_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define MSFL_WRLD_COMPILER_GNUC
#elif defined(_MSC_VER)
    #define MSFL_WRLD_COMPILER_MSVC
#endif
