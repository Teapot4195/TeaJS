/*
 Copyright (c) 2022 Alex Huang <huangalex409@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BB71C4B7_4DB2_4C89_BE07_5997EFB46506
#define BB71C4B7_4DB2_4C89_BE07_5997EFB46506

#include <string>

namespace ECMABase {
class Main {
public:
    virtual ~Main() {}
    /// @brief Called right after initialization of this class
    /// @param argc argument count passed in directly from main
    /// @param argv arguments, passed directly from main
    virtual void Preload(int argc, char** argv) {}
    /// @brief Load the file into memory (if it is small enough for that)
    /// @param path path to the file to be executed
    virtual void Load(std::string& path) {}
    /// @brief Called after Load, here as a tokenizing step. 
    virtual void Postload(void) {}
    /// @brief  Execute loaded program.
    /// @return return value for the program.
    virtual int Run(void) {
        return 0;
    }
};
}

#endif /* BB71C4B7_4DB2_4C89_BE07_5997EFB46506 */
