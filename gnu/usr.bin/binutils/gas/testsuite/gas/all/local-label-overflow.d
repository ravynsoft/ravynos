#source: local-label-overflow.s
#error_output: local-label-overflow.l
# Some hppa targets support local labels, others don't. It's a pain to
# enumerate all the combinations so just don't run the test for hppa.
#notarget: hppa*-*-*
#xfail: ia64-*-vms mmix-*-* sh-*-pe
