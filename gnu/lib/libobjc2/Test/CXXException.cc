
extern "C" void throw_int()
{
	throw 12;
}

extern "C" void throw_id();


extern "C" int catchall()
{
	try
	{
		throw_id();
	}
	catch(...)
	{
		throw;
	}
	__builtin_trap();
}
