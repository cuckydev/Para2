all:
	$(MAKE) -f Makefile.mkext
	$(MAKE) -f Makefile.mkelf
	$(MAKE) -f Makefile.mkirx
	$(MAKE) -f Makefile.mkiso

clean:
	$(MAKE) -f Makefile.mkext clean
	$(MAKE) -f Makefile.mkelf clean
	$(MAKE) -f Makefile.mkirx clean
	$(MAKE) -f Makefile.mkiso clean
