.PHONY: default andestar decoder cc_decoder sim_decoder run_test cc_test sim_test \
        zfa zicond

include Vars.mk

ifeq ($(VERBOSE),1)
    Q :=
else
    Q := @
    MAKEFLAGS += -s  # Enable silent mode
endif

default: andestar decoder
#default: andestar zicond

decoder: cc_decoder sim_decoder

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

# --------------------------------------------------------------------------------
#zfa   fcvtmod.w.d  compiler does not like operands supplied
#zfa   FMVH.X.d  RV32 only
#zfa   FMVP.d.X  RV32 only

zfa_tests=fleq.d fleq.h fleq.q fleq.S fli.d fli.h fli.q fli.s \
          fltq.d fltq.h fltq.q fltq.s fmaxm.d fmaxm.h fmaxm.q fmaxm.s fminm.d \
          fminm.h fminm.q fminm.s fmvh.x.q fmvp.q.x fround.d fround.h fround.q \
          fround.s froundnx.d froundnx.h froundnx.q froundnx.s

zfa:	
	mkdir -p bin/zfa logs/zfa
	$(foreach test,$(zfa_tests),$(MAKE) run_test X=rv64ui-p E=zfa T=$(test);)

zicond:	
	mkdir -p bin/zicond logs/zicond
	$(MAKE) run_test X=rv64ui-p E=zicond T=czero_eqz
	$(MAKE) run_test X=rv64ui-p E=zicond T=czero_nez


# --------------------------------------------------------------------------------
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

# --------------------------------------------------------------------------------
cc_decoder:
	$(Q)mkdir -p bin/decoder logs/decoder
	$(CONDOR_CC) $(CONDOR_CC_OPTS) $(COMMON_LINK) \
  -I./src/decoder src/decoder/*.c src/decoder/*.S \
  -o bin/decoder/decoder.riscv -lm -lgcc
	$(Q)$(CONDOR_OBJD) $(CONDOR_OBJD_OPTS) \
	  ./bin/decoder/decoder.riscv > ./logs/decoder/decoder.riscv.dump

sim_decoder:
	$(Q)$(CONDOR_SIM) $(CONDOR_SIM_OPTS) \
	  ./bin/decoder/decoder.riscv > ./logs/decoder/decoder.log 2>&1 && \
	echo "Running decoder.riscv ...pass" || echo "Running decoder.riscv ...fail"

# --------------------------------------------------------------------------------
clean:
	-rm -rf bin/* logs/*

