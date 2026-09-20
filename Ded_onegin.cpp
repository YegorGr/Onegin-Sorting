#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#define HINT    "\x1b[93m"            //!< Sand color (yellow but light yellow)
#define SUCCESS "\x1b[38;5;46m"       //!< Bright green color (specially for success)
#define BOLD    "\x1b[1m"             //!< Very fat (like me)
#define RESET   "\x1b[0m"             //!< To reset style
#define PART_C  "\x1b[38;5;203m"      //!< Bright red color

struct TextInfo {

    char* buffer;
    char** index;
    size_t lines;

};

TextInfo ReadFromFile (const char* filename);
TextInfo GetFileName  (const char* filename);
int      WriteToFile  (const TextInfo* text, const char* filename);

void     Swap         (void* val1, void* val2, size_t size_elem);
void     myqsort      (void* data, const size_t size_array, size_t size_elem, int (*CompareFunc)(const void* a, const void* b));
int      CompareStr   (const void* a, const void* b);
int      HasLetters   (const char* str);

int main(int argc, const char* argv[])
{
    const TextInfo data = FileReadName((argc > 1) ? argv[1] : "onegin.txt");

    if (data.buffer == NULL)
    {
        printf("Failed to read file.\n");
        return 1;
    }

    const char filename_put[] = "Sorted_Onegin.txt";


    if (WriteToFile(&data, filename_put) == 0)
        printf(SUCCESS "\nSorted text successfully written to file: %s" RESET, filename_put);

    free(data.index);
    free(data.buffer);

    getchar();
}

TextInfo ReadFromFile (const char* filename)
{
    TextInfo info_text = {NULL, NULL, 0};

    int descriptor = open(filename, _O_RDONLY);

    if (descriptor == -1)
    {
        printf(PART_C "\nError opening file!" RESET);
        return info_text;
    }

    assert(descriptor != -1);

    struct stat mystat = {};
    fstat(descriptor, &mystat);

    size_t size_array = mystat.st_size + 1;

    char* buffer = (char*) (calloc(size_array, sizeof(char)));
    size_t real_size = read(descriptor, buffer, size_array);

    for (size_t i = real_size; i < size_array; i++)
        buffer[i] = '\0';

    size_t lines = 1;
    char* ptr = buffer;

    while ((ptr = strchr(ptr, '\n')) != NULL)
    {
        *ptr = '\0';

        ptr++;
        lines++;
    }

    char** index = (char**)calloc(lines, sizeof(char*));

    index[0] = buffer;
    char* ptrcopy = buffer;

    for (size_t i = 1; i < lines; i++)
    {
        ptrcopy = strchr(ptrcopy, '\0');

        if (ptrcopy != NULL)
        {
            ptrcopy++;
            index[i] = ptrcopy;
        }

    }

    qsort(index, lines, sizeof(char*), &CompareStr);

    close(descriptor);

    info_text.buffer = buffer;
    info_text.index  = index;
    info_text.lines  = lines;

    return info_text;
}

int CompareStr(const void* a, const void* b)
{
    const char* s1 = *(const char* const *)(a);
    const char* s2 = *(const char* const *)(b);

    while (*s1 != '\0' || *s2 != '\0')
    {
        while (*s1 != '\0' && !isalpha((unsigned char)*s1))
            s1++;

        while (*s2 != '\0' && !isalpha((unsigned char)*s2))
            s2++;

        if (*s1 == '\0' && *s2 == '\0')
            return 0;

        if (*s1 == '\0' || *s2 == '\0')
            return (unsigned char)*s1 - (unsigned char)*s2;

        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);

        if (c1 != c2)
            return c1 - c2;

        s1++;
        s2++;
    }

    return 0;
}

TextInfo GetFileName(const char* filename)
{
    const int MAXNAME = 256;
    const int MAXTRIES = 3;
    int tries = 0;
    
    TextInfo result = {NULL, NULL, 0};

    printf(HINT "\nWrite the file name for sorting the lines.");

    while(tries < MAXTRIES)
    {
        printf(HINT "\nTries left: %d" HINT
              "\nFilename (" SUCCESS "name.txt" HINT "): " RESET, MAXTRIES - tries);

        if (fgets(filename, MAXNAME, stdin) != NULL)
        {
            filename[strcspn(filename, "\r\n")] = '\0';

            printf(HINT "\nOpen the file: %s" RESET, filename);

            result = ReadFromFile(filename);
            if (result.buffer != NULL)
                return result;
        }
        tries++;
    }

    return result;
}

void Swap (void* val1, void* val2, size_t size_elem)
{
    char* p1 = (char*) val1;
    char* p2 = (char*) val2;

    for (size_t i = 0; i < size_elem; i++)
    {
        char p_temp = *p1;

        *p1 = *p2;
        *p2 = p_temp;

        p1++;
        p2++;
    }
}

void myqsort(void* data, size_t size_array, const size_t size_elem, int (*CompareFunc)(const void* a, const void* b))
{
    if (size_array <= 1)
        return;

    char* partition = (char*) calloc(1, size_elem);

    for (size_t i = 0; i < size_elem; i++)
    {
        partition[i] = *((char*)(data) + (size_array / 2) * size_elem + i);
    }

    char* left  = (char*) data;
    char* right = (char*) data + (size_array - 1) * size_elem;


    while (left <= right)
    {
        while (CompareFunc(left, partition) < 0)
            left += size_elem;

        while(CompareFunc(right, partition) > 0)
            right -= size_elem;

        if (left <= right)
        {
            Swap(left, right, size_elem);

            left += size_elem;
            right -= size_elem;
        }
    }

    free(partition);

    size_t new_size_l = (size_t) ((left - (char*)data) / size_elem);
    size_t new_size_r = size_array - new_size_l;

    if (new_size_l > 1 && new_size_l < size_array)
        myqsort(data, new_size_l, size_elem, CompareFunc);

    if (new_size_r > 1 && new_size_r < size_array)
        myqsort(left, new_size_r, size_elem, CompareFunc);
}

int WriteToFile (const TextInfo* text, const char* filename) // to do buffer???
{
    assert(text != NULL);
    assert(filename != NULL);

    FILE* file  = fopen(filename, "w");

    if (file == NULL)
    {
        printf(PART_C "\nError opening file for writing!" RESET);
        return -1;
    }

    for (size_t i = 0; i < text->lines; i++)
    {
        if (HasLetters(text->index[i]))
        {
            fputs(text->index[i], file);
            fputc('\n', file);
        }
    }

    fclose(file);

    return 0;
}

int HasLetters (const char* str)
{
    while (*str != '\0')
    {
        if (isalpha((unsigned char)*str))
    3
            return 1;

        str++;
    }

    return 0;
}

