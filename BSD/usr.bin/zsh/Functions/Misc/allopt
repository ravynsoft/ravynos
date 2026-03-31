# This function lists options with the no's in front removed for
# improved comprehension, i.e. `norcs off' becomes `rcs on'.
# The format is otherwise like that with `kshoptionprint' set,
# i.e. you can see all options whether on or off.
# It can take a list of option names or parts thereof to search for
# via egrep.
#
# Written by Sweth Chandramouli with hacks by Bart Schaefer.

listalloptions () {
   local OPT_NAME OPT_VALUE IFS=$' \t\n'
   builtin set -o | while read OPT_NAME OPT_VALUE ; do
      if [[ ${OPT_NAME#no} != ${OPT_NAME} ]] ; then
	 OPT_VALUE=${(L)${${OPT_VALUE:s/on/OFF}:s/off/on}}
	 OPT_NAME=${OPT_NAME#no}
      fi
      echo "${(r:21:)OPT_NAME} ${OPT_VALUE}"
   done
}

if [[ -n $@ ]]; then
    listalloptions | egrep "${(j.|.)@}"
else
    listalloptions
fi
