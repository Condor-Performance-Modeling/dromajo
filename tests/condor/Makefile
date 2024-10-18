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
	$(MAKE) run_andestar X=rv64ui-p T=addigp
	$(MAKE) run_andestar X=rv64ui-p T=bfos
	$(MAKE) run_andestar X=rv64ui-p T=bfoz
	$(MAKE) run_andestar X=rv64ui-p T=lea_h
	$(MAKE) run_andestar X=rv64ui-p T=lea_w
	$(MAKE) run_andestar X=rv64ui-p T=lea_d
	$(MAKE) run_andestar X=rv64ui-p T=lea_b_ze
	$(MAKE) run_andestar X=rv64ui-p T=lea_h_ze
	$(MAKE) run_andestar X=rv64ui-p T=lea_w_ze
	$(MAKE) run_andestar X=rv64ui-p T=lea_d_ze
	$(MAKE) run_andestar X=rv64ui-p T=lbgp
	$(MAKE) run_andestar X=rv64ui-p T=lbugp
	$(MAKE) run_andestar X=rv64ui-p T=lhgp
	$(MAKE) run_andestar X=rv64ui-p T=lhugp	
	$(MAKE) run_andestar X=rv64ui-p T=lwgp
	$(MAKE) run_andestar X=rv64ui-p T=lwugp
	$(MAKE) run_andestar X=rv64ui-p T=ldgp
	$(MAKE) run_andestar X=rv64ui-p T=sbgp
	$(MAKE) run_andestar X=rv64ui-p T=shgp
	$(MAKE) run_andestar X=rv64ui-p T=swgp
	$(MAKE) run_andestar X=rv64ui-p T=sdgp

run_andestar:
	$(MAKE) cc_andestar  X=$(X) TST=$(T)
	$(MAKE) sim_andestar X=$(X) TST=$(T)

cc_andestar:	
	$(Q)$(ANDES_CC) $(ANDES_CC_OPTS) -DXLEN=$(X) \
      ./src/andestar/$(TST).S -o ./bin/andestar/$(X)-$(TST)
	$(Q)$(ANDES_OBJD) $(ANDES_OBJD_OPTS) \
    ./bin/andestar/$(X)-$(TST) > ./logs/andestar/$(X)-$(TST).dump

sim_andestar:
	$(Q)$(ANDES_SIM) $(ANDES_SIM_OPTS) \
	    ./bin/andestar/$(X)-$(TST) > ./logs/andestar/$(X)-$(TST).log 2>&1 && \
	echo "Running $(X)-$(TST) ...pass" || echo "Running $(X)-$(TST) ...fail"

#sim_andestar:
#	$(Q)$(ANDES_SIM) $(ANDES_SIM_OPTS)  ./bin/andestar/$(TST) > \
#                                     ./logs/andestar/$(TST).log 2>&1 && \
#	echo "Running $(TST) ...pass" || { echo "Running $(X)-$(TST) ...fail"; exit 1; }
#
clean:
	-rm -rf bin/* logs/*

