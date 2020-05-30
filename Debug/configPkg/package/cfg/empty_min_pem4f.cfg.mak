# invoke SourceDir generated makefile for empty_min.pem4f
empty_min.pem4f: .libraries,empty_min.pem4f
.libraries,empty_min.pem4f: package/cfg/empty_min_pem4f.xdl
	$(MAKE) -f D:\EntireUniverse\UniStuff\2020\Semester1\Systems\Assignment\EGH456/src/makefile.libs

clean::
	$(MAKE) -f D:\EntireUniverse\UniStuff\2020\Semester1\Systems\Assignment\EGH456/src/makefile.libs clean

