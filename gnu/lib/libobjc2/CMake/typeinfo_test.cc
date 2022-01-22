#include <stdint.h>

namespace __cxxabiv1
{
	struct __class_type_info;
}

using __cxxabiv1::__class_type_info;

namespace std
{
	/**
	 * std::type_info defined with the GCC ABI.  This may not be exposed in
	 * public headers, but is required for correctly implementing the unified
	 * exception model.
	 */
	class type_info
	{
				public:
				virtual ~type_info();
				bool operator==(const type_info &) const;
				bool operator!=(const type_info &) const;
				bool before(const type_info &) const;
				private:
				type_info(const type_info& rhs);
				type_info& operator= (const type_info& rhs);
				const char *__type_name;
				protected:
				type_info(const char *name): __type_name(name) { }
				public:
				const char* name() const { return __type_name; }
				virtual bool __is_pointer_p() const;
				virtual bool __is_function_p() const;
				virtual bool __do_catch(const type_info *thrown_type,
				                        void **thrown_object,
				                        unsigned outer) const;
				virtual bool __do_upcast(
				                const __class_type_info *target,
				                void **thrown_object) const;
	};
}

class type_info2 : public std::type_info
{
	public:
	type_info2() : type_info("foo") {}
	virtual bool __is_pointer_p() const; 
	virtual bool __is_function_p() const { return true; }
	virtual bool __do_catch(const type_info *thrown_type,
	                        void **thrown_object,
	                        unsigned outer) const { return true; }
	virtual bool __do_upcast(
					const __class_type_info *target,
					void **thrown_object) const { return true; };
};
bool type_info2::__is_pointer_p() const { return true; }

int main()
{
	type_info2 s;
	return s.__is_pointer_p();
}
