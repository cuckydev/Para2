all:
	$(MAKE) -f Makefile.mkext
	$(MAKE) -f Makefile.mkirx
	$(MAKE) -f Makefile.mkelf
	$(MAKE) -f Makefile.mkiso

clean:
	$(MAKE) -f Makefile.elf clean
