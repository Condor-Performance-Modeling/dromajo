

#include "riscv_machine.h"
#include "dromajo_protos.h"

void execution_trace(RISCVMachine *m,int hartid,uint32_t insn_raw) {
  RISCVCPUState *cpu = m->cpu_state[hartid];
  uint64_t last_pc  = virt_machine_get_pc(m, hartid);
  int      priv     = riscv_get_priv_level(cpu);

  fprintf(dromajo_trace, "%d %d 0x%016" PRIx64 " (0x%08x)",
          hartid, priv, last_pc,
          (insn_raw & 3) == 3 ? insn_raw : (uint16_t)insn_raw);

      int iregno = riscv_get_most_recently_written_reg(cpu);
      int fregno = riscv_get_most_recently_written_fp_reg(cpu);

      if (cpu->pending_exception != -1) {
          fprintf(dromajo_trace, " exception %d, tval %016" PRIx64,
                  cpu->pending_exception,
                  riscv_get_priv_level(cpu) == PRV_M ? cpu->mtval : cpu->stval);
      } else if (iregno > 0) {
          fprintf(dromajo_trace, " x%2d 0x%016" PRIx64,
                  iregno, virt_machine_get_reg(m, hartid, iregno));
      } else if (fregno >= 0) {
          fprintf(dromajo_trace, " f%2d 0x%016" PRIx64,
                  fregno, virt_machine_get_fpreg(m, hartid, fregno));
      } else {
          for (int i = 31; i >= 0; i--) {
              if (cpu->most_recently_written_vregs[i]) {
                  fprintf(dromajo_trace, " v%2d 0x", i);
                  for (int j = VLEN / 8 - 1; j >= 0; j--) {
                      fprintf(dromajo_stderr, "%02" PRIx8, cpu->v_reg[i][j]);
                  }
              }
          }
      }

      putc('\n', dromajo_trace);
}


