#! /bin/bash

export OPT='--stf_priv_modes USHM --stf_force_zero_sha'
export DRO=../../bin/cpm_dromajo

echo "clean previous traces"
rm -f traces/*
mkdir -p traces

echo "create/extract elf's"
cd elf
rm -f *.riscv
tar xf *.bz2

echo "clean /extract reference stf's"
cd ../golden
rm -f golden/*.stf
rm -f golden/*.zstf
tar xf *.bz2

cd ..

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
stf_file_type="stf"
runRegression illegal bmi_mm.bare bmi_towers.bare

# Compressed
stf_file_type="zstf"
runRegression illegal bmi_mm.bare bmi_towers.bare
echo "number of diffs = $diffs"

exit $diffs
