# Pre-run
if(INVADER_BUILD_DEPENDENCY_SCRIPT_PRE_RUN)
    # For Qt6
    set(CMAKE_FIND_ROOT_PATH "${INVADER_MINGW_PREFIX}/static;${CMAKE_FIND_ROOT_PATH}")

    # Invader should obviously not build with shared libs
    set(BUILD_SHARED_LIBS NO)

    # Set these
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -lwinpthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLAC__NO_DLL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFLAC__NO_DLL")

    # From https://github.com/Martchus/PKGBUILDs/blob/master/cmake/mingw-w64-static/toolchain-mingw-static.cmake
    set(pkgcfg_lib_libbrotlicommon_brotlicommon "${INVADER_MINGW_PREFIX}/lib/libbrotlicommon-static.a" CACHE INTERNAL "static libbrotlicommon")
    set(pkgcfg_lib_libbrotlienc_brotlienc "${INVADER_MINGW_PREFIX}/lib/libbrotlienc-static.a;${INVADER_MINGW_PREFIX}/lib/libbrotlicommon-static.a" CACHE INTERNAL "static libbrotliend")
    set(pkgcfg_lib_libbrotlidec_brotlidec "${INVADER_MINGW_PREFIX}/lib/libbrotlidec-static.a;${INVADER_MINGW_PREFIX}/lib/libbrotlicommon-static.a" CACHE INTERNAL "static libbrotlidec")
    set(libbrotlicommon_STATIC_LDFLAGS "${pkgcfg_lib_libbrotlicommon_brotlicommon}" CACHE INTERNAL "static libbrotlicommon")
    set(libbrotlienc_STATIC_LDFLAGS "${pkgcfg_lib_libbrotlienc_brotlienc}" CACHE INTERNAL "static libbrotliend")
    set(libbrotlidec_STATIC_LDFLAGS "${pkgcfg_lib_libbrotlidec_brotlidec}" CACHE INTERNAL "static libbrotlidec")
    set(OPENSSL_DEPENDENCIES "-lws2_32;-lgdi32;-lcrypt32" CACHE INTERNAL "dependencies of static OpenSSL libraries")
    set(POSTGRESQL_DEPENDENCIES "-lpgcommon;-lpgport;-lintl;-lssl;-lcrypto;-lshell32;-lws2_32;-lsecur32;-liconv" CACHE INTERNAL "dependencies of static PostgreSQL libraries")
    set(MYSQL_DEPENDENCIES "-lssl;-lcrypto;-lshlwapi;-lgdi32;-lws2_32;-lpthread;-lz;-lm" CACHE INTERNAL "dependencies of static MySQL/MariaDB libraries")
    set(LIBPNG_DEPENDENCIES "-lz" CACHE INTERNAL "dependencies of static libpng")
    set(GLIB2_DEPENDENCIES "-lintl;-lws2_32;-lole32;-lwinmm;-lshlwapi;-lm" CACHE INTERNAL "dependencies of static Glib2")
    set(FREETYPE_DEPENDENCIES "-lbz2;-lharfbuzz;-lfreetype;-lbrotlidec-static;-lbrotlicommon-static" CACHE INTERNAL "dependencies of static FreeType2 library")
    set(HARFBUZZ_DEPENDENCIES "-lglib-2.0;${GLIB2_DEPENDENCIES};-lintl;-lm;-lfreetype;-lgraphite2" CACHE INTERNAL "dependencies of static HarfBuzz library")
    set(DBUS1_DEPENDENCIES "-lws2_32;-liphlpapi;-ldbghelp" CACHE INTERNAL "dependencies of static D-Bus1 library")

    # Replace .lib.a with .a
    set(FREETYPE_LIBRARY_RELEASE        "${INVADER_MINGW_PREFIX}/lib/libfreetype.a")
    set(HARFBUZZ_LIBRARIES              "${INVADER_MINGW_PREFIX}/lib/libharfbuzz.a")
    set(JPEG_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libjpeg.a")
    set(LIB_EAY                         "${INVADER_MINGW_PREFIX}/lib/libcrypto.a")
    set(LibArchive_LIBRARY              "${INVADER_MINGW_PREFIX}/lib/libarchive.a")
    set(PCRE2_LIBRARY_DEBUG             "${INVADER_MINGW_PREFIX}/lib/libpcre2-16.a")
    set(PCRE2_LIBRARY_RELEASE           "${INVADER_MINGW_PREFIX}/lib/libpcre2-16.a")
    set(PNG_LIBRARY_RELEASE             "${INVADER_MINGW_PREFIX}/lib/libpng.a")
    set(SSL_EAY                         "${INVADER_MINGW_PREFIX}/lib/libssl.a")
    set(TIFF_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libtiff.a")
    set(ZLIB_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libz.a")
    set(pkgcfg_lib_PC_HARFBUZZ_harfbuzz "${INVADER_MINGW_PREFIX}/lib/libharfbuzz.a")
    set(pkgcfg_lib_PC_PCRE2_pcre2-16    "${INVADER_MINGW_PREFIX}/lib/libpcre2-16.a")

    set(OPENSSL_USE_STATIC_LIBS ON)
    set(BOOST_USE_STATIC_LIBS ON)

# Fix any dependencies needed to be fixed
else()
    set(DEP_SQUISH_LIBRARIES squish gomp)
    set(SDL2_LIBRARIES ${SDL2_LIBRARIES} -lsetupapi -limm32 -lversion -lwinmm)
    set(TIFF_LIBRARIES ${TIFF_LIBRARIES} jpeg lzma)
    set(LibArchive_LIBRARIES ${LibArchive_LIBRARIES} lzma zstd bz2 iconv)
    set(FREETYPE_LIBRARIES freetype png brotlidec-static brotlicommon-static bz2 harfbuzz graphite2 freetype)
endif()
