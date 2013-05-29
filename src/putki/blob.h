#pragma once

namespace putki
{
    char *pack_int16_field(char *where, short val);
    char *pack_int32_field(char *where, int val);
    char *pack_string_field(char *where, const char *src, char *aux_beg, char *aux_end);
}