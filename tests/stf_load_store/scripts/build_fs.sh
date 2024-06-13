#cd "${TOP}"
sudo cp scripts/inittab $BUILDROOT/output/target/etc
sudo cp scripts/run_benchmark.sh $BUILDROOT/output/target/etc/init.d
sudo rm -rf $BUILDROOT/output/target/root/benchfs
sudo cp -r $BENCHMARKS/benchfs $BUILDROOT/output/target/root
sudo make -C $BUILDROOT
