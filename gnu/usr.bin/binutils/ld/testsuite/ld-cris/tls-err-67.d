#source: init.s
#source: tls-gdgotrelm.s --defsym r=8203
#as: --no-underscore --em=criself --pic
#ld: --shared -m crislinux
#error: \A[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*truncated[^\n]*\n[^\n]*too many[^\n]*\n[^\n]*\.o: in[^\n]*\n[^\n]*omitted[^\n]*\Z

# Check that the error messages get the right number of appended
# explanations with default values when bailing out and omitting
# further error messages for overflows.
