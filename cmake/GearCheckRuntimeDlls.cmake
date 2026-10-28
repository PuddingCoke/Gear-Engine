# =============================================================================
# GearCheckRuntimeDlls.cmake —— Libs 与 Runtime/Dlls 的配套关系检查
#
# 由 GearDemo.cmake 以 `cmake -P` 方式调用（Demo 构建前执行）：
#   cmake -DGEAR_LIBS_DIR=... -DGEAR_DLLS_DIR=... \
#         -DGEAR_DUMPBIN_EXECUTABLE=... -P GearCheckRuntimeDlls.cmake
#
# 对 Libs 下每个 .lib 分三类处理：
#   1. 正常 MSVC 导入库：dumpbin 能列出 "DLL name : xxx.dll"
#      -> 检查 Runtime/Dlls 下是否存在同名 DLL，缺失则报错（运行时必然加载失败）
#   2. 含 .idata$ 段但 dumpbin 认不出导入描述符（MinGW/dlltool 产物）
#      -> 报错：这种库无法用 MSVC 的 /DELAYLOAD 延迟加载，
#         需要用 dumpbin /exports + lib /def 重新生成 MSVC 导入库
#   3. 普通静态库：上面两者都没有，直接跳过
#
# 任何一项失败都以 FATAL_ERROR 结束，从而阻止后续的 Demo 构建。
# =============================================================================

if(NOT DEFINED GEAR_LIBS_DIR OR NOT DEFINED GEAR_DLLS_DIR)
    message(FATAL_ERROR "GearCheckRuntimeDlls: 缺少 GEAR_LIBS_DIR / GEAR_DLLS_DIR")
endif()

if(NOT EXISTS "${GEAR_DUMPBIN_EXECUTABLE}")
    message(FATAL_ERROR "GearCheckRuntimeDlls: dumpbin.exe 不存在：${GEAR_DUMPBIN_EXECUTABLE}")
endif()

file(GLOB_RECURSE _libs "${GEAR_LIBS_DIR}/*.lib")
list(SORT _libs)

set(_missing "")
set(_bad_import "")
set(_checked 0)

foreach(_lib IN LISTS _libs)
    # Engine 自身也输出到 Libs，是静态库，不参与检查
    if(_lib MATCHES "/Engine(-d)?\\.lib$")
        continue()
    endif()

    execute_process(
        COMMAND "${GEAR_DUMPBIN_EXECUTABLE}" /nologo /headers "${_lib}"
        OUTPUT_VARIABLE _out
        ERROR_QUIET
        RESULT_VARIABLE _rc)

    if(NOT _rc EQUAL 0)
        list(APPEND _bad_import "${_lib}（dumpbin 读取失败）")
        continue()
    endif()

    # dumpbin 输出形如：  DLL name     : avcodec-63.dll
    string(REGEX MATCHALL "[Dd][Ll][Ll] name[ \t]*:[ \t]*[^ \r\n]+" _matches "${_out}")
    if(_matches)
        list(REMOVE_DUPLICATES _matches)
        foreach(_match IN LISTS _matches)
            string(REGEX REPLACE "^[Dd][Ll][Ll] name[ \t]*:[ \t]*" "" _dll "${_match}")
            math(EXPR _checked "${_checked} + 1")
            if(NOT EXISTS "${GEAR_DLLS_DIR}/${_dll}")
                list(APPEND _missing "${_lib}  ->  ${_dll}")
            endif()
        endforeach()
    elseif(_out MATCHES "\\.idata\\$[0-9]")
        # 有导入段但 dumpbin 认不出导入描述符：MinGW/dlltool 风格的导入库
        list(APPEND _bad_import "${_lib}")
    endif()
endforeach()

if(_missing OR _bad_import)
    set(_detail "")
    if(_missing)
        string(APPEND _detail
            "\n以下导入库引用的 DLL 不在 Runtime/Dlls 中（运行时会加载失败）：\n")
        foreach(_item IN LISTS _missing)
            string(APPEND _detail "  ${_item}\n")
        endforeach()
    endif()
    if(_bad_import)
        string(APPEND _detail
            "\n以下 .lib 不是 MSVC 可用的导入库（含 .idata$ 段但认不出导入描述符，"
            "常见于 MinGW/dlltool 产物），无法用 /DELAYLOAD 延迟加载：\n")
        foreach(_item IN LISTS _bad_import)
            string(APPEND _detail "  ${_item}\n")
        endforeach()
        string(APPEND _detail
            "请用 `dumpbin /exports <dll>` 生成 .def，再用 "
            "`lib /def:<def> /machine:x64 /out:<lib>` 重新生成导入库。\n")
    endif()
    message(FATAL_ERROR "Gear 运行期依赖检查失败：${_detail}")
endif()

message(STATUS "Gear 运行期依赖检查通过（${_checked} 个导入库）")
