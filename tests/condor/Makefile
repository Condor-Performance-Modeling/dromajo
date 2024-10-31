.PHONY: default andestar icond run_test cc_test sim_test

include Vars.mk

ifeq ($(VERBOSE),1)
    Q :=
else
    Q := @
    MAKEFLAGS += -s  # Enable silent mode
endif

default: andestar
#default: andestar zicond

andestar:	
	mkdir -p bin/andestar logs/andestar
	$(MAKE) run_test X=rv64ui-p E=andestar T=addigp
	$(MAKE) run_test X=rv64ui-p E=andestar T=bbc
	$(MAKE) run_test X=rv64ui-p E=andestar T=bbs
	$(MAKE) run_test X=rv64ui-p E=andestar T=beqc
	$(MAKE) run_test X=rv64ui-p E=andestar T=bnec
	$(MAKE) run_test X=rv64ui-p E=andestar T=bfos
	$(MAKE) run_test X=rv64ui-p E=andestar T=bfoz
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_h
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_w
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_d
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_b_ze
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_h_ze
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_w_ze
	$(MAKE) run_test X=rv64ui-p E=andestar T=lea_d_ze
	$(MAKE) run_test X=rv64ui-p E=andestar T=lbgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=lbugp
	$(MAKE) run_test X=rv64ui-p E=andestar T=lhgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=lhugp	
	$(MAKE) run_test X=rv64ui-p E=andestar T=lwgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=lwugp
	$(MAKE) run_test X=rv64ui-p E=andestar T=ldgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=sbgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=shgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=swgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=sdgp
	$(MAKE) run_test X=rv64ui-p E=andestar T=ffb
	$(MAKE) run_test X=rv64ui-p E=andestar T=ffzmism
	$(MAKE) run_test X=rv64ui-p E=andestar T=ffmism
	$(MAKE) run_test X=rv64ui-p E=andestar T=flmism

zicond:	
	mkdir -p bin/zicond logs/zicond
	$(MAKE) run_test X=rv64ui-p E=zicond T=czero_eqz
	$(MAKE) run_test X=rv64ui-p E=zicond T=czero_neq

run_test:
	$(MAKE) cc_test  X=$(X) EXT=$(E) TST=$(T)
	$(MAKE) sim_test X=$(X) EXT=$(E) TST=$(T)

cc_test:	
	$(Q)$(ANDES_CC) $(ANDES_CC_OPTS) -DXLEN=$(X) \
	  ./src/$(EXT)/$(TST).S -o ./bin/$(EXT)/$(X)-$(TST)
	$(Q)$(ANDES_OBJD) $(ANDES_OBJD_OPTS) \
	  ./bin/$(EXT)/$(X)-$(TST) > ./logs/$(EXT)/$(X)-$(TST).dump

sim_test:
	$(Q)$(ANDES_SIM) $(ANDES_SIM_OPTS) \
	  ./bin/$(EXT)/$(X)-$(TST) > ./logs/$(EXT)/$(X)-$(TST).log 2>&1 && \
	echo "Running $(X)-$(TST) ...pass" || echo "Running $(X)-$(TST) ...fail"

clean:
	-rm -rf bin/* logs/*

