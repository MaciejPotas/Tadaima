/**
 * @file SystemTools.h
 * @brief Provides the declaration for the getexepath function.
 *
 * The getexepath function is used to retrieve the path of the executable file.
 * This can be useful for determining the location of resources relative to the executable.
 */

#pragma once

#include <string>

namespace tadaima
{
    /**
     * @brief Retrieves the path of the executable file.
     * @return A string containing the path of the executable file.
     */
    std::string getexepath();
}
