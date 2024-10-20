#! /bin/bash

export OPT='--stf_priv_modes USHM --stf_force_zero_sha'
export DRO=../../bin/cpm_dromajo

echo "clean previous traces"
mkdir -p traces
rm -f traces/*

echo "create/extract elf's"
cd elf
rm -f *.riscv
tar xf *.bz2

# clean /extract reference stf's
cd ../golden
rm -f golden/*.stf
rm -f golden/*.zstf
tar xf *.bz2

cd ..

diffs=0
stf_file_type="stf"
runRegression()
{
  echo "create the bare metal traces"
  echo "$@"
  for i in $@; do
    $DRO $OPT --stf_trace traces/$i.$stf_file_type  elf/$i.riscv
    echo ""
  done

  echo "compare to the golden traces"
  for i in $@; do
    diff traces/$i.$stf_file_type  golden/$i.$stf_file_type
    diffs=$(expr $diffs + $?)
  done
  echo ""
}

# Uncompressed
runRegression illegal bmi_mm.bare bmi_towers.bare

# Compressed with new stf features
export OPT='--stf_priv_modes USHM --stf_force_zero_sha'

stf_file_type="zstf"
runRegression illegal bmi_mm.bare bmi_towers.bare
echo "number of diffs = $diffs"

exit $diffs
