#include "ipc.h"

#ifdef _WIN32

#include <windows.h>
#include <stdexcept>
#include <string>

namespace ipc {

    namespace {
        std::string make_mapping_name(const std::string& name) {
            return "Local\\" + name + "_Mapping";
        }
        std::string make_mutex_name(const std::string& name) {
            return "Local\\" + name + "_Mutex";
        }
        void throw_last_error(const std::string& context) {
            DWORD err = GetLastError();
            throw std::runtime_error(context + " (error " + std::to_string(err) + ")");
        }
    }//anonymous namespace

    std::uint8_t* SharedSession::data() noexcept { return view_; }
    const std::uint8_t* SharedSession::data() const noexcept { return view_; }
    std::size_t SharedSession::size() const noexcept { return size_; }

    void SharedSession::close() noexcept {
        if (view_) { UnmapViewOfFile(view_); view_ = nullptr; }
        if (mapping_) { CloseHandle(static_cast<HANDLE>(mapping_)); mapping_ = nullptr; }
        if (mutex_) { CloseHandle(static_cast<HANDLE>(mutex_)); mutex_ = nullptr; }
        size_ = 0;
    }

    SharedSession::~SharedSession() {
        close();
    }

    SharedSession::SharedSession(SharedSession&& other) noexcept
        : mapping_(other.mapping_), mutex_(other.mutex_), view_(other.view_),
        size_(other.size_), owner_(other.owner_) {
        other.mapping_ = nullptr;
        other.mutex_ = nullptr;
        other.view_ = nullptr;
        other.size_ = 0;
        other.owner_ = false;
    }

    SharedSession& SharedSession::operator=(SharedSession&& other) noexcept {
        if (this != &other) {
            close();
            mapping_ = other.mapping_;
            mutex_ = other.mutex_;
            view_ = other.view_;
            size_ = other.size_;
            owner_ = other.owner_;
            other.mapping_ = nullptr;
            other.mutex_ = nullptr;
            other.view_ = nullptr;
            other.size_ = 0;
            other.owner_ = false;
        }
        return *this;
    }

    void SharedSession::lock() {
        if (!mutex_) throw std::runtime_error("SharedSession::lock: mutex not open");
        DWORD res = WaitForSingleObject(static_cast<HANDLE>(mutex_), INFINITE);
        if (res != WAIT_OBJECT_0) throw_last_error("WaitForSingleObject");
    }

    void SharedSession::unlock() {
        if (!mutex_) return;
        ReleaseMutex(static_cast<HANDLE>(mutex_));
    }

    SharedSession SharedSession::create(const std::string& name, std::size_t buffer_size) {
        SharedSession s;
        s.size_ = buffer_size;
        s.owner_ = true;

        HANDLE mapping = CreateFileMappingA(
            INVALID_HANDLE_VALUE,   // pagefile
            nullptr,                // default security
            PAGE_READWRITE,
            0, static_cast<DWORD>(buffer_size),
            make_mapping_name(name).c_str()
        );
        if (!mapping) throw_last_error("CreateFileMapping");
        s.mapping_ = mapping;

        void* view = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, buffer_size);
        if (!view) {
            CloseHandle(mapping);
            throw_last_error("MapViewOfFile");
        }
        s.view_ = static_cast<std::uint8_t*>(view);

        HANDLE mutex = CreateMutexA(nullptr, FALSE, make_mutex_name(name).c_str());
        if (!mutex) {
            UnmapViewOfFile(view);
            CloseHandle(mapping);
            throw_last_error("CreateMutex");
        }
        s.mutex_ = mutex;

        return s;
    }

    SharedSession SharedSession::open(const std::string& name) {
        SharedSession s;
        s.owner_ = false;

        HANDLE mapping = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, make_mapping_name(name).c_str());
        if (!mapping) throw_last_error("OpenFileMapping");
        s.mapping_ = mapping;

        void* view = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);  
        if (!view) {
            CloseHandle(mapping);
            throw_last_error("MapViewOfFile");
        }
        s.view_ = static_cast<std::uint8_t*>(view);

        MEMORY_BASIC_INFORMATION info;
        if (VirtualQuery(view, &info, sizeof(info)) == 0) {
            UnmapViewOfFile(view);
            CloseHandle(mapping);
            throw_last_error("VirtualQuery");
        }
        s.size_ = info.RegionSize;

        HANDLE mutex = OpenMutexA(SYNCHRONIZE, FALSE, make_mutex_name(name).c_str());
        if (!mutex) {
            UnmapViewOfFile(view);
            CloseHandle(mapping);
            throw_last_error("OpenMutex");
        }
        s.mutex_ = mutex;

        return s;
    }


}//namespace ipc

#else
namespace ipc {
    SharedSession SharedSession::create(const std::string&, std::size_t) {
        throw std::runtime_error("ipc::SharedSession not supported on this platform");
    }
    SharedSession SharedSession::open(const std::string&) {
        throw std::runtime_error("ipc::SharedSession not supported on this platform");
    }
    SharedSession::~SharedSession() = default;
    SharedSession::SharedSession(SharedSession&&) noexcept = default;
    SharedSession& SharedSession::operator=(SharedSession&&) noexcept = default;
    void SharedSession::close() noexcept {}
    void SharedSession::lock() {}
    void SharedSession::unlock() {}
    std::uint8_t* SharedSession::data() noexcept { return nullptr; }
    const std::uint8_t* SharedSession::data() const noexcept { return nullptr; }
    std::size_t SharedSession::size() const noexcept { return 0; }
#endif