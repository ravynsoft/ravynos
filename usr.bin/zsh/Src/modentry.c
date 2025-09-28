#include "zsh.mdh"

int setup_ _((Module));
int boot_ _((Module));
int cleanup_ _((Module));
int finish_ _((Module));
int modentry _((int boot, Module m, void *ptr));

/**/
int
modentry(int boot, Module m, void *ptr)
{
    switch (boot) {
    case 0:
	return setup_(m);
	break;

    case 1:
	return boot_(m);
	break;

    case 2:
	return cleanup_(m);
	break;

    case 3:
	return finish_(m);
	break;

    case 4:
	return features_(m, (char ***)ptr);
	break;

    case 5:
	return enables_(m, (int **)ptr);
	break;

    default:
	zerr("bad call to modentry");
	return 1;
	break;
    }
}
