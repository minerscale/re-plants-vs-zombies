#define _CRT_SECURE_NO_WARNINGS
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

// @Patoke: very barebones pak decryptor and extractor for re-plants-vs-zombies
// to use create a new project and add this file (compile with the latest c++ version if possible)

// comment out for no debug data
// #define PRINT_DEBUG_DATA

#define READ_FILE(name, type)                                                                                          \
    type name = *(type *)(ptr + file_offset);                                                                          \
    file_offset += sizeof type;

enum { FILEFLAGS_END = 0x80 };

int main() {
    if (!std::filesystem::exists("main.pak")) {
        printf("[ERROR] couldn't find main.pak\n");
        return 1;
    }

    // parse main.pak in memory
    FILE *fp = fopen("main.pak", "rb");

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *ptr = (char *)malloc(size);
    fread(ptr, 1, size, fp);
    fclose(fp);

    // unxor it
    for (int i = 0; i < size; i++)
        ptr[i] = (ptr[i]) ^ 0xF7;

    // write unxored main.pak to file
    FILE *wp = fopen("main.unpak", "wb");
    fwrite(ptr, 1, size, wp);
    fclose(wp);

    // read data
    uintptr_t file_offset = 0;

    // file header
    READ_FILE(magic, uint32_t);
    READ_FILE(version, uint32_t);

#ifdef PRINT_DEBUG_DATA
    printf("Magic: 0x%X\n", magic);
    printf("Version: %d\n", version);
#endif

    if (magic != 0xBAC04AC0) printf("[WARNING] bad file header magic value (0x%X)\n", magic);

    if (version > 0) printf("[WARNING] bad file version value (0x%X)\n", version);

    // packed files
    uintptr_t file_pos = 0;

    struct s_file_data {
        uintptr_t file_offset;
        int32_t file_size;
        std::string file_name;
    };

    std::vector<s_file_data> to_fix;
    to_fix.reserve(10000);

    while (true) {
        if (file_offset >= size) break;

        READ_FILE(flags, uint8_t);
        if (flags & FILEFLAGS_END) break;

        READ_FILE(name_width, uint8_t);

        char name[256];
        uint8_t idx = 0;
        for (; idx < name_width; idx++) {
            name[idx] = *(char *)(ptr + file_offset);

            if (name[idx] == '/' || name[idx] == '\\') {
                char old_val = name[idx];
                name[idx] = 0;

                std::filesystem::create_directory(name);

                name[idx] = old_val;
            }

            file_offset++;
        }

        name[idx] = 0;

        READ_FILE(file_size, int32_t);
        READ_FILE(file_time, uint64_t);

        to_fix.push_back({file_pos, file_size, name});

        file_pos += file_size;

#ifdef PRINT_DEBUG_DATA
        printf("File flags: 0x%X\n", flags);
        printf("Name width: %d\n", name_width);
        printf("File name: %s\n", name);
        printf("File size: %d\n", file_size);
        // printf("File time: 0x%X\n", file_time);
        printf("File offset: 0x%X\n", file_pos);
#endif
    }

    for (auto &file : to_fix) {
        // file_offset by now is the offset to the end of the file descriptor list
        file.file_offset += file_offset;

        if (std::filesystem::exists(file.file_name)) continue;

        std::ofstream file_data(file.file_name, std::ios::binary);
        file_data.write(ptr + file.file_offset, file.file_size);
        file_data.close();
    }

    free(ptr);

    printf("[SUCCESS] pak extracted successfully with a total of %d files\n", (int32_t)to_fix.size());

    return 0;
}