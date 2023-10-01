all:
	cd zz_utils/lwpr && make
	cd zz_utils/db_pool && make
	cd zz_utils/micro_server && make
	cd zz_metasvr/src && make

micro_server:
	cd zz_utils/micro_server && make

lwpr:
	cd zz_utils/lwpr && make

db_pool:
	cd zz_utils/db_pool && make

eventbus:
	cd zz_utils/eventbus && make

tools:
	cd zz_utils/tools && make

zz_metasvr:
	cd zz_metasvr/src && make

clean:
	cd zz_utils/lwpr && make $@
	cd zz_utils/db_pool && make $@
	cd zz_utils/eventbus && make $@
	cd zz_utils/micro_server && make $@
	cd zz_utils/tools && make $@
	cd zz_metasvr/src && make $@
