#include "stdafx.h"
#include "W32Cmd.h"
#include <windows.h>
#include <fstream>
namespace TR { namespace External {

class Handle
{
public:
    Handle():
        m_handle(0)
    {
    }
    
    explicit Handle(HANDLE handle):
        m_handle(handle)
    {
    }

    Handle(const Handle& handle) = delete;
    Handle(Handle&& handle) noexcept:
        m_handle(handle.m_handle)
    {
        handle.m_handle = 0;
    }
    
    Handle& operator = (const Handle& handle) = delete;
    //Handle& operator = (Handle&& handle) = delete;


    ~Handle()
    {
        if (m_handle)
        {
            ::CloseHandle(m_handle);
        }
    }

    operator HANDLE() const
    {
        return m_handle;
    }

    HANDLE release()
    {
        auto handle = m_handle;
        m_handle = 0;
        return handle;
    }

    void close()
    {
        if (m_handle)
        {
            ::CloseHandle(m_handle);
            m_handle = 0;
        }
    }

    private:
        HANDLE m_handle;
    };

std::runtime_error get_system_error(DWORD error_code = ::GetLastError())
{
    void* msg;
    ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error_code,
        MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
        (LPSTR)&msg,
        0, NULL);

    std::runtime_error err(static_cast<LPSTR>(msg));
    LocalFree(msg);
    return err;
}

Handle create_process(const std::wstring& cmd_line, HANDLE std_input, HANDLE std_output, HANDLE std_error)
{
    STARTUPINFO startup_info = { 0 };
    startup_info.cb = sizeof(STARTUPINFO);
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    startup_info.hStdInput = std_input;
    startup_info.hStdOutput = std_output;
    startup_info.hStdError = std_error;
    
    PROCESS_INFORMATION process_information = { 0 };
    if (!::CreateProcess(NULL,
        LPTSTR(cmd_line.c_str()),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &startup_info,
        &process_information))
    {
        throw get_system_error();
    }
    return Handle(process_information.hProcess);
}

struct Pipe
{
    Pipe(Handle read_handle, Handle write_handle):
        m_read_handle(std::move(read_handle)),
        m_write_handle(std::move(write_handle))
    {
    }

    Pipe(const Pipe& pipe_handle) = delete;
    Pipe(Pipe&& pipe_handle) noexcept:
        m_read_handle(std::move(pipe_handle.m_read_handle)),
        m_write_handle(std::move(pipe_handle.m_write_handle))
    {
    }

    Handle& operator = (const Handle& handle) = delete;
    //Handle& operator = (Handle&& handle) = delete;

    Handle m_read_handle;
    Handle m_write_handle;
};

Pipe create_pipe()
{
    HANDLE read_handle = 0;
    HANDLE write_handle = 0;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!::CreatePipe(&read_handle, &write_handle, &sa, 10*1024*1024))
    {
        throw get_system_error();
    }
    return Pipe(Handle(read_handle), Handle(write_handle));
}

Blob read_pipe(HANDLE handle)
{
    static char buff[1000] = { 0 };
    
    Blob data;
    DWORD count = 0;
    while (::ReadFile(handle, buff, _countof(buff), &count, nullptr))
    {
        std::copy(buff, buff + count, std::back_inserter(data));
        count = 0;
    }

    if (::GetLastError() != ERROR_BROKEN_PIPE)
    {
        throw get_system_error();
    }

    return data;
} 

void write_pipe(HANDLE handle, const Blob& data)
{
    DWORD count = 0;
    if (!::WriteFile(handle, &data[0], data.size(), &count, nullptr))
    {
        throw get_system_error();
    }
}

Blob exec_command(const std::wstring& path, const Blob& input)
{
    auto std_input = create_pipe();
    auto std_output = create_pipe();
    auto std_error = create_pipe();

    write_pipe(std_input.m_write_handle, input);
    std_input.m_write_handle.close();
    
    auto process_handle = create_process(path, std_input.m_read_handle, std_output.m_write_handle, std_error.m_write_handle);
    std_output.m_write_handle.close();
    std_error.m_write_handle.close();

    if (::WaitForSingleObject(process_handle, 5 * 60 * 1000) != WAIT_OBJECT_0)
    {
        throw std::runtime_error("External process wait timeout");
    }

    DWORD exit_code = 0;
    if (!::GetExitCodeProcess(process_handle, &exit_code))
    {
        throw get_system_error();
    }

    if (exit_code != 0)
    {
        auto err = read_pipe(std_error.m_read_handle);
        if (err.empty())
        {
            throw std::runtime_error("Invalid exit code of external process");
        }
        throw std::runtime_error({err.begin(), err.end()});
    }

    return read_pipe(std_output.m_read_handle);
}

std::string read_text(const std::wstring& file_system_path)
{
    std::ifstream fs(file_system_path);
    if (!fs)
    {
        throw std::runtime_error("Can`t read file");
    }

    return {std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>()};
}

}} //namespace TR { namespace External {