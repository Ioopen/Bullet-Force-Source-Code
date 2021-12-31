typedef unsigned long DWORD;
static DWORD libBase;

DWORD findLibrary(const char *library);
DWORD getAbsoluteAddress(const char* libraryName, DWORD relativeAddr);
DWORD getRealOffset(DWORD offset);
bool isLibraryLoaded(const char *libraryName);
void hook(void *orig_fcn, void* new_fcn, void **orig_fcn_ptr);

DWORD findLibrary(const char *library) {
    char filename[0xFF] = {0},
            buffer[1024] = {0};
    FILE *fp = NULL;
    DWORD address = 0;

    sprintf( filename, "/proc/self/maps");
    fp = fopen( filename, "rt" );
    if( fp == NULL ){
        perror("fopen");
        goto done;
    }

    while( fgets( buffer, sizeof(buffer), fp ) ) {
        if( strstr( buffer, library ) ){
            address = (DWORD)strtoul( buffer, NULL, 16 );
            goto done;
        }
    }

    done:

    if(fp){
        fclose(fp);
    }

    return address;
}
DWORD getAbsoluteAddress(const char* libraryName, DWORD relativeAddr) {
    if(libBase == 0)
        libBase = findLibrary(libraryName);
    if (libBase != 0)
        return (reinterpret_cast<DWORD>(libBase + relativeAddr));
    else
        return 0;
}
DWORD getRealOffset(DWORD offset) {
    if(libBase == 0)
        libBase = findLibrary("libil2cpp.so");
    if (libBase != 0)
        return (reinterpret_cast<DWORD>(libBase + offset));
    else
        return 0;
}

bool isLibraryLoaded(const char *libraryName) {
    char line[512] = {0};
    FILE *fp = fopen("/proc/self/maps", "rt");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, libraryName))
                return true;
        }
        fclose(fp);
    }
    return false;
}

uintptr_t string2Offset(const char *c) {
    int base = 16;
    // See if this function catches all possibilities.
    // If it doesn't, the function would have to be amended
    // whenever you add a combination of architecture and
    // compiler that is not yet addressed.
    static_assert(sizeof(uintptr_t) == sizeof(unsigned long)
                  || sizeof(uintptr_t) == sizeof(unsigned long long),
                  "Please add string to handle conversion for this architecture.");

    // Now choose the correct function ...
    if (sizeof(uintptr_t) == sizeof(unsigned long)) {
        return strtoul(c, nullptr, base);
    }

    // All other options exhausted, sizeof(uintptr_t) == sizeof(unsigned long long))
    return strtoull(c, nullptr, base);
}

void hook(void *orig_fcn, void* new_fcn, void **orig_fcn_ptr)
{
#if defined(__aarch64__)
    A64HookFunction(orig_fcn, new_fcn, orig_fcn_ptr);
#else
    MSHookFunction(orig_fcn, new_fcn, orig_fcn_ptr);
#endif
}
void LogAddress(void *object, const char *objectname)
{
    std::string whatever(objectname);
    whatever.append(": %p");
    LOGD(whatever.c_str(), object);
}
