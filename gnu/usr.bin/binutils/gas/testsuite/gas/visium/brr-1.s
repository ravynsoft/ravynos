        .text
foo:
	brr tr,foo
	 moviq r1,1
	brr tr,0
	 moviq r1,2
	brr tr,foo
	 moviq r1,4
	brr fa,.
