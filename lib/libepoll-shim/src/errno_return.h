#ifndef ERRNO_RETURN_H_
#define ERRNO_RETURN_H_

#define ERRNO_SAVE int const oe = errno

#define ERRNO_RETURN(ec, rc_on_error, rc_on_success) \
	if ((ec) != 0) {                             \
		errno = (ec);                        \
		return (rc_on_error);                \
	}                                            \
	errno = oe;                                  \
	return (rc_on_success)

#endif
