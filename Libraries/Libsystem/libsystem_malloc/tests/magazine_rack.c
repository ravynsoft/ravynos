//
//  magazine_rack.c
//  libmalloc
//
//  Created by Matt Wright on 8/29/16.
//
//

#include <darwintest.h>
#include "magazine_testing.h"

T_GLOBAL_META(T_META_RUN_CONCURRENTLY(true));

T_DECL(basic_magazine_init, "allocate magazine counts")
{
	struct rack_s rack;

	for (int i=1; i < 64; i++) {
		memset(&rack, 'a', sizeof(rack));
		rack_init(&rack, RACK_TYPE_NONE, i, 0);
		T_ASSERT_NOTNULL(rack.magazines, "%d magazine initialisation", i);
	}
}

T_DECL(basic_magazine_deinit, "allocate deallocate magazines")
{
	struct rack_s rack;
	memset(&rack, 'a', sizeof(rack));

	rack_init(&rack, RACK_TYPE_NONE, 1, 0);
	T_ASSERT_NOTNULL(rack.magazines, "magazine init");

	rack_destroy(&rack);
	T_ASSERT_NULL(rack.magazines, "magazine deinit");
}
