#! /bin/bash
rm -f stf_load_store.dump
STF_DUMP=$TOP/stf_tools/release/tools/stf_dump/stf_dump
STF_RDUMP=$TOP/stf_tools/release/tools/stf_record_dump/stf_record_dump
$STF_DUMP  traces/stf_load_store.zstf > stf_load_store.new.dump
$STF_DUMP  golden/stf_load_store.zstf > stf_load_store.gld.dump
$STF_RDUMP traces/stf_load_store.zstf > stf_load_store.new.rdump
$STF_RDUMP golden/stf_load_store.zstf > stf_load_store.gld.rdump
