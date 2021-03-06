# Makefile.  Generated from Makefile.in by configure.
 
TOPSRC      = 


CONFIG_DIR  = $(TOPSRC)
SUBDIRS     = src

all bare install conf clean realclean tests :: Makefile
	@for i in $(SUBDIRS) ; \
	do echo; \
	  echo 'Doing a make [$@] in <<' $$i '>>'; \
	  cd $$i ; $(MAKE) $@; cd ..; \
	done ;\
	echo; echo '* * * make [$@] completed at ' `date` ' * * *'

distclean :: realclean
	@cd $(TOPSRC); \
	rm -f config.status config.log \
	      Makefile $(CONFIG_DIR)/Makerules $(CONFIG_DIR)/Makedefs \
	      include/*.[Hh] lib/*.a lib/*.so bin/*; \
	cp Makeboot Makefile

Makefile :: configure $(CONFIG_DIR)/Makefile.in config.status
	./config.status

configure :: configure.ac acsite.m4
	cd $(TOPSRC) && autoconf

config.status :: configure
	./config.status --recheck

showvars ::
	@echo "***"; \
	 echo "*** Change dir to the lower levels and run this command."; \
	 echo "***"

