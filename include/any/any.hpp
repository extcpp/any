#ifndef INCLGUARD_any_hpp
#define INCLGUARD_any_hpp

#include <typeinfo>

namespace any
{
	namespace detail
	{
		enum class mode
		{
			destroy,
			copy,
			move
		};

		template<typename T> void interface(mode, void*, void*);

		using interface_t = void (*)(mode, void*, void*);

		template<typename T>
		struct model
		{
			template<typename... Args>
			model(Args&&... args)
				: interface(&detail::interface<T>),
				object(std::forward<Args>(args)...)
			{ }

			model(model const&) = default;
			model& operator= (model const&) = default;
			model(model&&) = default;
			model& operator= (model&&) = default;

			interface_t interface;
			T object;
		};

		template<typename T>
		void interface(mode m, void* ptr1, void* ptr2)
		{
			switch(m)
			{
			case mode::destroy:
				reinterpret_cast<model<T>*>(ptr1)->~model<T>();
				reinterpret_cast<model<T>*>(ptr1)->interface = nullptr;
				break;
			case mode::copy:
				new(ptr1) model<T>(*reinterpret_cast<model<T> const*>(ptr2));
				break;
			case mode::move:
				new(ptr1) model<T>(std::move(*reinterpret_cast<model<T>*>(ptr2)));
				break;
			}
		}

		std::uintptr_t typeid_by_data(void* any_data)
		{
			return *reinterpret_cast<std::uintptr_t*>(any_data);
		}

		template<typename T>
		std::uintptr_t typeid_by_type()
		{
			return reinterpret_cast<std::uintptr_t>(&interface<T>);
		}

	} // namespace detail

	template<std::size_t Size, std::size_t Alignment>
	class alignas(Alignment) any
	{
	public:
		constexpr static std::size_t size = Size;
		constexpr static std::size_t alignment = Alignment;

		template<typename _T, std::size_t _Size, std::size_t _Alignment>
		friend _T& any_cast(any<_Size, _Alignment>& a);

		template<typename _T, std::size_t _Size, std::size_t _Alignment>
		friend bool valid_cast(any<_Size, _Alignment>& a);

		~any()
		{
			destroy();
		}

		any()
		{
			std::memset(data, 0, size);
		}

		template<typename T,
			typename = std::enable_if_t<
				sizeof(detail::model<std::decay_t<T>>) <= size &&
				alignof(detail::model<std::decay_t<T>>) <= alignment && 
				std::is_copy_constructible<std::remove_reference_t<std::remove_cv_t<T>>>::value &&
				std::is_move_constructible<std::remove_reference_t<std::remove_cv_t<T>>>::value
			>
		>
		any(T&& object)
		{
			new(data) detail::model<std::decay_t<T>>(std::forward<T>(object));
		}

		any(any const& other)
		{
			assert(this != &other && "ill formed initialization");
			interface(other.data)(detail::mode::copy, data, reinterpret_cast<void*>(const_cast<char*>(other.data)));
		}

		any(any&& other)
		{
			assert(this != &other && "ill formed initialization");
			interface(other.data)(detail::mode::move, data, other.data);
		}

		any& operator= (any const& other)
		{
			if(this == &other)
				return *this;

			destroy();
			interface(other.data)(detail::mode::copy, data, reinterpret_cast<void*>(const_cast<char*>(other.data))); // const_cast to satisfy the interface; other data will only be read
			return *this;
		}

		any& operator= (any&& other)
		{
			if(this == &other)
				return *this;

			destroy();
			interface(other.data)(detail::mode::move, data, other.data);
			return *this;
		}

	private:
		static detail::interface_t interface(char const* data)
		{
			return reinterpret_cast<detail::interface_t>(*reinterpret_cast<std::uintptr_t const*>(data));
		}

		void destroy()
		{
			if(*reinterpret_cast<std::uintptr_t*>(data))
				interface(data)(detail::mode::destroy, data, nullptr);
		}

	private:
		template<typename T> friend T* any_cast(any&);
		template<typename T> friend T const* any_cast(any const&);

	private:
		char data[size];
	};

	template<typename T, std::size_t Size, std::size_t Alignment>
	bool valid_cast(any<Size, Alignment>& a)
	{
		return detail::typeid_by_data(a.data) == detail::typeid_by_type<T>();
	}

	template<typename T, std::size_t Size, std::size_t Alignment>
	T& any_cast(any<Size, Alignment>& a)
	{
		assert(valid_cast<T>(a) && "any_cast: any does not contain given type");
		return reinterpret_cast<detail::model<T>*>(a.data)->object;
	}
} // namespace any

#endif
