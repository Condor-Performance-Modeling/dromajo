.PHONY: default andestar cc_andestar sim_andestar

include Vars.mk

ifeq ($(VERBOSE),1)
    Q :=
else
    Q := @
    MAKEFLAGS += -s  # Enable silent mode
endif

default: andestar

andestar:	
	mkdir -p bin/andestar logs/andestar
	$(MAKE) run_andestar TST=addi
	# $(MAKE) run_andestar TST=addigp
	$(MAKE) run_andestar TST=lwgp
	$(MAKE) run_andestar TST=lwugp
	$(MAKE) run_andestar TST=sdgp

run_andestar:
	$(MAKE) cc_andestar  TST=$(TST)
	$(MAKE) sim_andestar TST=$(TST)

cc_andestar:	
	$(Q)$(ANDES_CC) $(ANDES_CC_OPTS) -o ./bin/andestar/$(TST) \
	                                    ./src/andestar/$(TST).S
	$(Q)$(ANDES_OBJD) $(ANDES_OBJD_OPTS) ./bin/andestar/$(TST) > \
                                      ./logs/andestar/$(TST).dump

sim_andestar:
	#@echo "running $(TST)"
	$(Q)$(ANDES_SIM) $(ANDES_SIM_OPTS)  ./bin/andestar/$(TST) > \
                                     ./logs/andestar/$(TST).log 2>&1 && \
	echo "Running $(TST) ...pass" || { echo "Running $(X)-$(TST) ...fail"; exit 1; }

clean:
	-rm -rf bin/* logs/*

