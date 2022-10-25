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

#include <modules/ECMA1997/Main.hpp>

#include <iostream>
#include <string>

/** returns 2 on non normal exit of engine (fatal error thrown)
 *  0 is nominal exit for both engine and program
 *  otherwise, non normal exit for executed program.
*/
int main(int argc, char** argv) {
    std::string ScriptPath = argv[argc - 1];
    int ret;
    try {
        ECMABase::Main* Engine = new ECMA1997::Main();
        Engine->Preload(argc, argv);
        Engine->Load(ScriptPath);
        Engine->Postload();
        ret = Engine->Run();
    } catch (std::exception& e) {
        std::cout << "FATAL ERROR IN ENGINE CODE: " << e.what() << std::endl;
        return 2;
    }
    return ret;
}