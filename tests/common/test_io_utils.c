#include "common/io_utils.h"
#include "../tests_helper.h"

#include <stdio.h>
#include <string.h>

static int test_io_read_bytes_success(void) {
    FILE *fp;
    char buffer[4];
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    if (fwrite("ABCD", 1, 4, fp) != 4) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    memset(buffer, 0, sizeof(buffer));
    ok = io_read_bytes(fp, buffer, 4);

    fclose(fp);

    return ok &&
           buffer[0] == 'A' &&
           buffer[1] == 'B' &&
           buffer[2] == 'C' &&
           buffer[3] == 'D';
}

static int test_io_read_bytes_null_file(void) {
    char buffer[4];
    return io_read_bytes(NULL, buffer, 4) == 0;
}

static int test_io_read_bytes_null_dst(void) {
    FILE *fp;
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = io_read_bytes(fp, NULL, 4);
    fclose(fp);

    return ok == 0;
}

static int test_io_read_bytes_eof(void) {
    FILE *fp;
    char buffer[4];
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    if (fwrite("AB", 1, 2, fp) != 2) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    memset(buffer, 0, sizeof(buffer));
    ok = io_read_bytes(fp, buffer, 4);

    fclose(fp);

    return ok == 0;
}

static int test_io_skip_bytes_success(void) {
    FILE *fp;
    int ok;
    long pos;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    if (fwrite("ABCDEFGH", 1, 8, fp) != 8) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    ok = io_skip_bytes(fp, 3);
    pos = ftell(fp);

    fclose(fp);

    return ok && pos == 3L;
}

static int test_io_skip_bytes_null_file(void) {
    return io_skip_bytes(NULL, 3) == 0;
}

static int test_io_skip_bytes_negative(void) {
    FILE *fp;
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = io_skip_bytes(fp, -1);
    fclose(fp);

    return ok == 0;
}

static int test_io_skip_bytes_then_read(void) {
    FILE *fp;
    char ch;
    int ok_skip;
    int ok_read;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    if (fwrite("ABCDEFGH", 1, 8, fp) != 8) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    ok_skip = io_skip_bytes(fp, 2);
    ok_read = io_read_bytes(fp, &ch, 1);

    fclose(fp);

    return ok_skip && ok_read && ch == 'C';
}

static int test_io_write_bytes_success(void) {
    FILE *fp;
    char buffer[4];
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = io_write_bytes(fp, "WXYZ", 4);
    if (!ok) {
        fclose(fp);
        return 0;
    }

    rewind(fp);
    memset(buffer, 0, sizeof(buffer));

    if (fread(buffer, 1, 4, fp) != 4) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    return buffer[0] == 'W' &&
           buffer[1] == 'X' &&
           buffer[2] == 'Y' &&
           buffer[3] == 'Z';
}

static int test_io_write_bytes_null_file(void) {
    return io_write_bytes(NULL, "ABCD", 4) == 0;
}

static int test_io_write_bytes_null_src(void) {
    FILE *fp;
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = io_write_bytes(fp, NULL, 4);
    fclose(fp);

    return ok == 0;
}

static int test_io_write_u16_le_success(void) {
    FILE *fp;
    unsigned char buffer[2];
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = io_write_u16_le(fp, 0x1234U);
    if (!ok) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    if (fread(buffer, 1, 2, fp) != 2) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    return buffer[0] == 0x34U && buffer[1] == 0x12U;
}

static int test_io_write_u16_le_null_file(void) {
    return io_write_u16_le(NULL, 0x1234U) == 0;
}

static int test_io_write_u32_le_success(void) {
    FILE *fp;
    unsigned char buffer[4];
    int ok;

    fp = tmpfile();
    if (fp == NULL) {
        return 0;
    }

    ok = io_write_u32_le(fp, 0x12345678UL);
    if (!ok) {
        fclose(fp);
        return 0;
    }

    rewind(fp);

    if (fread(buffer, 1, 4, fp) != 4) {
        fclose(fp);
        return 0;
    }

    fclose(fp);

    return buffer[0] == 0x78U &&
           buffer[1] == 0x56U &&
           buffer[2] == 0x34U &&
           buffer[3] == 0x12U;
}

static int test_io_write_u32_le_null_file(void) {
    return io_write_u32_le(NULL, 0x12345678UL) == 0;
}

int main(void) {
    test_report("io_read_bytes success", test_io_read_bytes_success());
    test_report("io_read_bytes null file", test_io_read_bytes_null_file());
    test_report("io_read_bytes null dst", test_io_read_bytes_null_dst());
    test_report("io_read_bytes EOF failure", test_io_read_bytes_eof());

    test_report("io_skip_bytes success", test_io_skip_bytes_success());
    test_report("io_skip_bytes null file", test_io_skip_bytes_null_file());
    test_report("io_skip_bytes negative count", test_io_skip_bytes_negative());
    test_report("io_skip_bytes then read", test_io_skip_bytes_then_read());

    test_report("io_write_bytes success", test_io_write_bytes_success());
    test_report("io_write_bytes null file", test_io_write_bytes_null_file());
    test_report("io_write_bytes null src", test_io_write_bytes_null_src());

    test_report("io_write_u16_le success", test_io_write_u16_le_success());
    test_report("io_write_u16_le null file", test_io_write_u16_le_null_file());

    test_report("io_write_u32_le success", test_io_write_u32_le_success());
    test_report("io_write_u32_le null file", test_io_write_u32_le_null_file());

    test_summary();

    return test_failed_count() == 0 ? 0 : 1;
}
