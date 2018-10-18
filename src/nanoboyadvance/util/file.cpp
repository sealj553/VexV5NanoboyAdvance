///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <stdexcept>
#include "file.hpp"

using namespace std;

namespace Util {

    namespace File {

        bool exists(string filename) {

            ifstream ifs(filename);
            bool exists = ifs.is_open();

            ifs.close();

            return exists;
        }

        int get_size(string filename) {
            //ifstream ifs(filename, ios::in | ios::binary | ios::ate);
            //if (ifs.is_open()) {
            //    ifs.seekg(0, ios::end);
            //    return ifs.tellg();
            //} else {
            //    perror(filename.c_str());
            //    throw std::runtime_error("unable to access file: " + filename);
            //}

            FILE *fp = fopen(filename.c_str(), "r");
            if(fp == NULL){
                throw std::runtime_error("unable to access file: " + filename);
            }
            fseek(fp, 0L, SEEK_END);
            return ftell(fp);
        }

        u8* read_data(string filename) {
            int size = get_size(filename);
            FILE *fp = fopen(filename.c_str(), "r");
            char *data = new char[size]; 
            if(fread(data, sizeof(char), size, fp) == 0){
                printf("unable to read file %s\n", filename.c_str());
                throw std::runtime_error("unable to read file data: " + filename);
            }
            fclose(fp);
            return (u8*)data;

            //ifstream ifs(filename, ios::in | ios::binary | ios::ate);
            //size_t filesize;
            //u8* data = 0;

            //if(ifs.is_open()){
            //    ifs.seekg(0, ios::end);
            //    filesize = ifs.tellg();
            //    ifs.seekg(0, ios::beg);
            //    data = new u8[filesize];
            //    ifs.read((char*)data, filesize);
            //    ifs.close();
            //} else {
            //    perror(filename.c_str());
            //    throw std::runtime_error("unable to read file: " + filename);
            //}

            //return data;
        }

        std::string read_as_string(std::string filename) {
            std::string str;
            std::ifstream ifs(filename);

            // Reserve enough space in the sting to avoid dynamic reallocation.
            ifs.seekg(0, std::ios::end);
            str.reserve(ifs.tellg());
            ifs.seekg(0, std::ios::beg);

            str.assign((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());

            return str;
        }

        void write_data(string filename, u8* data, int size) {
            ofstream ofs(filename, ios::out | ios::binary);

            if (ofs.is_open()) {
                ofs.write((char*)data, size);
                ofs.close();
            } else {
                throw std::runtime_error("unable to write file: " + filename);
            }
        }
    }
}
