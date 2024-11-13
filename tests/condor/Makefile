.PHONY: default andestar decoder cc_decoder sim_decoder run_test cc_test sim_test \
        zfa zicond

include Vars.mk

ifeq ($(VERBOSE),1)
    Q :=
else
    Q := @
    MAKEFLAGS += -s  # Enable silent mode
endif

default: andestar zicond 
#zfa
#decoder

decoder: cc_decoder sim_decoder

andestar:	
	mkdir -p bin/andestar logs/andestar
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=addigp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=bbc
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=bbs
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=beqc
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=bnec
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=bfos
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=bfoz
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_h
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_w
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_d
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_b_ze
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_h_ze
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_w_ze
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lea_d_ze
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lbgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lbugp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lhgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lhugp	
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lwgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=lwugp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=ldgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=sbgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=shgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=swgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=sdgp
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=ffb
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=ffzmism
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=ffmism
	$(MAKE) andes_run_test X=rv64ui-p E=andestar T=flmism

# --------------------------------------------------------------------------------
# Open compiler does not like operands supplied
# fcvtmod.*.*
#
# RV32 only
# fmvh.x.d
# fmvh.d.x
# fmvp.x.d
# fmvp.d.x
#
# No Q support needed at the moment
# fleq_q fli_q fltq_q fmaxm_q fminm_q fmvh_x_q fmvp_q_x fround_q froundnx_q fmvp_q_x
# --------------------------------------------------------------------------------
zfa_tests = fleq_h fleq_s fleq_d \
            fltq_h fltq_s fltq_d \
            fmaxm_h fmaxm_s fmaxm_d \
            fminm_h fminm_s fminm_d \
            fround_h fround_s fround_d \
            froundnx_h froundnx_s froundnx_d \
            fmvh_x_d \
            fcvtmod_w_d \
            fli_h fli_s fli_d

zfa:
	-mkdir -p bin/zfa logs/zfa
	$(MAKE) run_test X=rv64ud-p E=zfa T=fleq_d

#	$(foreach test,$(zfa_tests),$(MAKE) run_test X=rv64ui-p E=zfa T=$(test);)

zicond:	
	mkdir -p bin/zicond logs/zicond
	$(MAKE) run_test X=rv64ui-p E=zicond T=czero_eqz
	$(MAKE) run_test X=rv64ui-p E=zicond T=czero_nez

# -------------------------------------------------------------------------
andes_run_test:
	$(MAKE) andes_cc_test  X=$(X) EXT=$(E) TST=$(T)
	$(MAKE) andes_sim_test X=$(X) EXT=$(E) TST=$(T)

andes_cc_test:	
	$(Q)$(ANDES_CC) $(ANDES_CC_OPTS) -DXLEN=$(X) \
	  ./src/$(EXT)/$(TST).S -o ./bin/$(EXT)/$(X)-$(TST)
	$(Q)$(ANDES_OBJD) $(ANDES_OBJD_OPTS) \
	  ./bin/$(EXT)/$(X)-$(TST) > ./logs/$(EXT)/$(X)-$(TST).dump

andes_sim_test:
	$(Q)$(ANDES_SIM) $(ANDES_SIM_OPTS) \
	  ./bin/$(EXT)/$(X)-$(TST) > ./logs/$(EXT)/$(X)-$(TST).log 2>&1 && \
	echo "Running $(X)-$(TST) ...pass" || echo "Running $(X)-$(TST) ...fail"

# -------------------------------------------------------------------------
cc_decoder:
	$(Q)mkdir -p bin/decoder logs/decoder
	$(RISCV_CC) $(DECODER_CC_OPTS) $(DECODER_LNK) \
  -I./src/decoder src/decoder/*.c src/decoder/*.S \
  -o bin/decoder/decoder.riscv -lm -lgcc
	$(Q)$(RISCV_OBJD) $(RISCV_OBJD_OPTS) \
	  ./bin/decoder/decoder.riscv > ./logs/decoder/decoder.riscv.dump

sim_decoder:
	$(Q)$(RISCV_SIM) $(RISCV_SIM_OPTS) \
	  ./bin/decoder/decoder.riscv > ./logs/decoder/decoder.log 2>&1 && \
	echo "Running decoder.riscv ...pass" || echo "Running decoder.riscv ...fail"

# -------------------------------------------------------------------------
run_test:
	$(MAKE) cc_test  X=$(X) EXT=$(E) TST=$(T)
	$(MAKE) sim_test X=$(X) EXT=$(E) TST=$(T)

cc_test:	
	$(Q)$(RISCV_CC) $(RISCV_CC_OPTS) -DXLEN=$(X) \
	  ./src/$(EXT)/$(TST).S -o ./bin/$(EXT)/$(X)-$(TST)
	$(Q)$(RISCV_OBJD) $(RISCV_OBJD_OPTS) \
	  ./bin/$(EXT)/$(X)-$(TST) > ./logs/$(EXT)/$(X)-$(TST).dump

sim_test:
	$(Q)$(RISCV_SIM) $(RISCV_SIM_OPTS) \
	  ./bin/$(EXT)/$(X)-$(TST) > ./logs/$(EXT)/$(X)-$(TST).log 2>&1 && \
	echo "Running $(X)-$(TST) ...pass" || echo "Running $(X)-$(TST) ...fail"

# -------------------------------------------------------------------------
clean:
	-rm -rf bin/* logs/*

