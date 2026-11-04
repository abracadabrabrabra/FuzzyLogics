#pragma once
#include <cstdint>
#include <string>
#include <cstddef>
#include <stdexcept>

namespace ipc {

	class SharedSession {

	private:
		
		void close() noexcept;

		void* mapping_ = nullptr; // HANDLE
		void* mutex_ = nullptr; // HANDLE
		std::uint8_t* view_ = nullptr;
		std::size_t size_ = 0;
		bool owner_ = false;

	public:
		static SharedSession create(const std::string& name, std::size_t buffer_size);
		static SharedSession open(const std::string& name);
		SharedSession() = default;
		~SharedSession();

		SharedSession(const SharedSession& other) = delete;
		SharedSession& operator=(const SharedSession& other) = delete;

		SharedSession(SharedSession&& other) noexcept;
		SharedSession& operator=(SharedSession&& other) noexcept;

		void lock();
		void unlock();

		std::uint8_t* data() noexcept;
		const std::uint8_t* data() const noexcept;
		std::size_t size() const noexcept;
	};

}//namespace ipc